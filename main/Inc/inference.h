#ifndef __INFERENCE_H__
#define __INFERENCE_H__

#include "project_system.h"
#include "tflm.h"
#include "config.h"



typedef enum {
    SOH_MODEL = 0,      //SOH模型
    SOC_MODEL = 1,      //SOC模型
    RUL_MODEL = 2,      //RUL模型
    RSK_MODEL = 3,      //热失控模型
}model_t;

typedef enum
{
    STANDBY = 0,
    BEGIN_INTERENCE = 1, 
}inf_state_t;

//定义模型
// typedef struct{
//     float     *input_wicket_data[MODEL_MAX_NUM];                        //推理输入数据
//     float     inference_result[MODEL_MAX_NUM][PACK_DX_MAX_NUM][2];      //推理结果


//     Message   message[MODEL_NUM][PACK_DX_NUM];                   //模型消息
//     unsigned int  corrent_get_cycle[MODEL_NUM][PACK_DX_NUM];     //周期计数
//     uint8_t   update_model_ifo[MODEL_NUM] ;                      //模型信息更新
//     uint8_t   inference_idex[MODEL_NUM];
    
//     uint8_t   start_using[MODEL_NUM] ;                           //模型启用
//     uint8_t   update_use_ifo ;                                   //启用信息
//     uint8_t   communication_port_use[TX_PORT_NUM];
//     uint8_t   data_from[DATA_IN_SET_NUM];                        //数据来源
//     uint8_t   data_out[DATA_OUT_SET_NUM];                        //数据输出
//     uint8_t   result_out[RESULT_OUT_SET_NUM];                    //结果输出
//     uint8_t   sys_first_time;                                    //系统参数初始化标志
//     int_char  device_id ;
//     int_char  cd_fd_times[PACK_DX_NUM];                          //充放电次数
//     int_char  rated_capacity;                                    //额定容量
// }inference_describe_t;


typedef enum _wicket_full_state
{
    WICKET_NOT_FULL = 0,
    WICKET_FULL = 1,
}wicket_full_state_t;


/*单组推理数据定义*/
typedef struct _inference_data
{
    /* data */
    float       *input_wicket_data;          //推理输入数据
    float       *inference_result;           //推理结果
    uint32_t    current_row;
    wicket_full_state_t full_flag;
    inf_state_t state;
}inference_data_t;

/*电芯推理数据定义*/
typedef struct _inference_dx_data
{
    inference_data_t *dx_data;
    uint32_t  dx_num;
    uint32_t  input_wicket_row;
    uint32_t  input_wicket_col;
    uint32_t  result_num;
    uint16_t  input_value_type[13];  
}inference_dx_data_t;

/*模型推理数据定义*/
typedef struct _inference_describe
{
    inference_dx_data_t model_data;
    tflm_module_t tflm;
    model_t type;
}inference_model_data_t;


void inference_init(void);

#endif

