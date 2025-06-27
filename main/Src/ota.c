#include "ota.h"
#include "config.h"
#include "esp_ota_ops.h"


typedef enum 
{
    OTA_START = 0,
    OTA_update,
    OTA_END,
    OTA_ERROR,
}ota_process_t;

#define LITTLEFS_DIRECTORY  "/littlefs"
const char *ota_file = "ota.bin";
static const char *TAG = "PRJ_OTA";
const char *ota_file_path = "/littlefs/ota.bin";

esp_err_t ota_start(char *data, int len)
{
    esp_err_t err = ESP_OK;
    char *file_path = heap_caps_malloc(strlen(ota_file) + strlen(LITTLEFS_DIRECTORY) + 2, MALLOC_CAP_SPIRAM);
    snprintf(file_path, strlen(ota_file) + strlen(LITTLEFS_DIRECTORY) + 2, "%s/%s", LITTLEFS_DIRECTORY, ota_file);
    ESP_LOGI(TAG,"ota file_path:%s",file_path);
    FILE* file = fopen(file_path, "w");
    if(file == NULL)
    {
        err = ESP_FAIL;
        ESP_LOGE(TAG, "Failed to open ota file");
        return err;
    }

    size_t size = fwrite(data, 1, len, file);
    if(size != len)
    {
        ESP_LOGE(TAG, "Failed to write to file");
        heap_caps_free(file_path);
        fclose(file);
        err = ESP_FAIL;
        return err;
    }

    heap_caps_free(file_path);
    fclose(file);
    return err;
}


esp_err_t ota_update(char *data, int len)
{
    esp_err_t err = ESP_OK;
    char *file_path = heap_caps_malloc(strlen(ota_file) + strlen(LITTLEFS_DIRECTORY) + 2, MALLOC_CAP_SPIRAM);
    snprintf(file_path, strlen(ota_file) + strlen(LITTLEFS_DIRECTORY) + 2, "%s/%s", LITTLEFS_DIRECTORY, ota_file);
    ESP_LOGI(TAG,"ota file_path:%s",file_path);
    FILE* file = fopen(file_path, "a");
    if(file == NULL)
    {
        err = ESP_FAIL;
        ESP_LOGE(TAG, "Failed to open ota file");
        return err;
    }

    size_t size = fwrite(data, 1, len, file);
    if(size != len)
    {
        ESP_LOGE(TAG, "Failed to write to file");
        heap_caps_free(file_path);
        fclose(file);
        err = ESP_FAIL;
        return err;
    }

    heap_caps_free(file_path);
    fclose(file);
    return err;
}


esp_err_t ota_end(char *data, int len)
{
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG,"ota file_path:%s",ota_file_path);
    FILE* file = fopen(ota_file_path, "a");
    if(file == NULL)
    {
        err = ESP_FAIL;
        ESP_LOGE(TAG, "Failed to open ota file");
        return err;
    }

    size_t size = fwrite(data, 1, len, file);
    if(size != len)
    {
        ESP_LOGE(TAG, "Failed to write to file");
        fclose(file);
        err = ESP_FAIL;
        return err;
    }

    fclose(file);
    return err;
}


esp_err_t ota_file_read(char *data,int *len)
{ 
    esp_err_t err = ESP_OK;
    ESP_LOGI(TAG,"ota file_path:%s",ota_file_path);
    FILE* file = fopen(ota_file_path, "r");
    if(file == NULL)
    {
        err = ESP_FAIL;
        ESP_LOGE(TAG, "Failed to open ota file");
        return err;
    }
    fseek(file, 0L, SEEK_END);  // 将文件指针移动到文件末尾
    size_t size = ftell(file);
    rewind(file);
    *len = size;
    data = heap_caps_malloc(size, MALLOC_CAP_SPIRAM);
    if(data == NULL)
    {
        err = ESP_FAIL;
        ESP_LOGE(TAG, "Failed to allocate memory for ota file");
        return err;
    }
    memset(data, 0, size);
    size_t read_num = fread(data, 1, size, file);
    if(read_num != size)
    {
        err = ESP_FAIL;
        ESP_LOGE(TAG, "Failed to read file");
        heap_caps_free(data);
        fclose(file);
        return err;
    }

    fclose(file);

    return err;
}



void ota_process(void)
{
    char *ota_data = NULL;
    int len = 0;
    esp_ota_handle_t update_handle;
    esp_partition_t *update_partition;
    if(sys_config.ota_flag == OTA_FLAG_TURE)
    {
        esp_err_t err = ota_file_read(ota_data,&len);
        if(err == ESP_OK)
        {
            update_partition = esp_ota_get_next_update_partition(NULL);
            err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &update_handle);
            if(!err) return;
            err = esp_ota_write(update_handle, ota_data, len);
            err = esp_ota_end(update_handle);
            if(!err) return;
            err = esp_ota_set_boot_partition(update_partition);
            if(!err) return;

            esp_restart();
        }
    }

}





