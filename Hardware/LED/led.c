#include "led.h"

void turn_on_led(uint8_t led)
{
    switch(led)
    {
        case LED1:
        {
            HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_RESET);
            break;
        }
        case LED2:
        {
            HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_RESET);
            break;
        }
    }
}

void turn_off_led(uint8_t led)
{
    switch(led)
    {
        case LED1:
        {
            HAL_GPIO_WritePin(LED1_GPIO_Port,LED1_Pin,GPIO_PIN_SET);
            break;
        }
        case LED2:
        {
            HAL_GPIO_WritePin(LED2_GPIO_Port,LED2_Pin,GPIO_PIN_SET);
            break;
        }
    }
}

void toggle_led(uint8_t led)
{
    switch(led)
    {
        case LED1:
        {
            HAL_GPIO_TogglePin(LED1_GPIO_Port,LED1_Pin);
            break;
        }
        case LED2:
        {
            HAL_GPIO_TogglePin(LED2_GPIO_Port,LED2_Pin);
            break;
        }
    }
}
