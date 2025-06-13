#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "project_system.h"

#define USE     1
#define NOT_USE 0

typedef enum _model_data_source
{
    MODEL_DATA_SOURCE_BMS  = 0,
    MODEL_DATA_SOURCE_COMM = 1,
    MODEL_DATA_SOURCE_SELF = 2,
}model_data_source_t;


typedef struct _model_config
{
    /* data */
    uint8_t SOH_model_use;
    uint8_t SOC_model_use;
    uint8_t RUL_model_use;
    uint8_t RSK_model_use;  
}model_config_t;

typedef struct _comm_config
{
    /* data */
    uint8_t RS485_comm_use;
    uint8_t CAN_comm_use;
    uint8_t WIFI_comm_use;
    uint8_t BLUE_comm_use;
    uint8_t DTU_4G_comm_use;
}comm_config_t;


typedef struct _sys_config
{
    /* data */
    uint8_t  protocol_ver[3];
    uint32_t dev_id;
    uint8_t  dev_type[3];
    uint8_t  dev_ver;

    model_config_t      model_config;
    comm_config_t       comm_config;
    model_data_source_t model_input_source;
}sys_config_t;

extern sys_config_t sys_config;

#endif
