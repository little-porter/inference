#include "config.h"
#include "littlefs_ops.h"
#include "cJSON.h"

static const char *TAG = "PRJ_CONFIG"; 

sys_config_t sys_config;


void config_init(sys_config_t *config)
{
    ESP_LOGI(TAG, "Init config");
    littlefs_file_data_t config_file;
    char *str = littlefs_ops_read_file("config.json",&config_file);
    if(str == NULL) 
    {
        ESP_LOGI(TAG, "read config.json fail");
        return;
    }

    // 解析 JSON
    cJSON *root = cJSON_Parse(config_file.data);
    if (!root) {
        ESP_LOGE(TAG, "Error parsing JSON,use defualt config!");
        free(config_file.data);
        return;
    }
    //推理模块配置提取
    ESP_LOGI(TAG, "start config tl_device!");
    cJSON *tl_device = cJSON_GetObjectItemCaseSensitive(root, "tl_device");
    if(tl_device)
    {
        /*获取OTA标志*/
        cJSON *ota_flag = cJSON_GetObjectItemCaseSensitive(tl_device, "ota_flag");
        if(ota_flag && cJSON_IsNumber(ota_flag))
        {
            config->ota_flag = ota_flag->valueint;
        }
        else
        {
            config->ota_flag = OTA_FLAG_FALSE;
        }
        /*获取休眠模式标志*/
        cJSON *sleep_mode = cJSON_GetObjectItemCaseSensitive(tl_device, "sleep_mode");
        if(sleep_mode && cJSON_IsNumber(sleep_mode))
        {
            config->sleep_mode = sleep_mode->valueint;
        }
        else
        {
            config->sleep_mode = SLEEP_MODE_OFF;
        }
        /*获取设备ID*/
        cJSON *dev_id = cJSON_GetObjectItemCaseSensitive(tl_device, "dev_id");
        if(dev_id && cJSON_IsNumber(dev_id))
        {
            config->dev_id = dev_id->valueint;
        }
        else
        {
            config->dev_id = DEFAULT_DEV_ID;
        }
        /*获取模型输入数据源配置*/
        cJSON *input_data = cJSON_GetObjectItemCaseSensitive(tl_device, "input_data");
        if(input_data && cJSON_IsNumber(input_data))
        {
            if(input_data->valueint > MODEL_DATA_SOURCE_SELF)
            {
                config->model_input_source = MODEL_DATA_SOURCE_BMS;
            }
            else
            {
                config->model_input_source = input_data->valueint;
            }
            
        }
        else
        {
            config->model_input_source = MODEL_DATA_SOURCE_BMS;
        }
        /*获取推理模块数据输出端口配置*/
        cJSON *output_data = cJSON_GetObjectItemCaseSensitive(tl_device, "output_data");
        if(output_data && cJSON_IsNumber(output_data))
        {
            if(output_data->valueint > MODEL_OUTPUT_PORT_CAN)
            {
                config->model_output_port = MODEL_OUTPUT_PORT_WIFI;
            }
            else
            {
                config->model_output_port = output_data->valueint;
            }
            
        }
        else
        {
            config->model_output_port = MODEL_OUTPUT_PORT_WIFI;
        }
        /*获取推理模块推理结果输出端口配置*/
        cJSON *result = cJSON_GetObjectItemCaseSensitive(tl_device, "result");
        if(result && cJSON_IsNumber(result))
        {
            if(result->valueint > MODEL_OUTPUT_PORT_CAN)
            {
                config->model_result_port = MODEL_OUTPUT_PORT_WIFI;
            }
            else
            {
                config->model_result_port = result->valueint;
            }
            
        }
        else
        {
            config->model_result_port = MODEL_OUTPUT_PORT_WIFI;
        }     

        ESP_LOGI(TAG,"tl_config: ota_flag: %d ; sleep_mode: %d ; dev_id: %d ; input_data: %d ; output_data: %d ; result: %d",
                                        (int)config->ota_flag,(int)config->sleep_mode,
                                        (int)config->dev_id,(int)config->model_input_source,
                                        (int)config->model_output_port,
                                        (int)config->model_result_port
                                        );
    }
    else
    {
        /*默认配置*/
        config->ota_flag = OTA_FLAG_FALSE;
        config->sleep_mode = SLEEP_MODE_OFF;
        config->dev_id = DEFAULT_DEV_ID;
        config->model_input_source = MODEL_DATA_SOURCE_BMS;
        config->model_output_port = MODEL_OUTPUT_PORT_WIFI;
        config->model_result_port = MODEL_OUTPUT_PORT_WIFI;
    }

    /*获取rs485端口配置信息*/
    cJSON *rs485_device = cJSON_GetObjectItemCaseSensitive(root, "rs485_device");
    if(rs485_device)
    {
        cJSON *baudrate = cJSON_GetObjectItemCaseSensitive(rs485_device, "baudrate");
        if(baudrate && cJSON_IsNumber(baudrate))
        {
            config->rs485_config.baudrate = baudrate->valueint;
        }
        else
        {
            config->rs485_config.baudrate = 9600;
        }

        cJSON *use = cJSON_GetObjectItemCaseSensitive(rs485_device, "use");
        if(use && cJSON_IsNumber(use))
        {
            config->rs485_config.use = use->valueint;
        }
        else
        {
            config->rs485_config.use = NOT_USE;
        }

        ESP_LOGI(TAG, "RS485 config: baudrate: %d ;use: %d", (int)config->rs485_config.baudrate,(int)config->rs485_config.use);
    }
    else
    {
        config->rs485_config.baudrate = 9600;
        config->rs485_config.use = NOT_USE;
    }

    /*获取CAN端口配置信息*/
    cJSON *can_device = cJSON_GetObjectItemCaseSensitive(root, "can_device");
    if(can_device)
    {
        cJSON *baudrate = cJSON_GetObjectItemCaseSensitive(can_device, "baudrate");
        if(baudrate && cJSON_IsNumber(baudrate))
        {
            config->can_config.baudrate = baudrate->valueint;
        }
        else
        {
            config->can_config.baudrate = 500000;
        }

        cJSON *use = cJSON_GetObjectItemCaseSensitive(can_device, "use");
        if(use && cJSON_IsNumber(use))
        {
            config->can_config.use = use->valueint;
        }
        else
        {
            config->can_config.use = NOT_USE;
        }
        ESP_LOGI(TAG, "CAN config: baudrate: %d ;use: %d", (int)config->can_config.baudrate,(int)config->can_config.use);
    }
    else
    {
        config->can_config.baudrate = 500000;
        config->can_config.use = NOT_USE;
    }

     /*获取dtu_4g端口配置信息*/
    cJSON *dtu_4g = cJSON_GetObjectItemCaseSensitive(root, "dtu_4g");
    if(dtu_4g)
    {
        char name_str[10] = {0};
        cJSON *name = cJSON_GetObjectItemCaseSensitive(dtu_4g, "name");
        if(name && cJSON_IsString(name))
        {
            memcpy(name_str, name->valuestring, strlen(name->valuestring));
        }
        

        cJSON *baudrate = cJSON_GetObjectItemCaseSensitive(dtu_4g, "baudrate");
        if(baudrate && cJSON_IsNumber(baudrate))
        {
            config->dtu_4g_config.baudrate = baudrate->valueint;
        }
        else
        {
            config->dtu_4g_config.baudrate = 115200;
        }

        cJSON *use = cJSON_GetObjectItemCaseSensitive(dtu_4g, "use");
        if(use && cJSON_IsNumber(use))
        {
            config->dtu_4g_config.use = use->valueint;
        }
        else
        {
            config->dtu_4g_config.use = NOT_USE;
        }
        ESP_LOGI(TAG, "dtu_4g config:name: %s ; baudrate: %d ;use: %d",name_str, (int)config->dtu_4g_config.baudrate,(int)config->dtu_4g_config.use);
    }
    else
    {
        config->dtu_4g_config.baudrate = 115200;
        config->dtu_4g_config.use = NOT_USE;
    }

    /*获取wifi配置信息*/
    cJSON *wifi = cJSON_GetObjectItemCaseSensitive(root, "wifi");
    if(wifi)
    {
        cJSON *ssid = cJSON_GetObjectItemCaseSensitive(wifi, "ssid");
        if(ssid && cJSON_IsString(ssid))
        {
            memcpy(config->wifi_config.ssid, ssid->valuestring, strlen(ssid->valuestring));
        }
        else
        {
            memcpy(config->wifi_config.ssid, "12345678", strlen("12345678"));
        }

        cJSON *password = cJSON_GetObjectItemCaseSensitive(wifi, "password");
        if(password && cJSON_IsString(password))
        {
            memcpy(config->wifi_config.password, password->valuestring, strlen(password->valuestring));
        }
        else
        {
            memcpy(config->wifi_config.password, "12345678", strlen("12345678"));
        }

        cJSON *use = cJSON_GetObjectItemCaseSensitive(wifi, "use");
        if(use && cJSON_IsNumber(use))
        {
            config->wifi_config.use = use->valueint;
        }
        else
        {
            config->wifi_config.use = NOT_USE;
        }

        ESP_LOGI(TAG, "wifi_config: ssid: %s, password: %s, use: %d", config->wifi_config.ssid, config->wifi_config.password, (int)config->wifi_config.use);
    }
    else
    {
        memcpy(config->wifi_config.ssid, "12345678", strlen("12345678"));
        memcpy(config->wifi_config.password, "12345678", strlen("12345678"));
        config->wifi_config.use = NOT_USE;
    }

    /*获取soh模型配置信息*/
    cJSON *soh_model = cJSON_GetObjectItemCaseSensitive(root, "soh_model");
    if(soh_model)
    {
       cJSON *type = cJSON_GetObjectItemCaseSensitive(soh_model, "type"); 
       if (type && cJSON_IsNumber(type))
       {
            config->soh_config.type = type->valueint;
       }
       else
       {
            config->soh_config.type = 0;
       }

       cJSON *value_type = cJSON_GetObjectItemCaseSensitive(soh_model, "value_type");
       if(value_type && cJSON_IsArray(value_type))
       {
            int count = cJSON_GetArraySize(value_type);
            for(int i = 0; i < count; i++)
            {
                config->soh_config.value_type[i] = cJSON_GetArrayItem(value_type, i)->valueint;
            }
         
       }

       cJSON *file_name = cJSON_GetObjectItemCaseSensitive(soh_model, "file_name");
       if(file_name && cJSON_IsString(file_name))
       {
            memcpy(config->soh_config.file_name, file_name->valuestring, strlen(file_name->valuestring));
       }
       else
       {
            memcpy(config->soh_config.file_name, "soh_model.bin", strlen("soh_model.bin"));
       }

       cJSON *row = cJSON_GetObjectItemCaseSensitive(soh_model, "row");
       if(row && cJSON_IsNumber(row))
       {
            config->soh_config.row = row->valueint;
       }
       else
       {
            config->soh_config.row = 0;
       }

       cJSON *col = cJSON_GetObjectItemCaseSensitive(soh_model, "col");
       if(col && cJSON_IsNumber(col))
       {
            config->soh_config.column = col->valueint;
       }
       else
       {
            config->soh_config.column = 0;
       }

       cJSON *result = cJSON_GetObjectItemCaseSensitive(soh_model, "result");
       if(result && cJSON_IsNumber(result))
       {
            config->soh_config.result = result->valueint;
       }
       else
       {
            config->soh_config.result = 0;
       }

       cJSON *use = cJSON_GetObjectItemCaseSensitive(soh_model, "use");
       if(use && cJSON_IsNumber(use))
       {
            config->soh_config.use = use->valueint;
       }
       else
       {
            config->soh_config.use = NOT_USE;
       }

       ESP_LOGI(TAG, "soh_model config: type: %d, value_type: %s, file_name: %s, row: %d, column: %d, result: %d, use: %d", 
                            (int)config->soh_config.type, config->soh_config.value_type, 
                            config->soh_config.file_name, (int)config->soh_config.row, 
                            (int)config->soh_config.column, (int)config->soh_config.result, 
                            (int)config->soh_config.use);

    }
    else
    {
        config->soh_config.type = 0;
        memset(config->soh_config.value_type, 0, sizeof(config->soh_config.value_type));
        memcpy(config->soh_config.file_name, "soh_model.bin", strlen("soh_model.bin"));
        config->soh_config.row = 0;
        config->soh_config.column = 0;
        config->soh_config.result = 0;
        config->soh_config.use = NOT_USE;
    }
     /*获取soc模型配置信息*/
    cJSON *soc_model = cJSON_GetObjectItemCaseSensitive(root, "soc_model");
    if(soc_model)
    {
       cJSON *type = cJSON_GetObjectItemCaseSensitive(soc_model, "type"); 
       if (type && cJSON_IsNumber(type))
       {
            config->soc_config.type = type->valueint;
       }
       else
       {
            config->soc_config.type = 0;
       }

       cJSON *value_type = cJSON_GetObjectItemCaseSensitive(soc_model, "value_type");
       if(value_type && cJSON_IsArray(value_type))
       {
            int count = cJSON_GetArraySize(value_type);
            for(int i = 0; i < count; i++)
            {
                config->soc_config.value_type[i] = cJSON_GetArrayItem(value_type, i)->valueint;
            }
         
       }

       cJSON *file_name = cJSON_GetObjectItemCaseSensitive(soc_model, "file_name");
       if(file_name && cJSON_IsString(file_name))
       {
            memcpy(config->soc_config.file_name, file_name->valuestring, strlen(file_name->valuestring));
       }
       else
       {
            memcpy(config->soc_config.file_name, "soc_model.bin", strlen("soc_model.bin"));
       }

       cJSON *row = cJSON_GetObjectItemCaseSensitive(soc_model, "row");
       if(row && cJSON_IsNumber(row))
       {
            config->soc_config.row = row->valueint;
       }
       else
       {
            config->soc_config.row = 0;
       }

       cJSON *col = cJSON_GetObjectItemCaseSensitive(soc_model, "col");
       if(col && cJSON_IsNumber(col))
       {
            config->soc_config.column = col->valueint;
       }
       else
       {
            config->soc_config.column = 0;
       }

       cJSON *result = cJSON_GetObjectItemCaseSensitive(soc_model, "result");
       if(result && cJSON_IsNumber(result))
       {
            config->soc_config.result = result->valueint;
       }
       else
       {
            config->soc_config.result = 0;
       }

       cJSON *use = cJSON_GetObjectItemCaseSensitive(soc_model, "use");
       if(use && cJSON_IsNumber(use))
       {
            config->soc_config.use = use->valueint;
       }
       else
       {
            config->soc_config.use = NOT_USE;
       }

       ESP_LOGI(TAG, "soc_model config: type: %d, value_type: %s, file_name: %s, row: %d, column: %d, result: %d, use: %d", 
                            (int)config->soc_config.type, config->soc_config.value_type, 
                            config->soc_config.file_name, (int)config->soc_config.row, 
                            (int)config->soc_config.column, (int)config->soc_config.result, 
                            (int)config->soc_config.use);

    }
    else
    {
        config->soc_config.type = 0;
        memset(config->soc_config.value_type, 0, sizeof(config->soc_config.value_type));
        memcpy(config->soc_config.file_name, "soc_model.bin", strlen("soc_model.bin"));
        config->soc_config.row = 0;
        config->soc_config.column = 0;
        config->soc_config.result = 0;
        config->soc_config.use = NOT_USE;
    }

     /*获取rul模型配置信息*/
    cJSON *rul_model = cJSON_GetObjectItemCaseSensitive(root, "rul_model");
    if(rul_model)
    {
       cJSON *type = cJSON_GetObjectItemCaseSensitive(rul_model, "type"); 
       if (type && cJSON_IsNumber(type))
       {
            config->rul_config.type = type->valueint;
       }
       else
       {
            config->rul_config.type = 0;
       }

       cJSON *value_type = cJSON_GetObjectItemCaseSensitive(rul_model, "value_type");
       if(value_type && cJSON_IsArray(value_type))
       {
            int count = cJSON_GetArraySize(value_type);
            for(int i = 0; i < count; i++)
            {
                config->rul_config.value_type[i] = cJSON_GetArrayItem(value_type, i)->valueint;
            }
         
       }

       cJSON *file_name = cJSON_GetObjectItemCaseSensitive(rul_model, "file_name");
       if(file_name && cJSON_IsString(file_name))
       {
            memcpy(config->rul_config.file_name, file_name->valuestring, strlen(file_name->valuestring));
       }
       else
       {
            memcpy(config->rul_config.file_name, "rul_model.bin", strlen("rul_model.bin"));
       }

       cJSON *row = cJSON_GetObjectItemCaseSensitive(rul_model, "row");
       if(row && cJSON_IsNumber(row))
       {
            config->rul_config.row = row->valueint;
       }
       else
       {
            config->rul_config.row = 0;
       }

       cJSON *col = cJSON_GetObjectItemCaseSensitive(rul_model, "col");
       if(col && cJSON_IsNumber(col))
       {
            config->rul_config.column = col->valueint;
       }
       else
       {
            config->rul_config.column = 0;
       }

       cJSON *result = cJSON_GetObjectItemCaseSensitive(rul_model, "result");
       if(result && cJSON_IsNumber(result))
       {
            config->rul_config.result = result->valueint;
       }
       else
       {
            config->rul_config.result = 0;
       }

       cJSON *use = cJSON_GetObjectItemCaseSensitive(rul_model, "use");
       if(use && cJSON_IsNumber(use))
       {
            config->rul_config.use = use->valueint;
       }
       else
       {
            config->rul_config.use = NOT_USE;
       }

       ESP_LOGI(TAG, "rul_model config: type: %d, value_type: %s, file_name: %s, row: %d, column: %d, result: %d, use: %d", 
                            (int)config->rul_config.type, config->rul_config.value_type, 
                            config->rul_config.file_name, (int)config->rul_config.row, 
                            (int)config->rul_config.column, (int)config->rul_config.result, 
                            (int)config->rul_config.use);

    }
    else
    {
        config->rul_config.type = 0;
        memset(config->rul_config.value_type, 0, sizeof(config->rul_config.value_type));
        memcpy(config->rul_config.file_name, "rul_model.bin", strlen("rul_model.bin"));
        config->rul_config.row = 0;
        config->rul_config.column = 0;
        config->rul_config.result = 0;
        config->rul_config.use = NOT_USE;
    }

     /*获取rsk模型配置信息*/
    cJSON *rsk_model = cJSON_GetObjectItemCaseSensitive(root, "rsk_model");
    if(rsk_model)
    {
       cJSON *type = cJSON_GetObjectItemCaseSensitive(rsk_model, "type"); 
       if (type && cJSON_IsNumber(type))
       {
            config->rsk_config.type = type->valueint;
       }
       else
       {
            config->rsk_config.type = 0;
       }

       cJSON *value_type = cJSON_GetObjectItemCaseSensitive(rsk_model, "value_type");
       if(value_type && cJSON_IsArray(value_type))
       {
            int count = cJSON_GetArraySize(value_type);
            for(int i = 0; i < count; i++)
            {
                config->rsk_config.value_type[i] = cJSON_GetArrayItem(value_type, i)->valueint;
            }
         
       }

       cJSON *file_name = cJSON_GetObjectItemCaseSensitive(rsk_model, "file_name");
       if(file_name && cJSON_IsString(file_name))
       {
            memcpy(config->rsk_config.file_name, file_name->valuestring, strlen(file_name->valuestring));
       }
       else
       {
            memcpy(config->rsk_config.file_name, "rsk_model.bin", strlen("rsk_model.bin"));
       }

       cJSON *row = cJSON_GetObjectItemCaseSensitive(rsk_model, "row");
       if(row && cJSON_IsNumber(row))
       {
            config->rsk_config.row = row->valueint;
       }
       else
       {
            config->rsk_config.row = 0;
       }

       cJSON *col = cJSON_GetObjectItemCaseSensitive(rsk_model, "col");
       if(col && cJSON_IsNumber(col))
       {
            config->rsk_config.column = col->valueint;
       }
       else
       {
            config->rsk_config.column = 0;
       }

       cJSON *result = cJSON_GetObjectItemCaseSensitive(rsk_model, "result");
       if(result && cJSON_IsNumber(result))
       {
            config->rsk_config.result = result->valueint;
       }
       else
       {
            config->rsk_config.result = 0;
       }

       cJSON *use = cJSON_GetObjectItemCaseSensitive(rsk_model, "use");
       if(use && cJSON_IsNumber(use))
       {
            config->rsk_config.use = use->valueint;
       }
       else
       {
            config->rsk_config.use = NOT_USE;
       }

       ESP_LOGI(TAG, "rsk_model config: type: %d, value_type: %s, file_name: %s, row: %d, column: %d, result: %d, use: %d", 
                            (int)config->rsk_config.type, config->rsk_config.value_type, 
                            config->rsk_config.file_name, (int)config->rsk_config.row, 
                            (int)config->rsk_config.column, (int)config->rsk_config.result, 
                            (int)config->rsk_config.use);

    }
    else
    {
        config->rsk_config.type = 0;
        memset(config->rsk_config.value_type, 0, sizeof(config->rsk_config.value_type));
        memcpy(config->rsk_config.file_name, "rsk_model.bin", strlen("rsk_model.bin"));
        config->rsk_config.row = 0;
        config->rsk_config.column = 0;
        config->rsk_config.result = 0;
        config->rsk_config.use = NOT_USE;
    }

    /*获取pack配置信息*/
    cJSON *pack = cJSON_GetObjectItemCaseSensitive(root, "pack");
    if(pack)
    {
        cJSON *cell_number = cJSON_GetObjectItemCaseSensitive(pack, "cell_number");
        if(cell_number && cJSON_IsNumber(cell_number))
        {
            config->pack_config.cell_num = cell_number->valueint;
        }
        else
        {
            config->pack_config.cell_num = 0;
        }

        cJSON *capacity = cJSON_GetObjectItemCaseSensitive(pack, "capacity");
        if(capacity && cJSON_IsNumber(capacity))
        {
            config->pack_config.capacity = capacity->valuedouble;
        }
        else
        {
            config->pack_config.capacity = 0;
        }

        ESP_LOGI(TAG, "pack config: cell_num: %d, capacity: %f", (int)config->pack_config.cell_num, config->pack_config.capacity);
    }
    else
    {
        config->pack_config.cell_num = 0;
        config->pack_config.capacity = 0;
    }

    // 清理资源
    cJSON_Delete(root);
    // cJSON_Delete(flag);
    heap_caps_free(config_file.data);
}


