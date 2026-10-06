#include "buzzer.h"

volatile uint8_t buzzer_state = BUZZER_OFF;
volatile uint8_t buzzer_error_state = BUZZER_WAIT_FIRST_ON;
volatile uint32_t buzzer_duration;
volatile uint32_t buzzer_start_time;

volatile uint32_t buzzer_wait_duration;
volatile uint32_t buzzer_wait_start_time;

void buzzer_init(void)
{
}

static void buzzer_on(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, SET);
}

static void buzzer_off(void)
{
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, RESET);
}

static void buzzer_toggle(void)
{
    HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
}

void buzzer_beep(uint32_t ms)
{
    buzzer_on();
    buzzer_duration = ms;
    buzzer_start_time = HAL_GetTick();
    buzzer_state = BUZZER_ON;
}

void buzzer_task(void)
{
    switch (buzzer_state)
    {
    case BUZZER_ON:
    {
        if (HAL_GetTick() - buzzer_start_time >= buzzer_duration)
        {
            buzzer_off();
            buzzer_state = BUZZER_OFF;
        }
        break;
    }
    case BUZZER_OFF:
    {
        break;
    }
    default:
    {
        buzzer_state = BUZZER_OFF;
        buzzer_off();
        break;
    }
    }
}

void buzzer_success(void)
{
    buzzer_beep(100);
}

void buzzer_error(void)
{
    buzzer_on();
    buzzer_duration = 100;
    buzzer_start_time = HAL_GetTick();
    buzzer_error_state = BUZZER_FIRST_ON;
}

void buzzer_task_error(void)
{
    switch (buzzer_error_state)
    {
    case BUZZER_WAIT_FIRST_ON:
    {
        break;
    }
    case BUZZER_FIRST_ON:
    {
        if (HAL_GetTick() - buzzer_start_time >= buzzer_duration)
        {
            buzzer_error_state = BUZZER_WAIT_SECOND_ON;
            buzzer_off();
            buzzer_wait_start_time = HAL_GetTick();
            buzzer_wait_duration = 100;
        }
        break;
    }
    case BUZZER_WAIT_SECOND_ON:
    {
        if (HAL_GetTick() - buzzer_wait_start_time >= buzzer_wait_duration)
        {
            buzzer_on();
            buzzer_start_time = HAL_GetTick();
            buzzer_error_state = BUZZER_SECOND_ON;
            buzzer_duration = 100;
        }
        break;
    }
    case BUZZER_SECOND_ON:
    {
        if (HAL_GetTick() - buzzer_start_time >= buzzer_wait_duration)
        {
            buzzer_off();
            buzzer_error_state = BUZZER_WAIT_FIRST_ON;
        }
        break;
    }
    default:
    {
        buzzer_off();
        buzzer_error_state = BUZZER_WAIT_FIRST_ON;
        break;
    }
    }
}
