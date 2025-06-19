#pragma once

#include "project_system.h"

#include "driver/gpio.h"

typedef enum _led_state_t
{
    LED_STATE_OFF,
    LED_STATE_ON,
    LED_STATE_BLINK,
    LED_STATE_BLINK_FAST,
    LED_STATE_BLINK_SLOW,
}led_state_t;

typedef enum _led_default_level_t
{
    LED_DEFAULT_LEVEL_LOW,
    LED_DEFAULT_LEVEL_HIGH,
}led_default_level_t;


typedef struct _led_t
{
    /* data */
    led_state_t state;
    led_default_level_t default_level;
    gpio_num_t  pin;
}led_t;

extern led_t led_run;

void led_init(led_t *led, gpio_num_t pin, led_default_level_t level,led_state_t state);


