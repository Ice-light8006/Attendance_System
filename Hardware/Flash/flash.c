#include "flash.h"

HAL_StatusTypeDef Flash_Write(uint32_t addr, uint8_t *data, uint32_t len)
{
    // 字节对齐检查
    if (addr % 2 != 0)
    {
        return HAL_ERROR;
    }

    HAL_FLASH_Unlock();
    uint16_t tmp;
    for (uint32_t i = 0; i < len; i += 2)
    {
        tmp = 0;
        tmp |= (uint16_t)data[i];
        if (i + 1 < len)
        {
            tmp |= (((uint16_t)data[i + 1]) << 8);
        }
        if (HAL_FLASH_Program(TYPEPROGRAM_HALFWORD, addr + i, tmp) != HAL_OK)
        {
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }

    HAL_FLASH_Lock();
    return HAL_OK;
}

uint8_t Flash_ReadByte(uint32_t addr)
{
    return *((uint8_t *)addr);
}

uint16_t Flash_ReadHalfWord(uint32_t addr)
{
    return *(uint16_t *)addr;
}

void Flash_Read(uint32_t addr, uint8_t *data, uint32_t len)
{
    for (uint32_t i = 0; i < len; i++)
    {
        data[i] = *(uint8_t *)(addr + i);
    }
}

// 擦除 addr 所在的 Flash Page（页）
HAL_StatusTypeDef Flash_Erase(uint32_t addr)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t PageError;

    HAL_FLASH_Unlock();

    erase.TypeErase = FLASH_TYPEERASE_PAGES;
    erase.PageAddress = addr;
    erase.NbPages = 1;

    HAL_StatusTypeDef status;

    status = HAL_FLASHEx_Erase(
        &erase,
        &PageError);

    HAL_FLASH_Lock();

    return status;
}
