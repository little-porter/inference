#include <stdio.h>

#include "wifi.h"
// #include "tflm.h"

void app_main(void)
{

    wifi_device_t wifi_device = {
        .wifi_config.sta.ssid     = "HONOR",
        .wifi_config.sta.password = "01366795",
    };
    wifi_device_init(&wifi_device);


    while(true)
    {
        printf("Hello World\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
