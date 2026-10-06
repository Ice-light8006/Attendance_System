#include "AS608_bsp.h"
#include "usart.h"
#include "Buzzer.h"
extern uint32_t uart2_rx_count;
extern volatile uint8_t RX_len;
extern uint8_t aRxBuffer[RXBUFFERSIZE];
extern uint16_t cnt;
volatile uint8_t status = IDLE;
#define protocol_printf printf

volatile uint8_t enrollCurStatus = WAIT_FIRST_PRESSED;

volatile uint8_t fingerIndexTable[128];

#define UPDATA_FINGER_TABLE 0xA3

void AS608_bsp_Init(void)
{
    GZ_ValidTempleteNum(&cnt);
    PS_ReadIndexTable(0,fingerIndexTable);
    PS_ReadIndexTable(1,fingerIndexTable);
    PS_ReadIndexTable(2,fingerIndexTable);
    PS_ReadIndexTable(3,fingerIndexTable);
    updateFingerTable();
}

uint8_t AS608_Enroll(void)
{
    uint8_t ensure;
    switch (enrollCurStatus)
    {
    case WAIT_FIRST_PRESSED:
    {
        ensure = FingerPrintExist();
        if (ensure == FINGER_NO_EXIST)
        {
            enrollCurStatus = WAIT_FIRST_PRESSED;
            return ensure;
        }
        enrollCurStatus = WAIT_FIRST_GETIMG;
        uploadToHost(FIRST_PRESSED, "", 0);
        buzzer_beep(100);
        break;
    }
    case WAIT_FIRST_GETIMG:
    {
        ensure = GZ_GetImage();
        if (ensure != 0x00)
        {
            status = IDLE;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            uploadToHost(FIRST_GETIMG_FAILED, "", 0);
            return ensure;
        }
        enrollCurStatus = WAIT_FIRST_GENCHAR;
        uploadToHost(FIRST_GETIMG, "", 0);
        break;
    }
    case WAIT_FIRST_GENCHAR:
    {
        ensure = GZ_GenChar(CharBuffer1);
        // printf("After first GenChar: 0x%02X\r\n", ensure);
        if (ensure != 0x00)
        {
            status = IDLE;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            uploadToHost(FIRST_GENCHAR_FAILED, "", 0);
            return ensure;
        }
        enrollCurStatus = WAIT_RELEASED;
        uploadToHost(FIRST_GENCHAR, "", 0);
        break;
    }
    case WAIT_RELEASED:
    {
        buzzer_beep(100);
        if (HAL_GPIO_ReadPin(AS608_TCH_GPIO_Port, AS608_TCH_Pin) == GPIO_PIN_RESET)
        {
            enrollCurStatus = WAIT_SECOND_PRESSED;
            uploadToHost(RELEASED, "", 0);
        }
        break;
    }
    case WAIT_SECOND_PRESSED:
    {
        if (HAL_GPIO_ReadPin(AS608_TCH_GPIO_Port, AS608_TCH_Pin) == GPIO_PIN_SET)
        {
            buzzer_beep(100);
            enrollCurStatus = WAIT_SECOND_GETIMG;
            uploadToHost(SECOND_PRESSED, "", 0);
        }
        break;
    }
    case WAIT_SECOND_GETIMG:
    {
        ensure = GZ_GetImage();
        // printf("After second GetImage: 0x%02X\r\n", ensure);
        if (ensure != 0x00)
        {
            status = IDLE;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            uploadToHost(SECOND_GETIMG_FAILED, "", 0);
            return ensure;
        }
        uploadToHost(SECOND_GETIMG, "", 0);
        enrollCurStatus = WAIT_SECOND_GENCHAR;
        break;
    }
    case WAIT_SECOND_GENCHAR:
    {
        ensure = GZ_GenChar(CharBuffer2);
        // printf("After second GenChar: 0x%02X\r\n", ensure);
        if (ensure != 0x00)
        {
            status = IDLE;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            uploadToHost(SECOND_GENCHAR_FAILED, "", 0);
            return ensure;
        }
        uploadToHost(SECOND_GENCHAR, "", 0);
        enrollCurStatus = WAIT_MATCH;
        break;
    }
    case WAIT_MATCH:
    {
        ensure = GZ_Match();
        // printf("After Match: 0x%02X\r\n", ensure);
        if (ensure != 0x00)
        {
            status = IDLE;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            uploadToHost(MATCH_FAILED, "", 0);
            return ensure;
        }
        uploadToHost(MATCH, "", 0);
        enrollCurStatus = WAIT_REG_MODEL;
        break;
    }
    case WAIT_REG_MODEL:
    {
        ensure = GZ_RegModel();
        // printf("After RegModel: 0x%02X\r\n", ensure);
        if (ensure != 0x00)
        {
            status = IDLE;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            uploadToHost(REG_MODEL_FAILED, "", 0);
            return ensure;
        }
        uploadToHost(REG_MODEL, "", 0);
        enrollCurStatus = WAIT_STORE_CHAR;
        break;
    }
    case WAIT_STORE_CHAR:
    {
        ensure = GZ_StoreChar(CharBuffer1, cnt);
        // printf("After StoreChar: 0x%02X\r\n", ensure);
        if (ensure != 0x00)
        {
            status = IDLE;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            uploadToHost(STORE_CHAR_FAILED, "", 0);
            return ensure;
        }
        uploadToHost(STORE_CHAR, "", 0);
        enrollCurStatus = WAIT_SECOND_RELEASED;
        break;
    }
    case WAIT_SECOND_RELEASED:
    {
        if (HAL_GPIO_ReadPin(AS608_TCH_GPIO_Port, AS608_TCH_Pin) == GPIO_PIN_RESET)
        {
            uploadToHost(ENROLL_SUCCEED, "", 0);
            buzzer_success();
            cnt++;

            for(int i = 0;i<4;i++)
            {
                PS_ReadIndexTable(i,fingerIndexTable);
            }
            
            updateFingerTable();
            enrollCurStatus = WAIT_FIRST_PRESSED;
        }
        break;
    }
    }
    return 0;
}

