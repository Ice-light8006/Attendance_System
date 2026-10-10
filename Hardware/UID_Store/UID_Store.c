#include "UID_Store.h"
#include "flash.h"
#include <stddef.h>

/* The metadata uses reserved halfwords in slots 0, 1 and 2. Thus all 256
 * eight-byte slots remain available for UID records. Each metadata halfword
 * is programmed only once per erase cycle.
 *
 * ERASED -> RECEIVING -> VALID -> COPYING -> ERASED.
 * A COPYING source remains authoritative until the destination becomes VALID.
 */
#define UID_COMMIT_MARK       0xA55AU
#define UID_RECEIVING_MARK    0x7A31U
#define UID_VALID_MARK        0x59C6U
#define UID_COPYING_MARK      0x2D48U

#define UID_COMMIT_OFFSET    4U
#define UID_RECEIVING_OFFSET 6U
#define UID_VALID_OFFSET     (UID_STORE_SLOT_SIZE + 6U)
#define UID_COPYING_OFFSET   (2U * UID_STORE_SLOT_SIZE + 6U)

typedef enum
{
    PAGE_EMPTY,
    PAGE_RECEIVING,
    PAGE_VALID,
    PAGE_COPYING,
    PAGE_DAMAGED
} PageState;

static uint32_t active_page;
static uint8_t initialized;

static uint32_t slot_addr(uint32_t page, uint16_t index)
{
    return page + (uint32_t)index * UID_STORE_SLOT_SIZE;
}

static uint8_t page_is_erased(uint32_t page)
{
    uint32_t offset;
    for (offset = 0U; offset < UID_STORE_PAGE_SIZE; offset += 2U)
    {
        if (Flash_ReadHalfWord(page + offset) != 0xFFFFU)
        {
            return 0U;
        }
    }
    return 1U;
}

static PageState page_state(uint32_t page)
{
    uint16_t receiving = Flash_ReadHalfWord(page + UID_RECEIVING_OFFSET);
    uint16_t valid = Flash_ReadHalfWord(page + UID_VALID_OFFSET);
    uint16_t copying = Flash_ReadHalfWord(page + UID_COPYING_OFFSET);

    if (receiving == 0xFFFFU && valid == 0xFFFFU && copying == 0xFFFFU)
    {
        return page_is_erased(page) ? PAGE_EMPTY : PAGE_DAMAGED;
    }
    if (receiving != UID_RECEIVING_MARK)
    {
        return PAGE_DAMAGED;
    }
    if (valid == 0xFFFFU && copying == 0xFFFFU)
    {
        return PAGE_RECEIVING;
    }
    if (valid == UID_VALID_MARK && copying == 0xFFFFU)
    {
        return PAGE_VALID;
    }
    if (valid == UID_VALID_MARK && copying == UID_COPYING_MARK)
    {
        return PAGE_COPYING;
    }
    return PAGE_DAMAGED;
}

/* Page and record markers are storage protocol data, always written last. */
static uint8_t write_marker(uint32_t address, uint16_t value)
{
    uint8_t bytes[2];

    if ((address & 1U) != 0U || Flash_ReadHalfWord(address) != 0xFFFFU)
    {
        return UID_STORE_ERR_FLASH;
    }
    bytes[0] = (uint8_t)value;
    bytes[1] = (uint8_t)(value >> 8);
    if (Flash_Write(address, bytes, sizeof(bytes)) != HAL_OK ||
        Flash_ReadHalfWord(address) != value)
    {
        return UID_STORE_ERR_FLASH;
    }
    return UID_STORE_OK;
}

static uint8_t erase_and_verify_page(uint32_t page)
{
    if (Flash_Erase(page) != HAL_OK || !page_is_erased(page))
    {
        return UID_STORE_ERR_FLASH;
    }
    return UID_STORE_OK;
}

static uint8_t is_committed(uint32_t page, uint16_t index)
{
    return Flash_ReadHalfWord(slot_addr(page, index) + UID_COMMIT_OFFSET) == UID_COMMIT_MARK;
}

static void read_uid(uint32_t page, uint16_t index, UID_Store_UID *uid)
{
    Flash_Read(slot_addr(page, index), uid->bytes, 4U);
}

