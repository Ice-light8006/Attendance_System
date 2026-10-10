#ifndef HOST_HEARTBEAT_H
#define HOST_HEARTBEAT_H

#include <stdint.h>

#define HOST_PING 0xB8U
#define HOST_PONG 0xABU

/* Cache the ping received in the USART3 interrupt. */
void Host_Heartbeat_OnFrame(const uint8_t *data, uint8_t len);

/* Send the pong from the main loop, outside the interrupt. */
void Host_Heartbeat_Task(void);

#endif /* HOST_HEARTBEAT_H */
