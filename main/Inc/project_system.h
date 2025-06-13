#ifndef __PROJECT_SYSTEM_H__
#define __PROJECT_SYSTEM_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <string.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_heap_caps.h"
#include "esp_timer.h"

#include "driver/gpio.h"


#define MODEL_MAX_NUM               4
#define PACK_DX_MAX_NUM             20


#define USE_BMS_DATA
#define USE_TESTDEV_DATA
extern QueueHandle_t  bms_rx_queue;
typedef struct _bms_msg 
{
    uint8_t  data[200];         //数据
    uint16_t num;               //数据长度
}bms_msg_t;







#ifdef __cplusplus
}
#endif

#endif
