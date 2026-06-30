#include "app_bringup_thread.h"

#include "app_boot_diag.h"
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

static uint8_t g_app_sd_card_probe_block[SD_NAND_BLOCK_SIZE] __attribute__((aligned(32)));

void App_BringUpThreadEntry(ULONG thread_input)
{
  (void)thread_input;
  App_BootDiag_SetStage(APP_BOOT_STAGE_BRINGUP_THREAD_ENTER);

  AppPower_Init();

  g_app_sd_card_present = sd_nand_is_inserted();
  if (g_app_sd_card_present != 0U)
  {
    g_app_sd_card_init_status = sd_nand_init();
    if (g_app_sd_card_init_status == SD_NAND_OK)
    {
      g_app_sd_card_block_count = sd_nand_get_block_count();
      g_app_sd_card_block_size = sd_nand_get_block_size();
      g_app_sd_card_capacity_bytes = sd_nand_get_capacity_bytes();
      g_app_sd_card_block0_read_status = sd_nand_read_block0(g_app_sd_card_probe_block);
    }
  }
  else
  {
    g_app_sd_card_init_status = SD_NAND_ERROR_NO_CARD;
  }

  while (1)
  {
    App_BootDiag_SetStage(APP_BOOT_STAGE_BRINGUP_THREAD_LOOP);
    g_app_boot_diag.bringup_loop_count++;
    AppPower_Poll(1000U);
    LED0_TOGGLE();
    tx_thread_sleep(APP_HEARTBEAT_TICKS);
  }
}
