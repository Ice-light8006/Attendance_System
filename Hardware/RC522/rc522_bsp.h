#ifndef RC522_BSP_H
#define RC522_BSP_H

#define READ_CARD_SUCCEED 0xA8
#define UPDATE_UID_LIST 0xA9

#include "main.h"
#include "mfrc522.h"
#include "usart.h"

enum RC522_Status
{
    WAIT_FIND_CARD,
    FIND_CARD,
    WAIT_GET_UID,
    GET_UID,
    WAIT_SLEEP,
    SLEEPED
};

extern volatile uint8_t myCardUID[4];

extern volatile uint8_t RC522_Statu;

void RC522_ReadCardUID(void);

void notifyHostReadCard(void);

void notifyHostUpdateUIDList(void);

#endif
