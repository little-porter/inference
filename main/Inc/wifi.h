#ifndef __WIFI_H__
#define __WIFI_H__

#include "project_system.h"
#include "esp_wifi.h"

#define WIFI_STA_DISCONNECTED_BIT       (1<<0)      //作为STA模式时，WIFI断开连接
#define WIFI_STA_CONNECTED_BIT          (1<<1)      //作为STA模式时，WIFI连接成功
#define WIFI_STA_GET_IP_BIT             (1<<2)      //作为STA模式时，WIFI获取到IP地址


typedef enum _wifi_status_t
{
    WIFI_STATUS_DISCONNECTED = 0,
    WIFI_STATUS_CONNECTED    = 1,
}wifi_status_t;


typedef struct _wifi_device_t
{
    wifi_config_t       wifi_config;
    wifi_status_t       status;
    EventGroupHandle_t  event;
}wifi_device_t;

void wifi_device_init(wifi_device_t *wifi_dev);

void wifi_init_softap(wifi_device_t *wifi_dev);

#endif
