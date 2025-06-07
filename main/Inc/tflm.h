#ifndef __TFLM_H__
#define __TFLM_H__

#include "project_system.h"

#ifdef __cplusplus
extern "C" {
#endif




void tflm_init(void);
void tflm_run(uint8_t model_type,float *input_data,uint32_t input_num,float *output_data);

// typedef struct _tflm_module
// {
//     /* data */
//     tflite::MicroInterpreter *interpreter;      //解释器
//     const unsigned char *model_data;            //模型数据
//     uint8_t *tf_area;                           //张量区域
// }tflm_module_t;

// extern tflm_module_t tflm_soc;
// extern tflm_module_t tflm_soh;
// extern tflm_module_t tflm_rul;
// extern tflm_module_t tflm_rsk;

#ifdef __cplusplus
}
#endif

#endif





