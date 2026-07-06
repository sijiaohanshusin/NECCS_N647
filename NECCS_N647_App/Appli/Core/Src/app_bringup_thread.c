#include "app_bringup_thread.h"

#include "app_boot_diag.h"
#include "app_camera.h"
#include "app_media.h"
#include "app_power.h"
#include "main.h"
#include "./LED/led.h"
#include "./SD_NAND/sd_nand.h"

#define APP_HEARTBEAT_TICKS  ((ULONG)TX_TIMER_TICKS_PER_SECOND)

volatile AppBringUpSnapshot_t g_app_bringup_snapshot =
{
  .magic = APP_BRINGUP_STATUS_MAGIC,
};
volatile uint32_t g_app_bringup_control_mask = APP_BRINGUP_CONTROL_DEFAULT_MASK;

volatile uint32_t g_app_sd_card_present = 0U;
volatile uint32_t g_app_sd_card_init_status = SD_NAND_ERROR_NOT_READY;
volatile uint32_t g_app_sd_card_block_count = 0U;
volatile uint32_t g_app_sd_card_block_size = 0U;
volatile uint64_t g_app_sd_card_capacity_bytes = 0U;
volatile uint32_t g_app_sd_card_block0_read_status = SD_NAND_ERROR_NOT_READY;

static uint32_t App_BringUpModuleMask(AppBringUpModule_t module)
{
  return (module < APP_BRINGUP_MODULE_COUNT) ? (1UL << (uint32_t)module) : 0U;
}

static void App_BringUpStatusTouch(AppBringUpModule_t module, int32_t status)
{
  if (module >= APP_BRINGUP_MODULE_COUNT)
  {
    return;
  }

  g_app_bringup_snapshot.magic = APP_BRINGUP_STATUS_MAGIC;
  g_app_bringup_snapshot.seq++;
  g_app_bringup_snapshot.last_status[module] = status;
  g_app_bringup_snapshot.last_update_ms[module] = HAL_GetTick();
  __DMB();
}

void App_BringUpStatus_Reset(void)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  for (uint32_t module = 0U; module < (uint32_t)APP_BRINGUP_MODULE_COUNT; module++)
  {
    g_app_bringup_snapshot.init_status[module] = 0;
    g_app_bringup_snapshot.start_status[module] = 0;
    g_app_bringup_snapshot.last_status[module] = 0;
    g_app_bringup_snapshot.last_update_ms[module] = 0U;
    g_app_bringup_snapshot.heartbeat[module] = 0U;
  }
  g_app_bringup_snapshot.magic = APP_BRINGUP_STATUS_MAGIC;
  g_app_bringup_snapshot.seq++;
  g_app_bringup_snapshot.enabled_mask = 0U;
  g_app_bringup_snapshot.started_mask = 0U;
  g_app_bringup_snapshot.ready_mask = 0U;
  g_app_bringup_snapshot.failed_mask = 0U;
  g_app_bringup_snapshot.skipped_mask = 0U;
  g_app_bringup_snapshot.active_mask = 0U;
  g_app_bringup_snapshot.control_mask = g_app_bringup_control_mask;
  g_app_bringup_snapshot.loop_count = 0U;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

uint8_t App_BringUpControl_IsEnabled(uint32_t control_bit)
{
  g_app_bringup_snapshot.control_mask = g_app_bringup_control_mask;
  return ((g_app_bringup_control_mask & control_bit) != 0U) ? 1U : 0U;
}

void App_BringUpStatus_Enable(AppBringUpModule_t module)
{
  uint32_t mask = App_BringUpModuleMask(module);

  if (mask == 0U)
  {
    return;
  }

  g_app_bringup_snapshot.enabled_mask |= mask;
  App_BringUpStatusTouch(module, 0);
}

void App_BringUpStatus_Start(AppBringUpModule_t module, int32_t status)
{
  uint32_t mask = App_BringUpModuleMask(module);

  if (mask == 0U)
  {
    return;
  }

  g_app_bringup_snapshot.started_mask |= mask;
  g_app_bringup_snapshot.skipped_mask &= ~mask;
  g_app_bringup_snapshot.active_mask |= mask;
  g_app_bringup_snapshot.start_status[module] = status;
  if (status != 0)
  {
    g_app_bringup_snapshot.failed_mask |= mask;
  }
  App_BringUpStatusTouch(module, status);
}

void App_BringUpStatus_Ready(AppBringUpModule_t module, int32_t status)
{
  uint32_t mask = App_BringUpModuleMask(module);

  if (mask == 0U)
  {
    return;
  }

  g_app_bringup_snapshot.ready_mask |= mask;
  g_app_bringup_snapshot.failed_mask &= ~mask;
  g_app_bringup_snapshot.skipped_mask &= ~mask;
  g_app_bringup_snapshot.init_status[module] = status;
  App_BringUpStatusTouch(module, status);
}

void App_BringUpStatus_Fail(AppBringUpModule_t module, int32_t status)
{
  uint32_t mask = App_BringUpModuleMask(module);

  if (mask == 0U)
  {
    return;
  }

  g_app_bringup_snapshot.failed_mask |= mask;
  g_app_bringup_snapshot.ready_mask &= ~mask;
  g_app_bringup_snapshot.skipped_mask &= ~mask;
  g_app_bringup_snapshot.last_status[module] = status;
  App_BringUpStatusTouch(module, status);
}

