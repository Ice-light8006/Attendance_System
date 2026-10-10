#ifndef UID_STORE_H
#define UID_STORE_H

#include <stdint.h>

/* STM32F103RCT6: 256 KB Flash, 2 KB per page. */
#define UID_STORE_PAGE_SIZE       2048U
#define UID_STORE_SLOT_SIZE       8U
#define UID_STORE_SLOT_COUNT      256U
#define UID_STORE_PAGE1_ADDR      0x0803F000UL
#define UID_STORE_PAGE2_ADDR      0x0803F800UL

/* Success is 0xFF so error codes can start at the requested 0x00. */
#define UID_STORE_OK             0xFFU
#define UID_STORE_ERR_INDEX      0x00U
#define UID_STORE_ERR_EMPTY      0x01U
#define UID_STORE_ERR_DUPLICATE  0x02U
#define UID_STORE_ERR_FULL       0x03U
#define UID_STORE_ERR_NOT_INIT   0x04U
#define UID_STORE_ERR_FLASH      0x05U
#define UID_STORE_ERR_STATE      0x06U
#define UID_STORE_ERR_ARG        0x07U
#define UID_STORE_ERR_OCCUPIED   0x08U
#define UID_STORE_ERR_NOT_FOUND  0x09U

typedef struct
{
    uint8_t bytes[4];
} UID_Store_UID;

/* Call after HAL initialization, before using any other UID_Store function.
 * All APIs are synchronous and must be called from the main context, not ISR.
 * If a Flash error occurs, call Init again before retrying a mutation.
 * A migration may already be committed when cleanup reports a Flash error;
 * after Init, query the index to learn the durable result.
 */
uint8_t UID_Store_Init(void);

/* list[i] is 1 for a committed UID at index i; otherwise it is 0. */
uint8_t UID_Store_GetPresenceList(uint8_t list[UID_STORE_SLOT_COUNT]);

/* On success, writes the four raw UID bytes to *uid. */
uint8_t UID_Store_Get(uint16_t index, UID_Store_UID *uid);

/* Finds a committed UID and writes its zero-based slot index to *index.
 * Returns UID_STORE_ERR_NOT_FOUND when the UID is not stored. */
uint8_t UID_Store_Find(const UID_Store_UID *uid, uint16_t *index);

/* Inserts at the lowest free index and writes that index to *index. */
uint8_t UID_Store_Insert(const UID_Store_UID *uid, uint16_t *index);

/* Inserts at a specified free index; an occupied index is never overwritten. */
uint8_t UID_Store_InsertAt(uint16_t index, const UID_Store_UID *uid);

/* Atomically replaces the UID at index while preserving the index. */
uint8_t UID_Store_Update(uint16_t index, const UID_Store_UID *uid);

/* Copies surviving records to the other page, retaining their indices. */
uint8_t UID_Store_Delete(uint16_t index);

#endif /* UID_STORE_H */
