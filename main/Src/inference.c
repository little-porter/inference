#include "inference.h"
#include "esp_task_wdt.h"
#include "bms.h"

static const char *TAG = "PRJ_INFERENCE";




inference_model_data_t *g_inference_data[MODEL_MAX_NUM];

extern float  test_data[280][3];
void model_data_uniformization(float *interence_wicket,uint32_t row,uint32_t col, float *dx_data,uint32_t now_row);
/* 推理任务回调函数 */
void inference_task_callback(model_t model_type)
{
    inference_model_data_t *inference_data = NULL;
    for (int i = 0; i < MODEL_MAX_NUM; i++)
    {
        /* code */
        if(g_inference_data[i]->type == model_type)
        {
            inference_data = g_inference_data[i];
            break;
        }
    }
    if(inference_data == NULL) return;

    

    uint32_t dx_num = inference_data->model_data.dx_num;                                 //获取电芯数量
    uint32_t input_size = inference_data->model_data.input_wicket_col                     //计算输入数据长度
                                 * inference_data->model_data.input_wicket_row * sizeof(float);              
    
    float *input_data = heap_caps_calloc(1,input_size, MALLOC_CAP_8BIT | MALLOC_CAP_DMA);                   //申请推理数据内存
   
    // for(int i = 0; i < input_size/4; i++)
    // {
    //     printf("%f ",input_data[i]);
    // }
    
    for(int i = 0; i < dx_num; i++)
    {
        
        if(inference_data->model_data.dx_data[i].full_flag == WICKET_NOT_FULL)      continue;
        // model_data_uniformization(input_data,inference_data->model_data.input_wicket_row,inference_data->model_data.input_wicket_col,&test_data,0);
        model_data_uniformization(input_data,inference_data->model_data.input_wicket_row,inference_data->model_data.input_wicket_col,inference_data->model_data.dx_data[i].input_wicket_data,inference_data->model_data.dx_data[i].current_row);
            float *output_data = inference_data->model_data.dx_data[i].inference_result;                        //获取推理结果存储地址
        if(inference_data->model_data.dx_data[i].state == STANDBY)                                          //判断输入数据是否更新，是否需要推理
        {
            inference_data->model_data.dx_data[i].state = STANDBY;
            
            // tflm_run(model_type,input_data,input_size/4,output_data);                                   //进行推理
            tflm_run(&inference_data->tflm,input_data,input_size/4,output_data,inference_data->model_data.result_num);
            // ESP_ERROR_CHECK(esp_task_wdt_reset());      
            vTaskDelay(5 / portTICK_PERIOD_MS); 
            printf("Model %d--DX %d inference finish! result is",model_type,i);
            for(int i = 0; i < inference_data->model_data.result_num; i++)   
            {
                printf(" %f ",output_data[i]);
            }    
            float send_output[10] = {0};
            int send_num = 0;
            if(model_type == RSK_MODEL)
            {
                send_num = inference_data->model_data.result_num/5;
                // memcpy(send_output,output_data,send_num); 
            }
            else
            {
                send_num = inference_data->model_data.result_num;
                memcpy(send_output,output_data,send_num*sizeof(float)); 
            }
            
            bms_device_result_send(model_type,send_output,send_num);
            
            printf("\r\n");
        }
    }

    heap_caps_free(input_data);                                                                      //释放推理数据窗口 
}



