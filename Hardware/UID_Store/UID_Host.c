#include "UID_Host.h"
#include "UID_Store.h"
#include "usart.h"

/* One pending command is enough because the host requests one UID at a time. */
static volatile uint8_t pending_state;
static volatile uint8_t pending_cmd;
static volatile uint8_t pending_len;
static volatile uint8_t pending_data[5];
static uint8_t store_status = UID_STORE_ERR_NOT_INIT;

static void send_list(void)
{
    uint8_t occupied[UID_STORE_SLOT_COUNT];
    uint8_t bitmap[UID_STORE_SLOT_COUNT / 8U] = {0};
    uint16_t i;
    uint8_t result;

    result = store_status == UID_STORE_OK ? UID_Store_GetPresenceList(occupied)
                                          : store_status;
    if (result != UID_STORE_OK)
    {
        uint8_t reply[3] = {UID_HOST_REQUEST_LIST, 0U, result};
        uploadToHost(UID_HOST_RESULT, reply, sizeof(reply));
        return;
    }
    for (i = 0U; i < UID_STORE_SLOT_COUNT; ++i)
    {
        if (occupied[i] != 0U)
        {
            bitmap[i / 8U] |= (uint8_t)(1U << (i % 8U));
        }
    }
    uploadToHost(UID_HOST_LIST, bitmap, sizeof(bitmap));
}

void UID_Host_Init(void)
{
    pending_state = 0U;
    store_status = UID_Store_Init();
    if (store_status == UID_STORE_OK)
    {
        send_list();
    }
}

void UID_Host_OnFrame(uint8_t cmd, const uint8_t *data, uint8_t len)
{
    uint8_t i;
    if (cmd < UID_HOST_QUERY || cmd > UID_HOST_REQUEST_LIST ||
        len > sizeof(pending_data) || pending_state != 0U)
    {
        return;
    }
    pending_state = 1U;
    pending_cmd = cmd;
    pending_len = len;
    for (i = 0U; i < len; ++i)
    {
        pending_data[i] = data[i];
    }
    pending_state = 2U;
}

void UID_Host_Task(void)
{
    uint8_t cmd;
    uint8_t len;
    uint8_t data[5];
    uint8_t reply[7];
    uint8_t result;
    uint8_t i;
    UID_Store_UID uid;

    if (pending_state != 2U)
    {
        return;
    }
    pending_state = 3U;
    cmd = pending_cmd;
    len = pending_len;
    for (i = 0U; i < len; ++i)
    {
        data[i] = pending_data[i];
    }

    if (cmd == UID_HOST_REQUEST_LIST)
    {
        if (len == 0U)
        {
            if (store_status != UID_STORE_OK)
            {
                store_status = UID_Store_Init();
            }
            pending_state = 0U;
            send_list();
        }
        else
        {
            pending_state = 0U;
        }
        return;
    }

    reply[0] = cmd;
    reply[1] = len > 0U ? data[0] : 0U;
    reply[2] = UID_STORE_ERR_ARG;

    if ((cmd == UID_HOST_QUERY || cmd == UID_HOST_DELETE) && len == 1U)
    {
        result = cmd == UID_HOST_QUERY ? UID_Store_Get(data[0], &uid)
                                       : UID_Store_Delete(data[0]);
    }
    else if ((cmd == UID_HOST_INSERT || cmd == UID_HOST_UPDATE) && len == 5U)
    {
        for (i = 0U; i < 4U; ++i)
        {
            uid.bytes[i] = data[i + 1U];
        }
        result = cmd == UID_HOST_INSERT ? UID_Store_InsertAt(data[0], &uid)
                                        : UID_Store_Update(data[0], &uid);
    }
    else
    {
        result = UID_STORE_ERR_ARG;
    }
    reply[2] = result;
    if (result == UID_STORE_ERR_FLASH)
    {
        store_status = result;
    }
    /* The host may request the next row as soon as the reply starts. */
    pending_state = 0U;
    if (cmd == UID_HOST_QUERY && result == UID_STORE_OK)
    {
        for (i = 0U; i < 4U; ++i)
        {
            reply[i + 3U] = uid.bytes[i];
        }
        uploadToHost(UID_HOST_RESULT, reply, 7U);
    }
    else
    {
        uploadToHost(UID_HOST_RESULT, reply, 3U);
    }
}
