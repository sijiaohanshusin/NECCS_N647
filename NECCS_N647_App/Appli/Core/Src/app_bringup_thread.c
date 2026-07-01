#include "app_bringup_thread.h"

#include "app_boot_diag.h"
#include "app_debug_config.h"
#include "app_media.h"
#include "app_power.h"
#include "main.h"
#include "./LED/led.h"
#include "./SD_NAND/sd_nand.h"

#define APP_HEARTBEAT_TICKS  ((ULONG)TX_TIMER_TICKS_PER_SECOND)

volatile uint32_t g_app_sd_card_present = 0U;
volatile uint32_t g_app_sd_card_init_status = SD_NAND_ERROR_NOT_READY;
volatile uint32_t g_app_sd_card_block_count = 0U;
volatile uint32_t g_app_sd_card_block_size = 0U;
volatile uint64_t g_app_sd_card_capacity_bytes = 0U;
volatile uint32_t g_app_sd_card_block0_read_status = SD_NAND_ERROR_NOT_READY;

static void App_BringUpMirrorMediaStatus(void)
{
#if APP_TEMP_BQ_DEBUG_MODE
  g_app_sd_card_present = 0U;
  g_app_sd_card_init_status = SD_NAND_ERROR_NOT_READY;
  g_app_sd_card_block_count = 0U;
  g_app_sd_card_block_size = 0U;
  g_app_sd_card_capacity_bytes = 0U;
  g_app_sd_card_block0_read_status = SD_NAND_ERROR_NOT_READY;
#else
  AppMediaStatus_t media_status;

  AppMedia_GetStatus(&media_status);

  g_app_sd_card_present = ((media_status.flags & APP_MEDIA_FLAG_CARD_PRESENT) != 0U) ? 1U : 0U;
  if ((media_status.flags & APP_MEDIA_FLAG_SD_READY) != 0U)
  {
    g_app_sd_card_init_status = SD_NAND_OK;
  }
  else if (media_status.last_error == APP_MEDIA_ERROR_SD_INIT)
  {
    g_app_sd_card_init_status = SD_NAND_ERROR;
  }
  else if (media_status.sd_status == SD_NAND_ERROR_NO_CARD)
  {
    g_app_sd_card_init_status = SD_NAND_ERROR_NO_CARD;
  }
  else
  {
    g_app_sd_card_init_status = SD_NAND_ERROR_NOT_READY;
  }

  g_app_sd_card_block_count = media_status.total_blocks;
  g_app_sd_card_block_size = (media_status.total_blocks != 0U) ? SD_NAND_BLOCK_SIZE : 0U;
  g_app_sd_card_capacity_bytes = ((uint64_t)media_status.total_blocks) * ((uint64_t)SD_NAND_BLOCK_SIZE);
  g_app_sd_card_block0_read_status = ((media_status.flags & APP_MEDIA_FLAG_FS_MOUNTED) != 0U) ?
                                     SD_NAND_OK :
                                     SD_NAND_ERROR_NOT_READY;
#endif
}

void App_BringUpThreadEntry(ULONG thread_input)
{
  (void)thread_input;
  App_BootDiag_SetStage(APP_BOOT_STAGE_BRINGUP_THREAD_ENTER);

  AppPower_Init();
  App_BringUpMirrorMediaStatus();

  while (1)
  {
    App_BootDiag_SetStage(APP_BOOT_STAGE_BRINGUP_THREAD_LOOP);
    g_app_boot_diag.bringup_loop_count++;
    App_BringUpMirrorMediaStatus();
    AppPower_Poll(1000U);
    LED0_TOGGLE();
    tx_thread_sleep(APP_HEARTBEAT_TICKS);
  }
}
