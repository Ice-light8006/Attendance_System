#ifndef BSP_H
#define BSP_H

#define IDLE_MODE 0xA1
#define ENROLL_MODE 0xA2
#define TOGGLE_MODE 0xB1

void bsp_init();

void bsp_loop();

#endif
