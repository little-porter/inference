#include "data_interface.h"
#include "bms.h"

#define MODEL_DATA_TYPE_NUM          13

#define MODEL_WICKET_ROW             280
#define MODEL_WICKET_COL             MODEL_DATA_TYPE_NUM

// #define DX_WICKET_FULL               1
// #define DX_WICKET_NOT_FULL           0

typedef enum {
    DATA_TYPE_CFDLC = 0 ,           //充放电次数
    DATA_TYPE_CFDZT = 1 ,           //充放电状态
    DATA_TYPE_DY    = 2 ,           //电压
    DATA_TYPE_DL    = 3 ,           //电流
    DATA_TYPE_CFDL  = 4 ,           //充放电率
    DATA_TYPE_CFDT  = 5 ,           //充放电时间
    DATA_TYPE_DCLZ  = 6 ,           //电池内阻
    DATA_TYPE_IC    = 7 ,           //增量容量
    DATA_TYPE_SOC   = 8 ,           //荷电状态
    DATA_TYPE_RUL   = 9 ,           //剩余寿命
    DATA_TYPE_SOH   = 10 ,          //健康状态
    DATA_TYPE_RES   = 11 ,          //电池内阻 
    DATA_TYPE_TEMP  = 12 ,          //电池温度
    // RSK_FLAG,                    //热失控率
}data_type_flag;


typedef enum _data_update_flag
{
    DATA_UPDATE_FLAG_FALSE = 0,
    DATA_UPDATE_FLAG_TRUE = 1,
}data_update_flag_t;


typedef struct _model_dx_data
{
    float     real_data[MODEL_WICKET_COL];                                            //模型窗口原始数据
    data_update_flag_t update_flag;   
}model_dx_data_t;



typedef struct{
    uint8_t   dx_num;                           //电芯数量
    model_dx_data_t dx_data[PACK_DX_MAX_NUM];   //电芯数据
    uint32_t  data_get_cycle;                   //数据获取周期
}model_data_t;

// typedef struct _model_interfance
// {
//     model_data_t model_data[MODEL_MAX_NUM];
//     uint8_t      model_num;
// }model_interface_t;

model_data_t model_data;

void model_data_uniformization(float *interence_wicket,uint32_t row,uint32_t col, float *dx_data,uint32_t now_row)
{
    // if(dx_data->full_flag == DX_WICKET_NOT_FULL) return;
    float *min = (float *)heap_caps_calloc(1,col*sizeof(float),MALLOC_CAP_8BIT);
    float *max = (float *)heap_caps_calloc(1,col*sizeof(float),MALLOC_CAP_8BIT);
    float *temp = (float *)heap_caps_calloc(1,col*sizeof(float),MALLOC_CAP_8BIT);

    float (*p_dx_data)[col] = dx_data;
    float (*p_wicket_data)[col] = interence_wicket;

    for(int  i = 0; i < col; i++)
    {
        min[i] = p_dx_data[0][i];
        max[i] = p_dx_data[0][i];
    }
    /*寻找最大值、最小值*/
    for(int  i = 0; i < col; i++)
    {
        for(int j = 0; j < row; j++)
        {
            if(p_dx_data[j][i] < min[i])
            {
                min[i] = p_dx_data[j][i];
            }
            if(p_dx_data[j][i] > max[i])
            {
                max[i] = p_dx_data[j][i];
            }
        }
        temp[i] = (max[i] - min[i]);
    }

    for(int i = 0; i < row; i++)
    {
        uint32_t index_row = 0;
        if(now_row+i > row)
        {
            index_row = now_row + i - row;
        }
        else
        {
            index_row = now_row + i;
        }

        for(int j = 0; j < col; j++)
        {
            if(temp[j] == 0)
            {
                p_wicket_data[i][j] = 99.0/100;
            }
            else
            {
                p_wicket_data[i][j] = (p_dx_data[i][j] - min[j])/temp[j];
            }
        }

    }

}



