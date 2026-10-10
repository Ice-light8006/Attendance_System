#ifndef AS608_BSP_H
#define AS608_BSP_H

#include "AS608.h"
#include <stdio.h>

#define FINGER_NO_EXIST 0x02

#define FIRST_PRESSED 0xC1         // 第一次按下手指

#define FIRST_GETIMG 0xC2          // 第一次采集指纹图像
#define FIRST_GETIMG_FAILED 0xD2   // 第一次采集指纹图像失败

#define FIRST_GENCHAR 0xC3         // 从第一次采集的指纹图像提取指纹特征
#define FIRST_GENCHAR_FAILED 0xD4  // 从第一次采集的指纹图像提取指纹特征失败

#define RELEASED 0xC4              // 松开手指

#define SECOND_PRESSED 0xC5        // 第二次按下

#define SECOND_GETIMG 0xC6         // 第二次采集指纹图像
#define SECOND_GETIMG_FAILED 0xD6  // 第二次采集指纹图像失败

#define SECOND_GENCHAR 0xC7        // 从第二次采集的指纹图像提取指纹特征
#define SECOND_GENCHAR_FAILED 0xD7 // 从第二次采集的指纹图像提取指纹特征失败

#define MATCH 0xC8                 // 比较两次指纹特征是否来自同一个手指
#define MATCH_FAILED 0xD8          // 比较两次指纹特征是否来自同一个手指失败

#define REG_MODEL 0xC9             // 把两次特征合成为一个指纹模版
#define REG_MODEL_FAILED 0xD9      // 把两次指纹特征合成为一个指纹模版失败

#define STORE_CHAR 0xCA            // 把模版保存到模块的指定ID
#define STORE_CHAR_FAILED 0xDA     // 把模版保存到模块的指定ID失败

#define ENROLL_SUCCEED 0xCB        // 指纹录入成功！

extern volatile uint8_t fingerIndexTable[128];
extern volatile uint8_t status;

enum enrollStatus
{
    WAIT_FIRST_PRESSED,
    WAIT_FIRST_GETIMG,
    WAIT_FIRST_GENCHAR,
    WAIT_RELEASED,
    WAIT_SECOND_PRESSED,
    WAIT_SECOND_GETIMG,
    WAIT_SECOND_GENCHAR,
    WAIT_MATCH,
    WAIT_REG_MODEL,
    WAIT_STORE_CHAR,
    WAIT_SECOND_RELEASED
};

enum verifyStatus
{
    WAIT_PRESSED,
    WAIT_GETIMG,
    WAIT_GENCHAR,
    WAIT_SEARCH,
    VERIFY_WAIT_RELEASED_SUCCEED,
    VERIFY_WAIT_RELEASED_FAILED
};

#define PRESSED 0xE1
#define GETIMG 0xE2
#define GETIMG_FAILED 0xE3
#define GENCHAR 0xE4
#define GENCHAR_FAILED 0xE5
#define SEARCH 0xE6
#define SEARCH_FAILED 0xE7
#define VERIFY_SUCCEED 0xE8
#define VERIFY_FAILED 0xE9

enum DeviceStatus
{
    IDLE,
    ENROLL,
    VERIFY
};

extern volatile uint8_t status;

static void printEnsureMessage(uint8_t ensure)
{
    const char* p = EnsureMessage(ensure);
    printf("%s",p);
}

static uint8_t FingerPrintExist()
{
    if(HAL_GPIO_ReadPin(AS608_TCH_GPIO_Port,AS608_TCH_Pin)==GPIO_PIN_RESET)
    {
        return FINGER_NO_EXIST;
    }
    else
    {
        return 1;
    }
}

void toggle_Status(void);

void AS608_bsp_Init(void);

uint8_t AS608_Enroll(void);

void updateFingerCount(uint16_t cnt);

uint8_t AS608_Verify(SearchResult* p);

uint8_t AS608_IsFingerExist(uint16_t index);

void updateFingerTable();

#endif