#include "wifi.h"
#include "nvs_flash.h"


static const char *TAG = "PRJ_WIFI";

//wifi 事件
// static void wifi_event_handler(void* arg, esp_event_base_t event_base,
//                                      int32_t event_id, void* event_data)
// {
//     wifi_device_t *wifi_dev = (wifi_device_t *)arg;

//     switch (event_id)
//     {
//     case WIFI_EVENT_AP_STACONNECTED:  //AP模式-有STA连接成功
//         // 作为ap，有sta连接
// //      ESP_LOGI(TAG, "station:" MACSTR " join,AID=%d\n",MAC2STR(event->event_info.sta_connected.mac),event->event_info.sta_connected.aid);
//         //设置事件位
        
//         break;
//     case WIFI_EVENT_AP_STADISCONNECTED://AP模式-有STA断线
// //      ESP_LOGI(TAG, "station:" MACSTR "leave,AID=%d\n",MAC2STR(event->event_info.sta_disconnected.mac),event->event_info.sta_disconnected.aid);
//         //重新建立server
//         xEventGroupClearBits(wifi_dev->event, WIFI_STA_DISCONNECTED_BIT);
//         break;

//     case WIFI_EVENT_STA_START:
//         ESP_LOGI(TAG, "WIFI_STA_START!");
//         // esp_wifi_connect();
//         break;
//     case WIFI_EVENT_STA_DISCONNECTED:
//         ESP_LOGI(TAG, "WIFI_STA_DISCONNECTED!");
//         wifi_dev->status = WIFI_STATUS_DISCONNECTED;
//         esp_wifi_connect();
//         break;
//     case WIFI_EVENT_STA_CONNECTED:
//         ESP_LOGI(TAG, "WIFI_STA_CONNECTED!");
//         xEventGroupSetBits(wifi_dev->event, WIFI_STA_CONNECTED_BIT);
//         break;

//     case IP_EVENT_STA_GOT_IP:
//         ESP_LOGI(TAG, "WIFI_STA_GET_IP!");
//         xEventGroupSetBits(wifi_dev->event, WIFI_STA_GET_IP_BIT);
//         wifi_dev->status = WIFI_STATUS_CONNECTED;
//         break;
//     default:
//         break;
//     }
// }
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data);
void wifi_device_init(wifi_device_t *wifi_dev)
{
    //初始化事件组
    wifi_dev->event = xEventGroupCreate();

     //初始化FLASH
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);


    //初始化TCP/IP底层栈
    ESP_ERROR_CHECK(esp_netif_init());
    //创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    //创建默认WIFI STA。在任何初始化错误的情况下，此API中止。
    esp_netif_create_default_wifi_sta();
    
    

    //WiFi栈配置参数传递给esp_wifi_init调用。
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    //为WIFI任务分配资源
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    //将事件处理程序的实例注册到默认循环。
    //这个函数的功能与esp_event_handler_instance_register_with相同，
    //只是它将处理程序注册到默认的事件循环。
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        wifi_dev,
                                                        NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        wifi_dev,
                                                        NULL));


    //设置WIFI工作模式为STA模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    //设置STA模式的配置
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_dev->wifi_config));
    //开启WIFI
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_sta finished. SSID:%s password:%s ",
             wifi_dev->wifi_config.sta.ssid, wifi_dev->wifi_config.sta.password);

}

#define EXAMPLE_ESP_WIFI_SSID      "ESP32S31111111"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
// #define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
// #define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN


#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_mac.h"
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d, reason=%d",
                 MAC2STR(event->mac), event->aid, event->reason);
    }
}



void wifi_init_softap(wifi_device_t *wifi_dev)
{
    //初始化事件组
    wifi_dev->event = xEventGroupCreate();

    esp_err_t ret = nvs_flash_init();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        wifi_dev,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .password = EXAMPLE_ESP_WIFI_PASS,
            .authmode = WIFI_AUTH_WPA3_PSK,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
            .max_connection = 4,
            .pmf_cfg = {
                    .required = true,
            },
            },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

}