static uint8_t slot_is_blank(uint32_t page, uint16_t index)
{
    uint32_t address = slot_addr(page, index);
    return Flash_ReadHalfWord(address) == 0xFFFFU &&
           Flash_ReadHalfWord(address + 2U) == 0xFFFFU &&
           Flash_ReadHalfWord(address + UID_COMMIT_OFFSET) == 0xFFFFU;
}

static uint8_t write_uid(uint32_t page, uint16_t index, const UID_Store_UID *uid)
{
    uint32_t address = slot_addr(page, index);
    uint8_t bytes[4];
    uint8_t i;

    if (!slot_is_blank(page, index))
    {
        return UID_STORE_ERR_FLASH;
    }
    for (i = 0U; i < 4U; ++i)
    {
        bytes[i] = uid->bytes[i];
    }
    for (i = 0U; i < 4U; i += 2U)
    {
        uint16_t value = (uint16_t)bytes[i] | ((uint16_t)bytes[i + 1U] << 8);
        /* An all-ones halfword already has the desired erased value. */
        if (value != 0xFFFFU &&
            Flash_Write(address + i, &bytes[i], 2U) != HAL_OK)
        {
            return UID_STORE_ERR_FLASH;
        }
        if (Flash_ReadHalfWord(address + i) != value)
        {
            return UID_STORE_ERR_FLASH;
        }
    }
    /* The commit marker is written after both UID halfwords are verified. */
    return write_marker(address + UID_COMMIT_OFFSET, UID_COMMIT_MARK);
}

static uint32_t other_page(uint32_t page)
{
    return page == UID_STORE_PAGE1_ADDR ? UID_STORE_PAGE2_ADDR : UID_STORE_PAGE1_ADDR;
}

static uint8_t prepare_destination(uint32_t page)
{
    if (!page_is_erased(page))
    {
        uint8_t result = erase_and_verify_page(page);
        if (result != UID_STORE_OK)
        {
            return result;
        }
    }
    return write_marker(page + UID_RECEIVING_OFFSET, UID_RECEIVING_MARK);
}

/* omit_index == 256 means copy all records. replacement permits an atomic
 * update: the new UID is committed in the destination before page VALID. */
static uint8_t migrate(uint16_t omit_index, const UID_Store_UID *replacement)
{
    uint32_t source = active_page;
    uint32_t destination = other_page(source);
    uint16_t i;
    UID_Store_UID uid;
    uint8_t result;

    if (page_state(source) == PAGE_VALID)
    {
        result = write_marker(source + UID_COPYING_OFFSET, UID_COPYING_MARK);
        if (result != UID_STORE_OK)
        {
            initialized = 0U;
            return result;
        }
    }
    else if (page_state(source) != PAGE_COPYING)
    {
        initialized = 0U;
        return UID_STORE_ERR_STATE;
    }

    result = prepare_destination(destination);
    if (result != UID_STORE_OK)
    {
        initialized = 0U;
        return result;
    }
    for (i = 0U; i < UID_STORE_SLOT_COUNT; ++i)
    {
        if (i != omit_index && is_committed(source, i))
        {
            read_uid(source, i, &uid);
            result = write_uid(destination, i, &uid);
            if (result != UID_STORE_OK)
            {
                initialized = 0U;
                return result;
            }
        }
    }
    if (replacement != NULL)
    {
        result = write_uid(destination, omit_index, replacement);
        if (result != UID_STORE_OK)
        {
            initialized = 0U;
            return result;
        }
    }
    result = write_marker(destination + UID_VALID_OFFSET, UID_VALID_MARK);
    if (result != UID_STORE_OK)
    {
        initialized = 0U;
        return result;
    }

    active_page = destination;
    /* Do not permit another migration until the old page has been erased. */
    result = erase_and_verify_page(source);
    if (result != UID_STORE_OK)
    {
        initialized = 0U;
        return result;
    }
    return UID_STORE_OK;
}