void App_BringUpStatus_Skip(AppBringUpModule_t module, int32_t status)
{
  uint32_t mask = App_BringUpModuleMask(module);

  if (mask == 0U)
  {
    return;
  }

  g_app_bringup_snapshot.skipped_mask |= mask;
  g_app_bringup_snapshot.started_mask &= ~mask;
  g_app_bringup_snapshot.ready_mask &= ~mask;
  g_app_bringup_snapshot.failed_mask &= ~mask;
  g_app_bringup_snapshot.active_mask &= ~mask;
  g_app_bringup_snapshot.last_status[module] = status;
  App_BringUpStatusTouch(module, status);
}

void App_BringUpStatus_Heartbeat(AppBringUpModule_t module, int32_t status)
{
  if (module >= APP_BRINGUP_MODULE_COUNT)
  {
    return;
  }

  g_app_bringup_snapshot.heartbeat[module]++;
  App_BringUpStatusTouch(module, status);
}

void App_BringUpStatus_GetSnapshot(AppBringUpSnapshot_t *snapshot)
{
  uint32_t primask;

  if (snapshot == 0)
  {
    return;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  *snapshot = g_app_bringup_snapshot;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

static void App_BringUpMirrorMediaStatus(void)
{
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
}

void App_BringUpThreadEntry(ULONG thread_input)
{
  int32_t camera_status;
  const uint8_t power_enabled =
      App_BringUpControl_IsEnabled(APP_BRINGUP_CONTROL_POWER);
  const uint8_t camera_enabled =
      App_BringUpControl_IsEnabled(APP_BRINGUP_CONTROL_CAMERA);
  const uint8_t camera_test_pattern =
      App_BringUpControl_IsEnabled(APP_BRINGUP_CONTROL_CAMERA_TEST_PATTERN);
  const uint8_t media_enabled =
      App_BringUpControl_IsEnabled(APP_BRINGUP_CONTROL_MEDIA);

  (void)thread_input;
  App_BootDiag_SetStage(APP_BOOT_STAGE_BRINGUP_THREAD_ENTER);

  if (power_enabled != 0U)
  {
    AppPower_Init();
    App_BringUpStatus_Ready(APP_BRINGUP_MODULE_I2C, 0);
  }

  if (media_enabled != 0U)
  {
    App_BringUpStatus_Start(APP_BRINGUP_MODULE_MEDIA, 0);
    App_BringUpMirrorMediaStatus();
    App_BringUpStatus_Ready(APP_BRINGUP_MODULE_MEDIA, (int32_t)g_app_sd_card_init_status);
  }
  else
  {
    App_BringUpStatus_Skip(APP_BRINGUP_MODULE_MEDIA, 0);
  }

  if (camera_enabled != 0U)
  {
    App_BringUpStatus_Start(APP_BRINGUP_MODULE_CAMERA, 0);
    camera_status = AppCamera_Init();
    if (camera_status == APP_CAMERA_OK)
    {
      if (camera_test_pattern != 0U)
      {
        camera_status = AppCamera_SetTestPattern(1U);
      }
    }
    if (camera_status == APP_CAMERA_OK)
    {
      App_BringUpStatus_Ready(APP_BRINGUP_MODULE_CAMERA, camera_status);
      camera_status = AppCamera_StartPreview();
      if (camera_status == APP_CAMERA_OK)
      {
        App_BringUpStatus_Ready(APP_BRINGUP_MODULE_UI_OVERLAY, camera_status);
      }
      else
      {
        App_BringUpStatus_Fail(APP_BRINGUP_MODULE_UI_OVERLAY, camera_status);
      }
    }
    else
    {
      App_BringUpStatus_Fail(APP_BRINGUP_MODULE_CAMERA, camera_status);
    }
  }
  else
  {
    App_BringUpStatus_Skip(APP_BRINGUP_MODULE_CAMERA, 0);
    App_BringUpStatus_Skip(APP_BRINGUP_MODULE_UI_OVERLAY, 0);
  }

  while (1)
  {
    App_BootDiag_SetStage(APP_BOOT_STAGE_BRINGUP_THREAD_LOOP);
    g_app_boot_diag.bringup_loop_count++;
    g_app_bringup_snapshot.loop_count++;
    if (media_enabled != 0U)
    {
      App_BringUpMirrorMediaStatus();
      App_BringUpStatus_Heartbeat(APP_BRINGUP_MODULE_MEDIA, (int32_t)g_app_sd_card_init_status);
    }
    if (power_enabled != 0U)
    {
      AppPower_Poll(1000U);
    }
    if (camera_enabled != 0U)
    {
      AppCamera_Poll(1000U);
      App_BringUpStatus_Heartbeat(APP_BRINGUP_MODULE_CAMERA, (int32_t)g_app_camera_last_error);
    }
    LED0_TOGGLE();
    tx_thread_sleep(APP_HEARTBEAT_TICKS);
  }
}
