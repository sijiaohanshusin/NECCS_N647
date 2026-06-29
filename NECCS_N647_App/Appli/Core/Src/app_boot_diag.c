#include "app_boot_diag.h"
#include "stm32n6xx_hal.h"

#define APP_BOOT_DIAG_STATUS_NOT_RUN  0xFFFFFFFFUL

volatile AppBootDiag_t g_app_boot_diag =
{
  .magic = APP_BOOT_DIAG_MAGIC,
  .stage = APP_BOOT_STAGE_RESET,
  .previous_stage = APP_BOOT_STAGE_RESET,
  .error_stage = APP_BOOT_STAGE_RESET,
  .error_code = 0U,
  .tx_app_pool_status = APP_BOOT_DIAG_STATUS_NOT_RUN,
  .app_threadx_status = APP_BOOT_DIAG_STATUS_NOT_RUN,
  .touchgfx_pool_status = APP_BOOT_DIAG_STATUS_NOT_RUN,
  .touchgfx_alloc_status = APP_BOOT_DIAG_STATUS_NOT_RUN,
  .touchgfx_thread_status = APP_BOOT_DIAG_STATUS_NOT_RUN,
  .touchgfx_init_status = APP_BOOT_DIAG_STATUS_NOT_RUN,
};

void App_BootDiag_SetStage(uint32_t stage)
{
  g_app_boot_diag.previous_stage = g_app_boot_diag.stage;
  g_app_boot_diag.stage = stage;
  __DMB();
}

void App_BootDiag_SetError(uint32_t code)
{
  g_app_boot_diag.error_stage = g_app_boot_diag.stage;
  g_app_boot_diag.error_code = code;
  __DMB();
}

void App_BootDiag_RecordFault(uint32_t stage)
{
  App_BootDiag_SetStage(stage);
  g_app_boot_diag.error_stage = stage;
  g_app_boot_diag.error_code = stage;
  g_app_boot_diag.cfsr = SCB->CFSR;
  g_app_boot_diag.hfsr = SCB->HFSR;
  g_app_boot_diag.dfsr = SCB->DFSR;
  g_app_boot_diag.afsr = SCB->AFSR;
  g_app_boot_diag.bfar = SCB->BFAR;
  g_app_boot_diag.mmfar = SCB->MMFAR;
  g_app_boot_diag.icsr = SCB->ICSR;
  g_app_boot_diag.shcsr = SCB->SHCSR;
  g_app_boot_diag.ipsr = __get_IPSR();
  g_app_boot_diag.control = __get_CONTROL();
  g_app_boot_diag.msp = __get_MSP();
  g_app_boot_diag.psp = __get_PSP();
  __DMB();
}
