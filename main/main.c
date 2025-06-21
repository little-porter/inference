#include <stdio.h>

#include "wifi.h"
// #include "tflm.h"
#include "inference.h"
#include "bms.h"
#include "rs485.h"
#include "dtu_4g.h"
#include "littlefs_ops.h"
#include "config.h"
#include "led.h"
#include "data_interface.h"
#include "dl_rfft.h"

#define LED_GPIO       GPIO_NUM_1



uint8_t led_state_flag = 0;
void app_main(void)
{

    wifi_device_t wifi_device = {
        .wifi_config.sta.ssid     = "HONOR",
        .wifi_config.sta.password = "01366795",
    };
    // wifi_device_init(&wifi_device);
    wifi_init_softap(&wifi_device);

    littlefs_ops_init();
    config_init(&sys_config);

    inference_init();

    bms_device_init(&bms_device);
    rs485_driver_init(&rs485_driver);
    
    // dtu_4g_device_init(&dtu_4g_device);
    data_interface_init();

    led_init(&led_run,LED_GPIO,LED_DEFAULT_LEVEL_HIGH,LED_STATE_BLINK);
    while(true)
    {
        
        printf("Hello World\n");
        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}
