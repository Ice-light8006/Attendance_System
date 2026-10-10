#ifndef FLASH_H
#define FLASH_H

//最后一个Flash Page的首地址
#define FLASH_SAVE_ADDR 0x0803F800

#include "main.h"

HAL_StatusTypeDef Flash_Write(uint32_t addr,uint8_t *data,uint32_t len);

uint8_t Flash_ReadByte(uint32_t addr);

uint16_t Flash_ReadHalfWord(uint32_t addr);

void Flash_Read(uint32_t addr,uint8_t* data,uint32_t len);

HAL_StatusTypeDef Flash_Erase(uint32_t addr);

#endif
