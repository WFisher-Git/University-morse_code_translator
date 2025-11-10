
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "stdlib.h"
#include "includes/seven_segment.h"
#include "includes/buzzer.h"
#include "includes/potentiometer.h"

// define variables and pins
#define RED 13 
#define GREEN 12 
#define BRIGHTNESS 20
#define MAX_COLOUR_VALUE 255
#define MAX_PWM_LEVEL 65535

void setup_rgb()
{
    gpio_set_function(RED, GPIO_FUNC_PWM);
    gpio_set_function(GREEN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(RED);
    pwm_config config = pwm_get_default_config();
    pwm_init(slice_num, &config, true);
    slice_num = pwm_gpio_to_slice_num(GREEN);
    pwm_init(slice_num, &config, true);
}


void show_rgb(int r, int g)
{
    pwm_set_gpio_level(RED, ~(MAX_PWM_LEVEL * r / MAX_COLOUR_VALUE * BRIGHTNESS / 100));
    pwm_set_gpio_level(GREEN, ~(MAX_PWM_LEVEL * g / MAX_COLOUR_VALUE * BRIGHTNESS / 100));
}
