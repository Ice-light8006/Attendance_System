#include "bsp.h"
#include "led.h"
#include "rc522_bsp.h"
#include "AS608_bsp.h"
#include "usart.h"
#include "buzzer.h"
#include "UID_Host.h"
#include "Host_Heartbeat.h"

#define protocol_printf uploadToHost

#define PRESSED 0xA4  // 有手指按下
#define RELEASED 0xA5 // 没有手指按下

#define FINGER_MAXSIZE 1024

void bsp_init()
{
    buzzer_init();
    MFRC522_Init();
    AS608_bsp_Init();
    UID_Host_Init();
}

extern volatile uint8_t enrollCurStatus;
extern volatile uint8_t host_cmd_flag;   // 待处理指令标志
extern volatile uint16_t host_cmd_param; // 待处理指令参数
extern volatile uint8_t verify_statu;

void bsp_loop()
{
    // 使用静态变量保存上一次的状态，用于检测变化
    static uint8_t last_status = 0xFF;
    static GPIO_PinState last_pc0 = GPIO_PIN_SET; // 假设默认是高电平

    Host_Heartbeat_Task();
    UID_Host_Task();

    if (host_cmd_flag == TOGGLE_MODE)
    {
        host_cmd_flag = 0; // 清除标志位
        if (status == IDLE)
        {
            status = ENROLL;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            verify_statu = WAIT_PRESSED;
        }
        else if (status == ENROLL)
        {
            status = IDLE;
            enrollCurStatus = WAIT_FIRST_PRESSED;
            verify_statu = WAIT_PRESSED;
        }
    }
    else if (host_cmd_flag == DELETE_FINGER)
    {
        host_cmd_flag = 0; // 清除标志位
        uint8_t ensure = GZ_DeletChar(host_cmd_param, 1);
        if (ensure != 0x00)
        {
            uploadToHost(DELETE_FINGER_FAILED, "", 0);
        }
        else
        {
            cnt--;
            PS_ReadIndexTable(0,fingerIndexTable);
            PS_ReadIndexTable(1,fingerIndexTable);
            PS_ReadIndexTable(2,fingerIndexTable);
            PS_ReadIndexTable(3,fingerIndexTable);
            updateFingerTable();
            uploadToHost(DELETE_FINGER_SUCCEED, "", 0);
        }
    }

    // 1. 仅当工作模式发生切换时，才向上位机上报一次
    if (status != last_status)
    {
        if (status == ENROLL)
        {
            uploadToHost(ENROLL_MODE, "", 0);
        }
        else if (status == IDLE)
        {
            uploadToHost(IDLE_MODE, "", 0);
        }
        last_status = status;
    }

    // 3. 执行核心业务逻辑
    if (status == ENROLL)
    {
        uint8_t ensure = AS608_Enroll();
    }
    else if (status == IDLE)
    {
        SearchResult result;
        uint8_t ensure = AS608_Verify(&result);
    }

    RC522_ReadCardUID();

    buzzer_task();

    buzzer_task_error();

    
        // 建议加一个极小的延时（例如 5ms~10ms），防止 AS608 轮询过快导致模块发热或总线死锁
    HAL_Delay(10);
}
