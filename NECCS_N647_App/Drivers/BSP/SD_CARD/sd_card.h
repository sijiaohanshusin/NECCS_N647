/**
  ******************************************************************************
  * @file    sd_card.h
  * @brief   Board microSD card helper APIs.
  ******************************************************************************
  */

#ifndef __SD_CARD_H
#define __SD_CARD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define SD_CARD_BLOCK_SIZE             512U
#define SD_CARD_TIMEOUT_MS             5000U

#define SD_CARD_OK                     ((uint8_t)0U)
#define SD_CARD_ERROR                  ((uint8_t)1U)
#define SD_CARD_ERROR_TIMEOUT          ((uint8_t)2U)
#define SD_CARD_ERROR_NOT_READY        ((uint8_t)3U)
#define SD_CARD_ERROR_PARAM            ((uint8_t)4U)
#define SD_CARD_ERROR_VERIFY           ((uint8_t)5U)
#define SD_CARD_ERROR_NO_CARD          ((uint8_t)6U)

#ifndef SD_CARD_USE_DETECT_PIN
#define SD_CARD_USE_DETECT_PIN         0U
#endif

extern HAL_SD_CardInfoTypeDef g_sd_card_info_struct;
extern HAL_SD_CardCIDTypeDef g_sd_card_cid_struct;

uint8_t sd_card_is_inserted(void);
uint8_t sd_card_init(void);
uint8_t sd_card_read_disk(uint8_t *buf, uint32_t address, uint32_t count);
uint8_t sd_card_write_disk(uint8_t *buf, uint32_t address, uint32_t count);
uint8_t sd_card_read_block0(uint8_t *buf);
uint8_t sd_card_write_read_restore_test(uint32_t block_address);
uint32_t sd_card_get_block_count(void);
uint32_t sd_card_get_block_size(void);
uint64_t sd_card_get_capacity_bytes(void);

#ifdef __cplusplus
}
#endif

#endif
