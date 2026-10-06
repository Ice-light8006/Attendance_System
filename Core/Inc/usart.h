/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    usart.h
  * @brief   This file contains all the function prototypes for
  *          the usart.c file
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H__
#define __USART_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

extern UART_HandleTypeDef huart1;

extern UART_HandleTypeDef huart2;

extern UART_HandleTypeDef huart3;

/* USER CODE BEGIN Private defines */
#define RXBUFFERSIZE 256
extern uint8_t RxByte;
extern uint8_t aRxBuffer[RXBUFFERSIZE];
#define FRAME_HEADER_1 0x5A
#define FRAME_HEADER_2 0XBB
#define FRAME_TAIL_1 0x3B
#define FRAME_TAIL_2 0x4F
#define FRAME_HEADER_1 0x5A
#define FRAME_HEADER_2 0xBB
#define FRAME_TAIL_1 0x3B
#define FRAME_TAIL_2 0x4F
#define DELETE_FINGER 0xB2
#define DELETE_FINGER_FAILED 0xA6  // 通知上位机指纹删除失败
#define DELETE_FINGER_SUCCEED 0xA7 // 通知上位机指纹删除成功
/* USER CODE END Private defines */

void MX_USART1_UART_Init(void);
void MX_USART2_UART_Init(void);
void MX_USART3_UART_Init(void);

/* USER CODE BEGIN Prototypes */
void uploadToHost(uint8_t cmd,uint8_t* data,uint8_t len);
/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __USART_H__ */