void config_save(sys_config_t *config)
{
    char *config_file = heap_caps_malloc(1024*2, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
    int size = \
    snprintf(config_file,1024*2,"{"
                                    "\"tl_device\":{"
                                        "\"ota_flag\":%d,"
                                        "\"sleep_mode\":%d,"
                                        "\"dev_id\":%d,"
                                        "\"input_data\":%d,"
                                        "\"output_data\":%d,"
                                        "\"result\":%d"
                                    "},"

                                    "\"rs485_device\":{"
                                        "\"baudrate\":%d,"
                                        "\"use\":%d"
                                    "},"

                                    "\"can_device\":{"
                                        "\"baudrate\":%d,"
                                        "\"use\":%d"
                                    "},"

                                    "\"dtu_4g\":{"
                                        "\name\":\"Y100E\","
                                        "\"baudrate\":%d,"
                                        "\"use\":%d"
                                    "},"

                                    "\"wifi\":{"
                                        "\"ssid\":%s,"
                                        "\"password\":%s,"
                                        "\"use\":%d"
                                    "},"

                                    "\"soh_model\":{"
                                        "\"type\":%d,"
                                        "\value_type\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],"
                                        "\"file_name\":%s,"
                                        "\"row\":%d,"
                                        "\"col\":%d,"
                                        "\"result\":%d,"
                                        "\"use\":%d"
                                    "},"

                                     "\"soc_model\":{"
                                        "\"type\":%d,"
                                        "\value_type\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],"
                                        "\"file_name\":%s,"
                                        "\"row\":%d,"
                                        "\"col\":%d,"
                                        "\"result\":%d,"
                                        "\"use\":%d"
                                    "},"

                                     "\"rul_model\":{"
                                        "\"type\":%d,"
                                        "\value_type\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],"
                                        "\"file_name\":%s,"
                                        "\"row\":%d,"
                                        "\"col\":%d,"
                                        "\"result\":%d,"
                                        "\"use\":%d"
                                    "},"

                                     "\"rsk_model\":{"
                                        "\"type\":%d,"
                                        "\value_type\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d],"
                                        "\"file_name\":%s,"
                                        "\"row\":%d,"
                                        "\"col\":%d,"
                                        "\"result\":%d,"
                                        "\"use\":%d"
                                    "},"

                                    "\"pack\":{"
                                        "\"cell_number\":%d,"
                                        "\"capacity\":%f"
                                    "}"
                                "}",
                                (int)config->ota_flag,(int)config->sleep_mode,(int)config->dev_id,(int)config->model_input_source,
                                (int)config->model_output_port,(int)config->model_result_port,
                                
                                (int)config->rs485_config.baudrate,(int)config->rs485_config.use,

                                (int)config->can_config.baudrate,(int)config->can_config.use,

                                (int)config->dtu_4g_config.baudrate,(int)config->dtu_4g_config.use,

                                config->wifi_config.ssid,config->wifi_config.password,(int)config->wifi_config.use,

                                (int)config->soh_config.type,
                                (int)config->soh_config.value_type[0],(int)config->soh_config.value_type[1],(int)config->soh_config.value_type[2],
                                (int)config->soh_config.value_type[3],(int)config->soh_config.value_type[4],(int)config->soh_config.value_type[5],
                                (int)config->soh_config.value_type[6],(int)config->soh_config.value_type[7],(int)config->soh_config.value_type[8],
                                (int)config->soh_config.value_type[9],(int)config->soh_config.value_type[10],(int)config->soh_config.value_type[11],
                                (int)config->soh_config.value_type[12],
                                config->soh_config.file_name,
                                (int)config->soh_config.row,
                                (int)config->soh_config.column,
                                (int)config->soh_config.result,
                                (int)config->soh_config.use,

                                (int)config->soc_config.type,
                                (int)config->soc_config.value_type[0],(int)config->soc_config.value_type[1],(int)config->soc_config.value_type[2],
                                (int)config->soc_config.value_type[3],(int)config->soc_config.value_type[4],(int)config->soc_config.value_type[5],
                                (int)config->soc_config.value_type[6],(int)config->soc_config.value_type[7],(int)config->soc_config.value_type[8],
                                (int)config->soc_config.value_type[9],(int)config->soc_config.value_type[10],(int)config->soc_config.value_type[11],
                                (int)config->soc_config.value_type[12],
                                config->soc_config.file_name,
                                (int)config->soc_config.row,
                                (int)config->soc_config.column,
                                (int)config->soc_config.result,
                                (int)config->soc_config.use,

                                (int)config->rul_config.type,
                                (int)config->rul_config.value_type[0],(int)config->rul_config.value_type[1],(int)config->rul_config.value_type[2],
                                (int)config->rul_config.value_type[3],(int)config->rul_config.value_type[4],(int)config->rul_config.value_type[5],
                                (int)config->rul_config.value_type[6],(int)config->rul_config.value_type[7],(int)config->rul_config.value_type[8],
                                (int)config->rul_config.value_type[9],(int)config->rul_config.value_type[10],(int)config->rul_config.value_type[11],
                                (int)config->rul_config.value_type[12],
                                config->rul_config.file_name,
                                (int)config->rul_config.row,
                                (int)config->rul_config.column,
                                (int)config->rul_config.result,
                                (int)config->rul_config.use,

                                (int)config->rsk_config.type,
                                (int)config->rsk_config.value_type[0],(int)config->rsk_config.value_type[1],(int)config->rsk_config.value_type[2],
                                (int)config->rsk_config.value_type[3],(int)config->rsk_config.value_type[4],(int)config->rsk_config.value_type[5],
                                (int)config->rsk_config.value_type[6],(int)config->rsk_config.value_type[7],(int)config->rsk_config.value_type[8],
                                (int)config->rsk_config.value_type[9],(int)config->rsk_config.value_type[10],(int)config->rsk_config.value_type[11],
                                (int)config->rsk_config.value_type[12],
                                config->rsk_config.file_name,
                                (int)config->rsk_config.row,
                                (int)config->rsk_config.column,
                                (int)config->rsk_config.result,
                                (int)config->rsk_config.use,

                                (int)config->pack_config.cell_num,config->pack_config.capacity
                            );

    bool res = littlefs_ops_write_file("config.json",config_file,size);

    if(res)
    {
        ESP_LOGI(TAG,"config write success");
    }
    else
    {
        ESP_LOGE(TAG,"config write failed");
    }
}