void inference_data_update_timer_callback(void *pvParameters)
{
    extern inference_model_data_t *g_inference_data[MODEL_MAX_NUM];
    for(uint8_t i = 0; i < MODEL_MAX_NUM; i++)
    {
        if(g_inference_data[i] == NULL)     continue;

        for(uint32_t j = 0; j < g_inference_data[i]->model_data.dx_num; j++)
        {
            if(model_data.dx_data[j].update_flag != DATA_UPDATE_FLAG_TRUE)     continue;
           
            model_data.dx_data[j].update_flag = DATA_UPDATE_FLAG_FALSE;

            uint32_t row = g_inference_data[i]->model_data.dx_data[j].current_row;
            uint32_t col = g_inference_data[i]->model_data.input_wicket_col;
            for(uint32_t k = 0; k < g_inference_data[i]->model_data.input_wicket_col; k++)
            {
                g_inference_data[i]->model_data.dx_data[j].input_wicket_data[row*col+k] = model_data.dx_data[j].real_data[g_inference_data[i]->model_data.input_value_type[k]];        //按照数据类型为模型填充真实数据
            }

            g_inference_data[i]->model_data.dx_data[j].current_row++;
            g_inference_data[i]->model_data.dx_data[j].current_row %= g_inference_data[i]->model_data.input_wicket_row;

            if(g_inference_data[i]->model_data.dx_data[j].current_row == 0)
            {
                g_inference_data[i]->model_data.dx_data[j].full_flag = WICKET_FULL;
            }
           

            if((g_inference_data[i]->model_data.dx_data[j].full_flag == WICKET_NOT_FULL) && j == 0)
                printf("dx_num:%d,row:%d,col:%d\n",(int)j,(int)row,(int)col);
        }
        
    }
}

void inference_data_update_timer_init(void)
{
    TimerHandle_t xTimer = xTimerCreate(
        "infDataTimer",                             // 名称
        pdMS_TO_TICKS(1000),                      // m秒
        pdTRUE,                                     // 自动重载
        (void *)0,                                  // 回调函数参数
        inference_data_update_timer_callback        // 回调函数
    );
    xTimerStart(xTimer, 0);
}

void data_real_update_task_handler(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if(sys_config.model_input_source == MODEL_DATA_SOURCE_BMS)
        {
            if(bms_device.status == BMS_OFFLINE)  continue;;

            // if(model_data == NULL) continue;

            for(uint32_t i = 0; i < model_data.dx_num; i++)
            {
                // uint16_t row = model_data->dx_data[i].current_row;

                model_data.dx_data[i].real_data[DATA_TYPE_CFDLC] = bms_device.pack_cfd_lc;
                model_data.dx_data[i].real_data[DATA_TYPE_CFDZT] = bms_device.pack_cfd_zt;
                model_data.dx_data[i].real_data[DATA_TYPE_DY] = bms_device.cell_voltage[i] / 100;
                model_data.dx_data[i].real_data[DATA_TYPE_DL] = bms_device.pack_fCurrent;
                model_data.dx_data[i].real_data[DATA_TYPE_CFDL] = bms_device.pack_cfd_Crate;
                model_data.dx_data[i].real_data[DATA_TYPE_CFDT] = bms_device.pack_cfd_time;
                // model_data->dx_data[i].input_wicket[row][DATA_TYPE_DCLZ] = bms_device.pack_cfd_zt;
                model_data.dx_data[i].real_data[DATA_TYPE_IC] = bms_device.pack_cfd_ic;
                // model_data->dx_data[i].input_wicket[row][DATA_TYPE_SOC] = bms_device.;
                // model_data->dx_data[i].input_wicket[row][DATA_TYPE_RUL] = bms_device.;
                model_data.dx_data[i].real_data[DATA_TYPE_SOH] = bms_device.pack_SOH_estimate;
                model_data.dx_data[i].real_data[DATA_TYPE_RES] = bms_device.pack_cfd_inRes / 100;
                model_data.dx_data[i].real_data[DATA_TYPE_TEMP] = bms_device.high_temp_cell;

                // model_data->dx_data[i].current_row++;
                model_data.dx_data[i].update_flag = DATA_UPDATE_FLAG_TRUE;

            }

            

        }

        
    }
}

void data_interface_init(void)
{
    // model_data = heap_caps_malloc(sizeof(model_data_t), MALLOC_CAP_8BIT|MALLOC_CAP_SPIRAM);
    model_data.dx_num = sys_config.pack_config.cell_num;

    for(uint32_t j = 0; j < PACK_DX_MAX_NUM; j++)
        model_data.dx_data[j].update_flag = DATA_UPDATE_FLAG_FALSE;

    xTaskCreatePinnedToCore(data_real_update_task_handler, "data_real_task", 1024*5, NULL, 7, NULL, 0);

    inference_data_update_timer_init();
}















