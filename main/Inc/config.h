#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "project_system.h"

// #define USE     1
// #define NOT_USE 0

#define OTA_FLAG_TURE  1
#define OTA_FLAG_FALSE 0

#define SLEEP_MODE_ON  1
#define SLEEP_MODE_OFF 0
#define DEFAULT_DEV_ID 0x00000001

typedef enum _device_use
{
    NOT_USE = 0,
    USE     = 1,
}device_use_t;


typedef enum _model_data_source
{
    MODEL_DATA_SOURCE_BMS  = 0,
    MODEL_DATA_SOURCE_COMM = 1,
    MODEL_DATA_SOURCE_SELF = 2,
}model_data_source_t;

typedef enum _model_output_port
{
    MODEL_OUTPUT_PORT_WIFI    = 0,
    MODEL_OUTPUT_PORT_DTU_4G  = 1,
    MODEL_OUTPUT_PORT_RS485   = 2,
    MODEL_OUTPUT_PORT_CAN     = 3,
}model_output_port_t;

typedef struct _comm_config
{
    /* data */
    uint8_t RS485_comm_use;
    uint8_t CAN_comm_use;
    uint8_t WIFI_comm_use;
    uint8_t BLUE_comm_use;
    uint8_t DTU_4G_comm_use;
}comm_config_t;

typedef struct _can_config
{
    uint32_t baudrate;
    device_use_t use;
}can_config_t;

typedef struct _rs485_config
{
    uint32_t baudrate;
    device_use_t use;
}rs485_config_t;

typedef struct _dtu_4g_config
{
    uint32_t baudrate;
    device_use_t use;
}dtu_4g_config_t;

typedef struct _wifi_device_config
{
    uint8_t  ssid[32];
    uint8_t  password[32];
    device_use_t  use;
}wifi_device_config_t;

typedef struct _model_config
{
    uint8_t type;
    uint8_t value_type[13];
    char    file_name[32];
    uint16_t row;
    uint16_t column;
    uint16_t result;
    device_use_t use;
}model_config_t;

typedef struct _pack_config
{
    uint8_t cell_num;
    float   capacity;
}pack_config_t;

typedef struct _sys_config
{
    /* data */
    uint8_t  ota_flag;
    uint8_t  sleep_mode;

    uint8_t  protocol_ver[3];
    uint32_t dev_id;
    uint8_t  dev_type[3];
    uint8_t  dev_ver;

    comm_config_t       comm_config;
    model_data_source_t model_input_source;
    model_output_port_t model_output_port;
    model_output_port_t model_result_port;

    rs485_config_t rs485_config;
    can_config_t   can_config;
    dtu_4g_config_t dtu_4g_config;
    wifi_device_config_t  wifi_config;

    model_config_t soh_config;
    model_config_t soc_config;
    model_config_t rul_config;
    model_config_t rsk_config;

    pack_config_t pack_config;
}sys_config_t;

extern sys_config_t sys_config;


void config_init(sys_config_t *config);

#endif
