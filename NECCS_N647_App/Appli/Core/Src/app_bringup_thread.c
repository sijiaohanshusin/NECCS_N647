#include "app_bringup_thread.h"

#include "app_boot_diag.h"
#include "./LED/led.h"

#define APP_HEARTBEAT_TICKS  ((ULONG)TX_TIMER_TICKS_PER_SECOND)

void App_BringUpThreadEntry(ULONG thread_input)
{
  (void)thread_input;
  App_BootDiag_SetStage(APP_BOOT_STAGE_BRINGUP_THREAD_ENTER);

  while (1)
  {
    App_BootDiag_SetStage(APP_BOOT_STAGE_BRINGUP_THREAD_LOOP);
    g_app_boot_diag.bringup_loop_count++;
    LED0_TOGGLE();
    tx_thread_sleep(APP_HEARTBEAT_TICKS);
  }
}
