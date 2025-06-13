#include <stdio.h>

#include "wifi.h"
// #include "tflm.h"
#include "inference.h"
#include "bms.h"
#include "rs485.h"
#include "dtu_4g.h"

#define LED_GPIO       GPIO_NUM_1
void led_init(void){
     // 初始化输出GPIO
    esp_rom_gpio_pad_select_gpio(LED_GPIO);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // 禁用中断
    io_conf.mode = GPIO_MODE_OUTPUT; // 设置为输出模式
    io_conf.pin_bit_mask = (1ULL << LED_GPIO); // 选择具体的GPIO
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 禁用下拉电阻
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; // 禁用上拉电阻
    gpio_config(&io_conf);

}

uint8_t led_state_flag = 0;
void app_main(void)
{

    wifi_device_t wifi_device = {
        .wifi_config.sta.ssid     = "HONOR",
        .wifi_config.sta.password = "01366795",
    };
    // wifi_device_init(&wifi_device);

    inference_init();

    bms_device_init(&bms_device);
    rs485_driver_init(&rs485_driver);
    
    dtu_4g_device_init(&dtu_4g_device);

    led_init();
    while(true)
    {
        if(led_state_flag){
        led_state_flag = 0;
        }else{
        led_state_flag = 1 ;
        }
        gpio_set_level(LED_GPIO, led_state_flag);

        printf("Hello World\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