uint8_t UID_Store_Init(void)
{
    PageState first = page_state(UID_STORE_PAGE1_ADDR);
    PageState second = page_state(UID_STORE_PAGE2_ADDR);
    uint32_t inactive;
    uint8_t result;

    initialized = 0U;
    active_page = 0U;

    if (first == PAGE_EMPTY && second == PAGE_EMPTY)
    {
        result = write_marker(UID_STORE_PAGE1_ADDR + UID_RECEIVING_OFFSET,
                              UID_RECEIVING_MARK);
        if (result != UID_STORE_OK)
        {
            return result;
        }
        result = write_marker(UID_STORE_PAGE1_ADDR + UID_VALID_OFFSET,
                              UID_VALID_MARK);
        if (result != UID_STORE_OK)
        {
            return result;
        }
        active_page = UID_STORE_PAGE1_ADDR;
    }
    else if (first == PAGE_RECEIVING && second == PAGE_EMPTY)
    {
        /* First-time initialization was interrupted; no valid page exists. */
        result = erase_and_verify_page(UID_STORE_PAGE1_ADDR);
        if (result != UID_STORE_OK)
        {
            return result;
        }
        return UID_Store_Init();
    }
    else if (first == PAGE_VALID && second != PAGE_VALID && second != PAGE_COPYING)
    {
        active_page = UID_STORE_PAGE1_ADDR;
    }
    else if (second == PAGE_VALID && first != PAGE_VALID && first != PAGE_COPYING)
    {
        active_page = UID_STORE_PAGE2_ADDR;
    }
    else if (first == PAGE_COPYING && second == PAGE_VALID)
    {
        active_page = UID_STORE_PAGE2_ADDR;
    }
    else if (second == PAGE_COPYING && first == PAGE_VALID)
    {
        active_page = UID_STORE_PAGE1_ADDR;
    }
    else if (first == PAGE_COPYING && second != PAGE_COPYING)
    {
        active_page = UID_STORE_PAGE1_ADDR;
    }
    else if (second == PAGE_COPYING && first != PAGE_COPYING)
    {
        active_page = UID_STORE_PAGE2_ADDR;
    }
    else
    {
        return UID_STORE_ERR_STATE;
    }

    inactive = other_page(active_page);
    if (!page_is_erased(inactive))
    {
        result = erase_and_verify_page(inactive);
        if (result != UID_STORE_OK)
        {
            return result;
        }
    }
    initialized = 1U;
    return UID_STORE_OK;
}

uint8_t UID_Store_GetPresenceList(uint8_t list[UID_STORE_SLOT_COUNT])
{
    uint16_t i;
    if (list == NULL)
    {
        return UID_STORE_ERR_ARG;
    }
    if (!initialized)
    {
        return UID_STORE_ERR_NOT_INIT;
    }
    for (i = 0U; i < UID_STORE_SLOT_COUNT; ++i)
    {
        list[i] = is_committed(active_page, i);
    }
    return UID_STORE_OK;
}

uint8_t UID_Store_Get(uint16_t index, UID_Store_UID *uid)
{
    if (uid == NULL)
    {
        return UID_STORE_ERR_ARG;
    }
    if (index >= UID_STORE_SLOT_COUNT)
    {
        return UID_STORE_ERR_INDEX;
    }
    if (!initialized)
    {
        return UID_STORE_ERR_NOT_INIT;
    }
    if (!is_committed(active_page, index))
    {
        return UID_STORE_ERR_EMPTY;
    }
    read_uid(active_page, index, uid);
    return UID_STORE_OK;
}

uint8_t UID_Store_Find(const UID_Store_UID *uid, uint16_t *index)
{
    uint16_t i;
    UID_Store_UID existing;

    if (uid == NULL || index == NULL)
    {
        return UID_STORE_ERR_ARG;
    }
    if (!initialized)
    {
        return UID_STORE_ERR_NOT_INIT;
    }
    for (i = 0U; i < UID_STORE_SLOT_COUNT; ++i)
    {
        if (is_committed(active_page, i))
        {
            read_uid(active_page, i, &existing);
            if (existing.bytes[0] == uid->bytes[0] &&
                existing.bytes[1] == uid->bytes[1] &&
                existing.bytes[2] == uid->bytes[2] &&
                existing.bytes[3] == uid->bytes[3])
            {
                *index = i;
                return UID_STORE_OK;
            }
        }
    }
    return UID_STORE_ERR_NOT_FOUND;
}