uint8_t AS608_IsFingerExist(uint16_t index)
{
    uint8_t tmp1 = index / 8;
    uint8_t tmp2 = index % 8;
    return (fingerIndexTable[tmp1] >> tmp2) & 1;
}

volatile uint8_t verify_statu = WAIT_PRESSED;
SearchResult result;

uint8_t AS608_Verify(SearchResult *p)
{
    uint8_t ensure = 0x00;
    switch (verify_statu)
    {
    case WAIT_PRESSED:
    {
        ensure = FingerPrintExist();
        if (ensure == FINGER_NO_EXIST)
        {
            return ensure;
        }
        verify_statu = WAIT_GETIMG;
        result.mathscore = 0;
        result.pageID = 0;
        uploadToHost(PRESSED, "", 0);
        buzzer_beep(20);
        break;
    }
    case WAIT_GETIMG:
    {
        ensure = GZ_GetImage();
        if (ensure != 0x00)
        {
            verify_statu = VERIFY_WAIT_RELEASED_FAILED;
            uploadToHost(GETIMG_FAILED, "", 0);
            return ensure;
        }
        verify_statu = WAIT_GENCHAR;
        uploadToHost(GETIMG, "", 0);
        break;
    }
    case WAIT_GENCHAR:
    {
        ensure = GZ_GenChar(CharBuffer1);
        if (ensure != 0x00)
        {
            verify_statu = VERIFY_WAIT_RELEASED_FAILED;
            uploadToHost(GENCHAR_FAILED, "", 0);
            return ensure;
        }
        verify_statu = WAIT_SEARCH;
        uploadToHost(GENCHAR, "", 0);
        break;
    }
    case WAIT_SEARCH:
    {
        ensure = GZ_Search(CharBuffer1, 0, cnt, &result);
        if (ensure != 0x00)
        {
            verify_statu = VERIFY_WAIT_RELEASED_FAILED;
            uploadToHost(SEARCH_FAILED, "", 0);
            return ensure;
        }
        verify_statu = VERIFY_WAIT_RELEASED_SUCCEED;
        uploadToHost(SEARCH, "", 0);
        break;
    }
    case VERIFY_WAIT_RELEASED_SUCCEED:
    {
        if (HAL_GPIO_ReadPin(AS608_TCH_GPIO_Port, AS608_TCH_Pin) == GPIO_PIN_RESET)
        {
            verify_statu = WAIT_PRESSED;
            uint8_t data[4];
            data[0] = result.mathscore >> 8;
            data[1] = result.mathscore & (0xFF);
            data[2] = result.pageID >> 8;
            data[3] = result.pageID & (0xFF);
            uploadToHost(VERIFY_SUCCEED, data, 4);
            buzzer_beep(50);
            *p = result;
            ensure = 0x00;
        }
        break;
    }
    case VERIFY_WAIT_RELEASED_FAILED:
    {
        if (HAL_GPIO_ReadPin(AS608_TCH_GPIO_Port, AS608_TCH_Pin) == GPIO_PIN_RESET)
        {
            verify_statu = WAIT_PRESSED;
            uploadToHost(VERIFY_FAILED, "", 0);
            ensure = 0x00;
            buzzer_error();
        }
        break;
    }
    }
    return ensure;
}

void updateFingerTable()
{
    uploadToHost(UPDATA_FINGER_TABLE, fingerIndexTable, 128);
}
