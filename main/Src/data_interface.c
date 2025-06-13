#include "data_interface.h"

#define MODEL_DATA_TYPE_NUM          11
#define MODEL_WICKET_ROW             280
#define MODEL_WICKET_COL             MODEL_DATA_TYPE_NUM

#define DX_WICKET_FULL               1
#define DX_WICKET_NOT_FULL           0


typedef struct _model_dx_data
{
    uint8_t   input_data_type[MODEL_DATA_TYPE_NUM];                 //模型窗口输入数据类型
    float     input_wicket[MODEL_WICKET_ROW][MODEL_WICKET_COL];     //模型窗口原始数据
    uint32_t  current_row;                                          //当前数据更新行
    uint8_t   full_flag;                                            //窗口数据满标志    
}model_dx_data_t;



typedef struct{
    // uint8_t   type;                             //模型类型0-3
    uint32_t  wicket_row;                       //模型窗口行数
    uint32_t  wicket_cols;                      //模型窗口列数

    uint8_t   dx_num;                           //电芯数量
    model_dx_data_t dx_data[PACK_DX_MAX_NUM];   //电芯数据
    
    uint32_t  data_get_cycle;                   //数据获取周期
}model_data_t;

// typedef struct _model_interfance
// {
//     model_data_t model_data[MODEL_MAX_NUM];
//     uint8_t      model_num;
// }model_interface_t;

model_data_t *model_data;

extern inference_model_data_t *inference_data;

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
    for(uint8_t i = 0; i < inference_data->model_num; i++)
    {
        uint32_t  wicket_size = sizeof(float) * inference_data->model_data[i].input_wicket_col * inference_data->model_data[i].input_wicket_row;
        for(uint8_t j = 0; j < inference_data->model_data[i].dx_num; j++)
        {
            if(model_data->dx_data[j].full_flag == DX_WICKET_NOT_FULL) continue;

            inference_data->model_data[i].dx_data[j].input_wicket_data = heap_caps_calloc(1,wicket_size,MALLOC_CAP_8BIT);

        }
    }
}

void inference_data_update_timer_init(void)
{
    TimerHandle_t xTimer = xTimerCreate(
        "infDataTimer",                             // 名称
        pdMS_TO_TICKS(180000),                      // m秒
        pdTRUE,                                     // 自动重载
        (void *)0,                                  // 回调函数参数
        inference_data_update_timer_callback        // 回调函数
    );
}

void data_interface_init(void)
{

    
}




