#ifndef BUZZER_H
#define BUZZER_H

#include <stdint.h>
#include "main.h"
#include "gpio.h"

extern volatile uint8_t buzzer_state;
extern volatile uint32_t buzzer_duration;
extern volatile uint32_t buzzer_start_time;

enum Buzzer_States
{
    BUZZER_ON,
    BUZZER_OFF
};

enum Buzzer_Error_States
{
    BUZZER_WAIT_FIRST_ON,
    BUZZER_FIRST_ON,
    BUZZER_WAIT_SECOND_ON,
    BUZZER_SECOND_ON
};

void buzzer_init(void);

static void buzzer_on(void);

static void buzzer_off(void);

static void buzzer_toggle(void);

void buzzer_beep(uint32_t ms);

void buzzer_task(void);

void buzzer_success(void);

void buzzer_error(void);

#endif
