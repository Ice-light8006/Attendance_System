/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    usart.c
 * @brief   This file provides code for the configuration
 *          of the USART instances.
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
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <bsp.h>
#include <AS608_bsp.h>
#include "UID_Host.h"
#include "Host_Heartbeat.h"
#define FRAME_HEADER_1 0x5A
#define FRAME_HEADER_2 0xBB
#define FRAME_TAIL_1 0x3B
#define FRAME_TAIL_2 0x4F
#define DELETE_FINGER 0xB2
#define DELETE_FINGER_FAILED 0xA6  // 通知上位机指纹删除失败
#define DELETE_FINGER_SUCCEED 0xA7 // 通知上位机指纹删除成功
uint8_t aRxBuffer[RXBUFFERSIZE];
volatile uint8_t RX_len = 0;
uint8_t RxByte;
/* USER CODE END 0 */

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USART1 init function */

void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}
/* USART2 init function */

void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 57600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}
/* USART3 init function */

void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 115200;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspInit 0 */

  /* USER CODE END USART1_MspInit 0 */
    /* USART1 clock enable */
    __HAL_RCC_USART1_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN USART1_MspInit 1 */

  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspInit 0 */

  /* USER CODE END USART2_MspInit 0 */
    /* USART2 clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspInit 1 */

  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* USART3 interrupt Init */
    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==USART1)
  {
  /* USER CODE BEGIN USART1_MspDeInit 0 */

  /* USER CODE END USART1_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART1_CLK_DISABLE();

    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

  /* USER CODE BEGIN USART1_MspDeInit 1 */

  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2)
  {
  /* USER CODE BEGIN USART2_MspDeInit 0 */

  /* USER CODE END USART2_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART2_CLK_DISABLE();

    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);
  /* USER CODE BEGIN USART2_MspDeInit 1 */

  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3)
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();

    /**USART3 GPIO Configuration
    PB10     ------> USART3_TX
    PB11     ------> USART3_RX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10|GPIO_PIN_11);

    /* USART3 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);
  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */


#if 1
int fputc(int c,FILE* f)
{
  HAL_UART_Transmit(&huart1,(uint8_t*)&c,1,100);
  return c;
}
#endif
void uploadToHost(uint8_t cmd, uint8_t *data, uint8_t len)
{
  uint8_t frame[261];
  frame[0] = FRAME_HEADER_1;
  frame[1] = FRAME_HEADER_2;
  frame[2] = cmd;
  frame[3] = len;
  if (len != 0 && data != NULL)
  {
    memcpy(frame + 4, data, len);
  }
  frame[len + 4] = FRAME_TAIL_1;
  frame[len + 5] = FRAME_TAIL_2;
  HAL_UART_Transmit(&huart3, frame, len + 6, HAL_MAX_DELAY);
}
enum RxState
{
  WAIT_HEADER_1,
  WAIT_HEADER_2,
  WAIT_CMD,
  WAIT_LEN,
  WAIT_DATA,
  WAIT_TAIL_1,
  WAIT_TAIL_2
};

uint8_t rxData;
uint8_t rx_state = WAIT_HEADER_1;
uint8_t rx_cmd = 0;
uint8_t rx_len = 0;
uint8_t rx_data[256];
uint8_t data_index = 0;

extern volatile uint8_t enrollCurStatus;



extern volatile uint8_t status;
extern volatile uint8_t verify_statu;

volatile uint8_t host_cmd_flag = 0;   // 待处理指令标志
volatile uint16_t host_cmd_param = 0; // 待处理指令参数

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart == &huart3)
  {
    switch (rx_state)
    {
    case WAIT_HEADER_1:
    {
      if (rxData == FRAME_HEADER_1)
      {
        rx_state = WAIT_HEADER_2;
      }
      break;
    }
    case WAIT_HEADER_2:
    {
      if (rxData == FRAME_HEADER_2)
      {
        rx_state = WAIT_CMD;
      }
      else if (rxData == FRAME_HEADER_1)
      {
        rx_state = WAIT_HEADER_2;
      }
      else
      {
        rx_state = WAIT_HEADER_1;
      }
      break;
    }
    case WAIT_CMD:
    {
      rx_cmd = rxData;
      rx_state = WAIT_LEN;
      break;
    }
    case WAIT_LEN:
    {
      rx_len = rxData;
      data_index = 0;
      if (rx_len == 0)
      {
        rx_state = WAIT_TAIL_1;
      }
      else
      {
        rx_state = WAIT_DATA;
      }
      break;
    }
    case WAIT_DATA:
    {
      rx_data[data_index] = rxData;
      data_index++;
      if (data_index >= rx_len)
      {
        rx_state = WAIT_TAIL_1;
      }
      break;
    }
    case WAIT_TAIL_1:
    {
      if (rxData == FRAME_TAIL_1)
      {
        rx_state = WAIT_TAIL_2;
      }
      else
      {
        rx_state = WAIT_HEADER_1;
      }
      break;
    }
    case WAIT_TAIL_2:
    {
      rx_state = WAIT_HEADER_1;
      if (rxData == FRAME_TAIL_2)
      {
        switch (rx_cmd)
        {
        case TOGGLE_MODE:
        {
          host_cmd_flag = TOGGLE_MODE;
          break;
        }
        case DELETE_FINGER:
        {
          uint16_t tmp = rx_data[0];
          host_cmd_param = (tmp << 8) + rx_data[1]; // 取出参数
          host_cmd_flag = DELETE_FINGER;            // 记录指令
          break;
        }
        case UID_HOST_QUERY:
        case UID_HOST_INSERT:
        case UID_HOST_UPDATE:
        case UID_HOST_DELETE:
        case UID_HOST_REQUEST_LIST:
        {
          UID_Host_OnFrame(rx_cmd, rx_data, rx_len);
          break;
        }
        case HOST_PING:
        {
          Host_Heartbeat_OnFrame(rx_data, rx_len);
          break;
        }
        }
      }
    }
    break;
    }
  }
  HAL_UART_Receive_IT(&huart3, &rxData, 1);
}
/* USER CODE END 1 */

