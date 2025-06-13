#include "inference.h"
#include "tflm.h"

static const char *TAG = "PRJ_INFERENCE";

inference_model_data_t *inference_data;

extern float  test_data[280][3];
void model_data_uniformization(float *interence_wicket,uint32_t row,uint32_t col, float *dx_data,uint32_t now_row);
/* 推理任务回调函数 */
void inference_task_callback(model_t model_type)
{
    uint32_t dx_num = inference_data->model_data[model_type].dx_num;                                 //获取电芯数量
    uint32_t input_size = inference_data->model_data[model_type].input_wicket_col                     //计算输入数据长度
                                 * inference_data->model_data[model_type].input_wicket_row * sizeof(float);              
    
    float *input_data = heap_caps_calloc(1,input_size, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);                   //申请推理数据内存
    model_data_uniformization(input_data,inference_data->model_data[model_type].input_wicket_row,inference_data->model_data[model_type].input_wicket_col,&test_data,0);
    // for(int i = 0; i < input_size/4; i++)
    // {
    //     printf("%f ",input_data[i]);
    // }
    
    for(int i = 0; i < dx_num; i++)
    {
        float *output_data = inference_data->model_data[model_type].dx_data[i].inference_result;         //获取推理结果存储地址
        if(inference_data->model_data[model_type].dx_data[i].state == STANDBY)               //判断输入数据是否更新，是否需要推理
        {
            inference_data->model_data[model_type].dx_data[i].state = STANDBY;
            
            tflm_run(model_type,input_data,input_size/4,output_data);                                   //进行推理
                                                                                
            printf("Model %d--DX %d inference finish! result is %f %f!",model_type,i,output_data[0],output_data[1]);
        }
    }

    heap_caps_free(input_data);                                                                      //释放推理数据窗口 
}


/* 电芯SOC推理任务 */
void inference_soc_task_handler(void *pvParameter)
{
    while(1)
    {
        ESP_LOGI(TAG,"inference for soc start!");
        unsigned long  start_time = esp_timer_get_time();
        inference_task_callback(SOC_MODEL);
        unsigned long  end_time = esp_timer_get_time();
        ESP_LOGI(TAG,"inference for soc finish! using time is  %d ms!",(int)(end_time-start_time)/1000);
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/* 电芯SOH推理任务 */
void inference_soh_task_handler(void *pvParameter)
{
    while(1)
    {
        ESP_LOGI(TAG,"inference for soh start!");
        unsigned long  start_time = esp_timer_get_time();
        inference_task_callback(SOH_MODEL);
        unsigned long  end_time = esp_timer_get_time();
        ESP_LOGI(TAG,"inference for soh finish! using time is  %d ms!",(int)(end_time-start_time)/1000);
        vTaskDelay(pdMS_TO_TICKS(180000));
    }
}

/* 电芯RUL推理任务 */
void inference_rul_task_handler(void *pvParameter)
{
    while(1)
    {
        ESP_LOGI(TAG,"inference for rul start!");
        unsigned long  start_time = esp_timer_get_time();
        inference_task_callback(RUL_MODEL);
        unsigned long  end_time = esp_timer_get_time();
        ESP_LOGI(TAG,"inference for rul finish! using time is  %d ms!",(int)(end_time-start_time)/1000);
        vTaskDelay(pdMS_TO_TICKS(180000));
    }
}

/* 电芯RSK推理任务 */
void inference_rsk_task_handler(void *pvParameter)
{
    while(1)
    {
        ESP_LOGI(TAG,"inference for rsk start!");
        unsigned long  start_time = esp_timer_get_time();
        inference_task_callback(RSK_MODEL);
        unsigned long  end_time = esp_timer_get_time();
        ESP_LOGI(TAG,"inference for rsk finish! using time is  %d ms!",(int)(end_time-start_time)/1000);
        vTaskDelay(pdMS_TO_TICKS(180000));
    }
}


void inference_init(void)
{
    /* 初始化TFLM */
    tflm_init();    
    /* 申请数据内存 */
    inference_data = (inference_model_data_t *)heap_caps_calloc(1,sizeof(inference_model_data_t),MALLOC_CAP_SPIRAM);
    inference_data->model_num = MODEL_MAX_NUM;
    inference_data->model_data[SOC_MODEL].dx_num = 1;
    inference_data->model_data[SOC_MODEL].input_wicket_row  = 280;
    inference_data->model_data[SOC_MODEL].input_wicket_col  = 3;
    inference_data->model_data[SOC_MODEL].dx_data[0].state = STANDBY;
    /*  创建推理任务  */
    xTaskCreatePinnedToCore(inference_soc_task_handler, "inference_soc_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 4, NULL, 1);
    // xTaskCreatePinnedToCore(inference_soh_task_handler, "inference_soh_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 4, NULL, 1);
    // xTaskCreatePinnedToCore(inference_rul_task_handler, "inference_rul_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 4, NULL, 1);
    // xTaskCreatePinnedToCore(inference_rsk_task_handler, "inference_rsk_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 4, NULL, 1);
}




