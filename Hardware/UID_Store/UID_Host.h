#ifndef UID_HOST_H
#define UID_HOST_H

#include <stdint.h>

/* 0xA9 is reserved for the UID whitelist in the existing command list. */
#define UID_HOST_LIST          0xA9U
#define UID_HOST_RESULT        0xAAU

#define UID_HOST_QUERY         0xB3U
#define UID_HOST_INSERT        0xB4U
#define UID_HOST_UPDATE        0xB5U
#define UID_HOST_DELETE        0xB6U
#define UID_HOST_REQUEST_LIST  0xB7U

/* Called after USART3 initialization. Sends the 32-byte occupancy bitmap. */
void UID_Host_Init(void);

/* Called only after a complete USART3 host frame is parsed in the ISR. */
void UID_Host_OnFrame(uint8_t cmd, const uint8_t *data, uint8_t len);

/* Called in the main loop; Flash operations and replies happen here. */
void UID_Host_Task(void);

#endif /* UID_HOST_H */