extern const unsigned char model_data1[];
inference_model_data_t *inference_model_create(model_t model_type,const unsigned char *model_data)
{
    //申请模型数据区域
    inference_model_data_t *inference_data = heap_caps_malloc(sizeof(inference_model_data_t), MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
    //初始化模型解释器
    inference_data->tflm.input_col = 0;
    inference_data->tflm.input_row = 0;
    inference_data->tflm.model_data = model_data;
    tflm_create(&inference_data->tflm);

    //初始化模型数据
    inference_data->type = model_type;
    model_config_t *model_cfg = NULL;
    switch(model_type)
    {
        case RSK_MODEL:
            model_cfg = &sys_config.rsk_config;
            break;
        case RUL_MODEL:
            model_cfg = &sys_config.rul_config;
            break;
        case SOC_MODEL:
            model_cfg = &sys_config.soc_config;
            break;
        case SOH_MODEL:
            model_cfg = &sys_config.soh_config;
            break;
        default:
            break;

    }
    if(model_cfg == NULL)
    {
        heap_caps_free(inference_data);
        return NULL;
    }

    inference_data->model_data.dx_num = sys_config.pack_config.cell_num;
    // inference_data->model_data.result_num = inference_data->tflm.result_num;
    // inference_data->model_data.input_wicket_col = inference_data->tflm.input_col;
    // inference_data->model_data.input_wicket_row = inference_data->tflm.input_row;
    inference_data->model_data.result_num = model_cfg->result;
    inference_data->model_data.input_wicket_col = model_cfg->column;
    inference_data->model_data.input_wicket_row = model_cfg->row;



    memcpy(inference_data->model_data.input_value_type, model_cfg->value_type, sizeof(model_cfg->value_type));
    inference_data->model_data.dx_data = heap_caps_malloc(sizeof(inference_dx_data_t) * inference_data->model_data.dx_num, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);

    //初始化电芯数据
    // int wicket_size = inference_data->tflm.input_col * inference_data->tflm.input_row * sizeof(float);
    // int result_size = inference_data->model_data.result_num * sizeof(float);
    int wicket_size = inference_data->model_data.input_wicket_col * inference_data->model_data.input_wicket_row * sizeof(float);
    int result_size = inference_data->model_data.result_num * sizeof(float);

    for(int i = 0; i < inference_data->model_data.dx_num; i++)
    {
        inference_data->model_data.dx_data[i].input_wicket_data = heap_caps_malloc(wicket_size, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
        inference_data->model_data.dx_data[i].inference_result = heap_caps_malloc(result_size, MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
        inference_data->model_data.dx_data[i].state = STANDBY;
        inference_data->model_data.dx_data[i].full_flag = WICKET_NOT_FULL;
        inference_data->model_data.dx_data[i].current_row = 0;
    }



    //模型数据初始化成功，放入模型数据链表
    for(int i = 0; i < MODEL_MAX_NUM; i++)
    {
        if(g_inference_data[i] == NULL)
        {
            g_inference_data[i] = inference_data;
            break;
        }
    }

    return inference_data;
}

void inference_model_release(model_t model_type)
{ 
    inference_model_data_t *inference_data = NULL;
    for(int i = 0; i < MODEL_MAX_NUM; i++)
    {
        if(g_inference_data[i]->type == model_type)
        {
            inference_data = g_inference_data[i];
            g_inference_data[i] = NULL;
            break;
        }
    }
    if(inference_data == NULL)      return;

    for(int i = 0; i < inference_data->model_data.dx_num; i++)
    {
        heap_caps_free(inference_data->model_data.dx_data[i].input_wicket_data);
        heap_caps_free(inference_data->model_data.dx_data[i].inference_result);
    }

    heap_caps_free(inference_data->model_data.dx_data);

    heap_caps_free(inference_data);

    tflm_release(&inference_data->tflm);
}


void inference_task_handler(void *pvParameters)
{
    // esp_task_wdt_config_t twdt_config = {
    //     .timeout_ms = 10000, // 设置合适的超时时间
    //     .trigger_panic = true // 超时时触发panic
    // };
    // ESP_ERROR_CHECK(esp_task_wdt_init(&twdt_config));
    // ESP_ERROR_CHECK(esp_task_wdt_add(NULL));
    while (1)
    {
        /* code */
        for (size_t i = 0; i < MODEL_MAX_NUM; i++)
        {
            if(g_inference_data[i] == NULL) continue;
            unsigned long  start_time = 0,end_time = 0;
            /* code */
            switch (g_inference_data[i]->type)
            {
            case SOC_MODEL:
                /* code */
                ESP_LOGI(TAG,"inference for soc start!");
                start_time = esp_timer_get_time();
                inference_task_callback(SOC_MODEL);
                end_time = esp_timer_get_time();
                ESP_LOGI(TAG,"inference for soc finish! using time is  %d ms!",(int)(end_time-start_time)/1000);
                // vTaskDelay(pdMS_TO_TICKS(10000));
                break;
            case SOH_MODEL:
                /* code */
                ESP_LOGI(TAG,"inference for soh start!");
                start_time = esp_timer_get_time();
                inference_task_callback(SOH_MODEL);
                end_time = esp_timer_get_time();
                ESP_LOGI(TAG,"inference for soh finish! using time is  %d ms!",(int)(end_time-start_time)/1000);
                break;
            case RSK_MODEL:
                /* code */
                ESP_LOGI(TAG,"inference for rsk start!");
                start_time = esp_timer_get_time();
                inference_task_callback(RSK_MODEL);
                end_time = esp_timer_get_time();
                ESP_LOGI(TAG,"inference for rsk finish! using time is  %d ms!",(int)(end_time-start_time)/1000);
                break;
            case RUL_MODEL:
                /* code */
                ESP_LOGI(TAG,"inference for rul start!");
                start_time = esp_timer_get_time();
                inference_task_callback(RUL_MODEL);
                end_time = esp_timer_get_time();
                ESP_LOGI(TAG,"inference for rul finish! using time is  %d ms!",(int)(end_time-start_time)/1000);
                break;
            
            default:
                break;
            }

        }
        
        
        
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
    // esp_task_wdt_delete(NULL); // 可选：任务结束前移除
    
}

void inference_init(void)
{
    /* 初始化TFLM :张量区域和使用方法*/
    tflm_init();    
    /* 申请数据内存 */
    if(sys_config.soc_config.use == USE)
    {
        inference_model_create(SOC_MODEL,model_data1);
    }

    if(sys_config.rsk_config.use == USE)
    {
        inference_model_create(RSK_MODEL,model_data1);
    }

    /*  创建推理任务  */
    xTaskCreatePinnedToCore(inference_task_handler, "inference_task", configMINIMAL_STACK_SIZE * 8, NULL, tskIDLE_PRIORITY + 4, NULL, 1);
}




