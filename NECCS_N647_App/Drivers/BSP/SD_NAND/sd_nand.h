/**
  ******************************************************************************
  * @file    sd_nand.h
  * @brief   Board SD NAND helper APIs.
  ******************************************************************************
  */

#ifndef __SD_NAND_H
#define __SD_NAND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

#define SD_NAND_BLOCK_SIZE             512U
#define SD_NAND_TIMEOUT_MS             5000U
#define SD_NAND_FONT_RESERVED_BLOCKS   12313U

#define SD_NAND_OK                     ((uint8_t)0U)
#define SD_NAND_ERROR                  ((uint8_t)1U)
#define SD_NAND_ERROR_TIMEOUT          ((uint8_t)2U)
#define SD_NAND_ERROR_NOT_READY        ((uint8_t)3U)
#define SD_NAND_ERROR_PARAM            ((uint8_t)4U)
#define SD_NAND_ERROR_VERIFY           ((uint8_t)5U)
#define SD_NAND_ERROR_NO_CARD          ((uint8_t)6U)

extern HAL_SD_CardInfoTypeDef g_sd_nand_info_struct;
extern HAL_SD_CardCIDTypeDef g_sd_nand_cid_struct;

uint8_t sd_nand_is_inserted(void);
uint8_t sd_nand_init(void);
uint8_t sd_nand_read_disk(uint8_t *buf, uint32_t address, uint32_t count);
uint8_t sd_nand_write_disk(uint8_t *buf, uint32_t address, uint32_t count);
uint8_t sd_nand_read_block0(uint8_t *buf);
uint8_t sd_nand_write_read_restore_test(uint32_t block_address);
uint32_t sd_nand_get_block_count(void);
uint32_t sd_nand_get_block_size(void);
uint64_t sd_nand_get_capacity_bytes(void);

#ifdef __cplusplus
}
#endif

#endif
