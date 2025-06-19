#include  "led.h"

static const char *TAG = "PRJ_LED";

#define LED_MAX_NUM 10

led_t *led_list[LED_MAX_NUM] = {NULL};

led_t led_run;

TaskHandle_t led_task_handle = NULL;

void led_task_handler(void *pvParameters)
{
    while(1)
    {
        for(int i = 0; i < LED_MAX_NUM; i++)
        {
            if(led_list[i] != NULL)
            {
                switch(led_list[i]->state)
                {
                    case LED_STATE_ON:
                        if(led_list[i]->default_level == LED_DEFAULT_LEVEL_HIGH)
                            gpio_set_level(led_list[i]->pin, 1);
                        else
                            gpio_set_level(led_list[i]->pin, 0);
                        break;
                    case LED_STATE_OFF:
                       if(led_list[i]->default_level == LED_DEFAULT_LEVEL_HIGH)
                            gpio_set_level(led_list[i]->pin, 1);
                        else
                            gpio_set_level(led_list[i]->pin, 0);
                        break;
                    case LED_STATE_BLINK:
                        int state = gpio_get_level(led_list[i]->pin);
                        gpio_set_level(led_list[i]->pin, !state);
                        break;
                    default:
                        break;
                }
            }
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void led_init(led_t *led, gpio_num_t pin, led_default_level_t level,led_state_t state)
{
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE; // 禁用中断
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT; // 设置为输出模式
    io_conf.pin_bit_mask = (1ULL << pin); // 选择具体的GPIO
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE; // 禁用下拉电阻
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; // 禁用上拉电阻
    gpio_config(&io_conf);

    gpio_set_level(pin, level);
    led->pin = pin;
    led->state = state;
    led->default_level = level;

    for (int i = 0; i < LED_MAX_NUM; i++)
    {
        if(led_list[i] == NULL)
        {
            led_list[i] = led;
            break;
        }
    } 

    if(led_task_handle == NULL)
        xTaskCreatePinnedToCore(led_task_handler, "led_task", 1024, NULL, 5, &led_task_handle, 0);
}





