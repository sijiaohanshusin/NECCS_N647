/**
  ******************************************************************************
  * @file    sd_card.c
  * @brief   Board microSD card helper APIs.
  ******************************************************************************
  */

#include "sd_card.h"

#include <stdint.h>
#include <string.h>

extern SD_HandleTypeDef hsd1;

HAL_SD_CardInfoTypeDef g_sd_card_info_struct = {0};
HAL_SD_CardCIDTypeDef g_sd_card_cid_struct = {0};

static uint8_t s_sd_card_initialized = 0U;

static uint32_t sd_card_transfer_bytes(uint32_t count)
{
  if (count > ((0x7FFFFFFFUL - 31U) / SD_CARD_BLOCK_SIZE))
  {
    return 0U;
  }

  return count * SD_CARD_BLOCK_SIZE;
}

static void sd_card_cache_span(const void *addr, uint32_t len, void **aligned_addr, int32_t *aligned_len)
{
  const uintptr_t line_size = 32U;
  const uintptr_t start = ((uintptr_t)addr) & ~(line_size - 1U);
  const uintptr_t end = (((uintptr_t)addr) + (uintptr_t)len + line_size - 1U) & ~(line_size - 1U);

  *aligned_addr = (void *)start;
  *aligned_len = (int32_t)(end - start);
}

static void sd_card_clean_cache(const void *addr, uint32_t len)
{
  void *aligned_addr = NULL;
  int32_t aligned_len = 0;

  if (len == 0U)
  {
    return;
  }

  sd_card_cache_span(addr, len, &aligned_addr, &aligned_len);
  SCB_CleanDCache_by_Addr(aligned_addr, aligned_len);
  __DSB();
}

static void sd_card_invalidate_cache(void *addr, uint32_t len)
{
  void *aligned_addr = NULL;
  int32_t aligned_len = 0;

  if (len == 0U)
  {
    return;
  }

  sd_card_cache_span(addr, len, &aligned_addr, &aligned_len);
  SCB_InvalidateDCache_by_Addr(aligned_addr, aligned_len);
  __DSB();
}

static uint8_t sd_card_wait_transfer(uint32_t timeout_ms)
{
  const uint32_t start = HAL_GetTick();

  while (HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
  {
    if ((HAL_GetTick() - start) >= timeout_ms)
    {
      return SD_CARD_ERROR_TIMEOUT;
    }
  }

  return SD_CARD_OK;
}

uint8_t sd_card_is_inserted(void)
{
#if (SD_CARD_USE_DETECT_PIN != 0U) && defined(SD_DET_Pin) && defined(SD_DET_GPIO_Port)
  return (HAL_GPIO_ReadPin(SD_DET_GPIO_Port, SD_DET_Pin) == GPIO_PIN_RESET) ? 1U : 0U;
#else
  return 1U;
#endif
}

uint8_t sd_card_init(void)
{
  if (sd_card_is_inserted() == 0U)
  {
    s_sd_card_initialized = 0U;
    return SD_CARD_ERROR_NO_CARD;
  }

  if ((s_sd_card_initialized == 0U) || (hsd1.State == HAL_SD_STATE_RESET))
  {
    if (MX_SDMMC1_SD_Init() != HAL_OK)
    {
      s_sd_card_initialized = 0U;
      return SD_CARD_ERROR;
    }

    s_sd_card_initialized = 1U;
  }

  if (HAL_SD_GetCardInfo(&hsd1, &g_sd_card_info_struct) != HAL_OK)
  {
    s_sd_card_initialized = 0U;
    return SD_CARD_ERROR;
  }

  if (HAL_SD_GetCardCID(&hsd1, &g_sd_card_cid_struct) != HAL_OK)
  {
    s_sd_card_initialized = 0U;
    return SD_CARD_ERROR;
  }

  return SD_CARD_OK;
}

uint8_t sd_card_read_disk(uint8_t *buf, uint32_t address, uint32_t count)
{
  const uint32_t bytes = sd_card_transfer_bytes(count);
  uint8_t res;

  if ((buf == NULL) || (count == 0U) || (bytes == 0U))
  {
    return SD_CARD_ERROR_PARAM;
  }

  res = sd_card_init();
  if (res != SD_CARD_OK)
  {
    return res;
  }

  sd_card_clean_cache(buf, bytes);
  sd_card_invalidate_cache(buf, bytes);

  if (HAL_SD_ReadBlocks(&hsd1, buf, address, count, SD_CARD_TIMEOUT_MS) != HAL_OK)
  {
    return SD_CARD_ERROR;
  }

  res = sd_card_wait_transfer(SD_CARD_TIMEOUT_MS);
  sd_card_invalidate_cache(buf, bytes);

  return res;
}

uint8_t sd_card_write_disk(uint8_t *buf, uint32_t address, uint32_t count)
{
  const uint32_t bytes = sd_card_transfer_bytes(count);
  uint8_t res;

  if ((buf == NULL) || (count == 0U) || (bytes == 0U))
  {
    return SD_CARD_ERROR_PARAM;
  }

  res = sd_card_init();
  if (res != SD_CARD_OK)
  {
    return res;
  }

  sd_card_clean_cache(buf, bytes);

  if (HAL_SD_WriteBlocks(&hsd1, buf, address, count, SD_CARD_TIMEOUT_MS) != HAL_OK)
  {
    return SD_CARD_ERROR;
  }

  return sd_card_wait_transfer(SD_CARD_TIMEOUT_MS);
}

uint8_t sd_card_read_block0(uint8_t *buf)
{
  return sd_card_read_disk(buf, 0U, 1U);
}

uint8_t sd_card_write_read_restore_test(uint32_t block_address)
{
  static uint8_t original[SD_CARD_BLOCK_SIZE] __attribute__((aligned(32)));
  static uint8_t pattern[SD_CARD_BLOCK_SIZE] __attribute__((aligned(32)));
  static uint8_t verify[SD_CARD_BLOCK_SIZE] __attribute__((aligned(32)));
  uint8_t res;

  res = sd_card_init();
  if (res != SD_CARD_OK)
  {
    return res;
  }

  if ((block_address == 0U) || (block_address >= sd_card_get_block_count()))
  {
    return SD_CARD_ERROR_PARAM;
  }

  res = sd_card_read_disk(original, block_address, 1U);
  if (res != SD_CARD_OK)
  {
    return res;
  }

  for (uint32_t i = 0U; i < SD_CARD_BLOCK_SIZE; i++)
  {
    pattern[i] = (uint8_t)(0xA5U ^ (uint8_t)i);
  }

  res = sd_card_write_disk(pattern, block_address, 1U);
  if (res == SD_CARD_OK)
  {
    res = sd_card_read_disk(verify, block_address, 1U);
  }

  if ((res == SD_CARD_OK) && (memcmp(pattern, verify, SD_CARD_BLOCK_SIZE) != 0))
  {
    res = SD_CARD_ERROR_VERIFY;
  }

  if (sd_card_write_disk(original, block_address, 1U) != SD_CARD_OK)
  {
    return SD_CARD_ERROR;
  }

  return res;
}

uint32_t sd_card_get_block_count(void)
{
  return g_sd_card_info_struct.LogBlockNbr;
}

uint32_t sd_card_get_block_size(void)
{
  return g_sd_card_info_struct.LogBlockSize;
}

uint64_t sd_card_get_capacity_bytes(void)
{
  return ((uint64_t)g_sd_card_info_struct.LogBlockNbr) * ((uint64_t)g_sd_card_info_struct.LogBlockSize);
}
