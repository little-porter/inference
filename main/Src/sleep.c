#include "sleep.h"
#include "esp_sleep.h"
#include "lis3dh.h"


#define WAKEUP_PIN_1 GPIO_NUM_0
#define WAKEUP_PIN_2 GPIO_NUM_8

#define get_device_wake_state()  (gpio_get_level(WAKEUP_PIN_2) == 1)

void sleep_task_handler(void *pvParameters);
void sleep_init(void)
{
    // 配置 WAKEUP_PIN_2 为输入模式，启用上拉电阻
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << WAKEUP_PIN_2), 
        .mode = GPIO_MODE_INPUT,               // 输入模式
        .pull_up_en = GPIO_PULLUP_ENABLE,      // 使能上拉电阻
        .pull_down_en = GPIO_PULLDOWN_DISABLE, // 禁用下拉电阻
        .intr_type = GPIO_INTR_DISABLE,        // 禁用中断
    };
    gpio_config(&io_conf);

    // 配置 EXT0 唤醒（WAKEUP_PIN_2 低电平触发）
    esp_sleep_enable_ext0_wakeup(WAKEUP_PIN_2, 0);

    //加速度传感初始化
    lis3dh_driver_init(&lis3dh_device);

    xTaskCreatePinnedToCore(sleep_task_handler, "sleep_task", 1024*2, NULL, 5, NULL, 0);
}


void sleep_task_handler(void *pvParameters)
{
    uint16_t sleeptime = 0;
    while(1)
    {
        if(get_device_wake_state())
        {
            sleeptime++;
        }
        else
        {
            sleeptime = 0;
        }

        if(sleeptime >= 100)
        {
            sleeptime = 0;
            vTaskDelay(pdMS_TO_TICKS(1000)); 
            //进入深度睡眠模式
            esp_deep_sleep_start();
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}



