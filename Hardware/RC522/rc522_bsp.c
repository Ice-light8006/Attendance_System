#include "rc522_bsp.h"

volatile uint8_t myCardUID[4];

volatile uint8_t RC522_Statu = WAIT_FIND_CARD;

volatile uint8_t CardType[2];

void RC522_ReadCardUID(void)
{
    switch (RC522_Statu)
    {
    case WAIT_FIND_CARD:
    {
        if (PcdRequest(PICC_REQALL, CardType) == MI_OK)
        {
            RC522_Statu = FIND_CARD;
        }
        break;
    }
    case FIND_CARD:
    {
        RC522_Statu = WAIT_GET_UID;
        break;
    }
    case WAIT_GET_UID:
    {
        if (PcdAnticoll(myCardUID) == MI_OK)
        {
            RC522_Statu = GET_UID;
        }
        else
        {
            RC522_Statu = WAIT_FIND_CARD;
        }
        break;
    }
    case GET_UID:
    {
        RC522_Statu = WAIT_SLEEP;
        break;
    }
    case WAIT_SLEEP:
    {
        if (PcdHalt() == MI_OK)
        {
            RC522_Statu = SLEEPED;
        }
        else
        {
            RC522_Statu = WAIT_FIND_CARD;
        }
        break;
    }
    case SLEEPED:
    {
        notifyHostReadCard();
        RC522_Statu = WAIT_FIND_CARD;
        break;
    }
    }
}

void notifyHostReadCard(void)
{
    uploadToHost(READ_CARD_SUCCEED,myCardUID,4);
}

void notifyHostUpdateUIDList(void)
{
    
}