uint8_t UID_Store_Insert(const UID_Store_UID *uid, uint16_t *index)
{
    uint16_t i;
    uint16_t free_index = UID_STORE_SLOT_COUNT;
    UID_Store_UID existing;
    uint8_t result;

    if (uid == NULL || index == NULL)
    {
        return UID_STORE_ERR_ARG;
    }
    if (!initialized)
    {
        return UID_STORE_ERR_NOT_INIT;
    }
    for (i = 0U; i < UID_STORE_SLOT_COUNT; ++i)
    {
        if (is_committed(active_page, i))
        {
            read_uid(active_page, i, &existing);
            if (existing.bytes[0] == uid->bytes[0] &&
                existing.bytes[1] == uid->bytes[1] &&
                existing.bytes[2] == uid->bytes[2] &&
                existing.bytes[3] == uid->bytes[3])
            {
                return UID_STORE_ERR_DUPLICATE;
            }
        }
        else if (free_index == UID_STORE_SLOT_COUNT)
        {
            free_index = i;
        }
    }
    if (free_index == UID_STORE_SLOT_COUNT)
    {
        return UID_STORE_ERR_FULL;
    }
    if (!slot_is_blank(active_page, free_index))
    {
        /* An interrupted insert left dirty halfwords. Reclaim the slot. */
        result = migrate(UID_STORE_SLOT_COUNT, NULL);
        if (result != UID_STORE_OK)
        {
            return result;
        }
    }
    result = write_uid(active_page, free_index, uid);
    if (result == UID_STORE_OK)
    {
        *index = free_index;
    }
    else
    {
        initialized = 0U;
    }
    return result;
}

uint8_t UID_Store_InsertAt(uint16_t index, const UID_Store_UID *uid)
{
    uint16_t i;
    UID_Store_UID existing;
    uint8_t result;

    if (uid == NULL)
    {
        return UID_STORE_ERR_ARG;
    }
    if (index >= UID_STORE_SLOT_COUNT)
    {
        return UID_STORE_ERR_INDEX;
    }
    if (!initialized)
    {
        return UID_STORE_ERR_NOT_INIT;
    }
    if (is_committed(active_page, index))
    {
        return UID_STORE_ERR_OCCUPIED;
    }
    for (i = 0U; i < UID_STORE_SLOT_COUNT; ++i)
    {
        if (is_committed(active_page, i))
        {
            read_uid(active_page, i, &existing);
            if (existing.bytes[0] == uid->bytes[0] &&
                existing.bytes[1] == uid->bytes[1] &&
                existing.bytes[2] == uid->bytes[2] &&
                existing.bytes[3] == uid->bytes[3])
            {
                return UID_STORE_ERR_DUPLICATE;
            }
        }
    }
    if (!slot_is_blank(active_page, index))
    {
        /* Reclaim a partially written slot and commit the UID during migration. */
        return migrate(index, uid);
    }
    result = write_uid(active_page, index, uid);
    if (result != UID_STORE_OK)
    {
        initialized = 0U;
    }
    return result;
}

uint8_t UID_Store_Delete(uint16_t index)
{
    if (index >= UID_STORE_SLOT_COUNT)
    {
        return UID_STORE_ERR_INDEX;
    }
    if (!initialized)
    {
        return UID_STORE_ERR_NOT_INIT;
    }
    if (!is_committed(active_page, index))
    {
        return UID_STORE_ERR_EMPTY;
    }
    return migrate(index, NULL);
}

uint8_t UID_Store_Update(uint16_t index, const UID_Store_UID *uid)
{
    uint16_t i;
    UID_Store_UID existing;

    if (uid == NULL)
    {
        return UID_STORE_ERR_ARG;
    }
    if (index >= UID_STORE_SLOT_COUNT)
    {
        return UID_STORE_ERR_INDEX;
    }
    if (!initialized)
    {
        return UID_STORE_ERR_NOT_INIT;
    }
    if (!is_committed(active_page, index))
    {
        return UID_STORE_ERR_EMPTY;
    }
    for (i = 0U; i < UID_STORE_SLOT_COUNT; ++i)
    {
        if (is_committed(active_page, i))
        {
            read_uid(active_page, i, &existing);
            if (existing.bytes[0] == uid->bytes[0] &&
                existing.bytes[1] == uid->bytes[1] &&
                existing.bytes[2] == uid->bytes[2] &&
                existing.bytes[3] == uid->bytes[3])
            {
                return i == index ? UID_STORE_OK : UID_STORE_ERR_DUPLICATE;
            }
        }
    }
    return migrate(index, uid);
}

