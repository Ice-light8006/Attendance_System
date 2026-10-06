#ifndef LED_H
#define LED_H

#define LED1 1
#define LED2 2

#include <stdint.h>
#include <main.h>

void turn_on_led(uint8_t led);

void turn_off_led(uint8_t led);

void toggle_led(uint8_t led);

#endif
