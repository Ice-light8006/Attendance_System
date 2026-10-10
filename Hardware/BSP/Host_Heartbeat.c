#include "Host_Heartbeat.h"
#include "usart.h"

static volatile uint8_t ping_pending;
static volatile uint8_t ping_sequence[2];

void Host_Heartbeat_OnFrame(const uint8_t *data, uint8_t len)
{
    if (data == 0 || len != 2U || ping_pending != 0U)
    {
        return;
    }
    ping_sequence[0] = data[0];
    ping_sequence[1] = data[1];
    ping_pending = 1U;
}

void Host_Heartbeat_Task(void)
{
    uint8_t sequence[2];

    if (ping_pending == 0U)
    {
        return;
    }
    sequence[0] = ping_sequence[0];
    sequence[1] = ping_sequence[1];
    ping_pending = 0U;
    uploadToHost(HOST_PONG, sequence, sizeof(sequence));
}
