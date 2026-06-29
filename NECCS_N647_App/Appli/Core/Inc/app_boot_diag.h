#ifndef APP_BOOT_DIAG_H
#define APP_BOOT_DIAG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define APP_BOOT_DIAG_MAGIC                  0xB007D1A6UL

#define APP_BOOT_STAGE_RESET                 0x00000000UL
#define APP_BOOT_STAGE_MAIN_ENTER            0x00001000UL
#define APP_BOOT_STAGE_HAL_INIT_DONE         0x00001001UL
#define APP_BOOT_STAGE_CLOCK_DONE            0x00001002UL
#define APP_BOOT_STAGE_TICK_RECOVER_DONE     0x00001003UL
#define APP_BOOT_STAGE_HYPERRAM_INIT_START   0x00001010UL
#define APP_BOOT_STAGE_HYPERRAM_INIT_DONE    0x00001011UL
#define APP_BOOT_STAGE_HYPERRAM_TEST_DONE    0x00001012UL
#define APP_BOOT_STAGE_GPIO_DONE             0x00001020UL
#define APP_BOOT_STAGE_DMA2D_DONE            0x00001021UL
#define APP_BOOT_STAGE_LTDC_DONE             0x00001022UL
#define APP_BOOT_STAGE_CRC_DONE              0x00001023UL
#define APP_BOOT_STAGE_RIF_DONE              0x00001024UL
#define APP_BOOT_STAGE_BSP_LED_DONE          0x00001030UL
#define APP_BOOT_STAGE_BSP_RGBLCD_DONE       0x00001031UL
#define APP_BOOT_STAGE_THREADX_ENTER         0x00001040UL

#define APP_BOOT_STAGE_TX_APP_DEFINE_ENTER   0x00002000UL
#define APP_BOOT_STAGE_TX_APP_POOL_DONE      0x00002001UL
#define APP_BOOT_STAGE_TX_APP_THREAD_DONE    0x00002002UL
#define APP_BOOT_STAGE_TX_TOUCHGFX_POOL_DONE 0x00002010UL
#define APP_BOOT_STAGE_TX_TOUCHGFX_INIT_DONE 0x00002011UL

#define APP_BOOT_STAGE_TOUCHGFX_THREAD_ENTER 0x00003000UL
#define APP_BOOT_STAGE_TOUCHGFX_PREOS_START  0x00003001UL
#define APP_BOOT_STAGE_TOUCHGFX_PREOS_DONE   0x00003002UL
#define APP_BOOT_STAGE_TOUCHGFX_PROCESS      0x00003003UL

#define APP_BOOT_STAGE_BRINGUP_THREAD_ENTER  0x00004000UL
#define APP_BOOT_STAGE_BRINGUP_THREAD_LOOP   0x00004001UL

#define APP_BOOT_STAGE_ERROR_HANDLER         0x0000E000UL
#define APP_BOOT_STAGE_NMI                   0x0000E001UL
#define APP_BOOT_STAGE_HARDFAULT             0x0000E002UL
#define APP_BOOT_STAGE_MEMMANAGE             0x0000E003UL
#define APP_BOOT_STAGE_BUSFAULT              0x0000E004UL
#define APP_BOOT_STAGE_USAGEFAULT            0x0000E005UL
#define APP_BOOT_STAGE_SECUREFAULT           0x0000E006UL

typedef struct
{
  uint32_t magic;
  uint32_t stage;
  uint32_t previous_stage;
  uint32_t error_stage;
  uint32_t error_code;

  uint32_t tx_app_pool_status;
  uint32_t app_threadx_status;
  uint32_t touchgfx_pool_status;
  uint32_t touchgfx_alloc_status;
  uint32_t touchgfx_thread_status;
  uint32_t touchgfx_init_status;
  uint32_t touchgfx_preos_count;

  uint32_t ltdc_up_irq_count;
  uint32_t dma2d_irq_count;
  uint32_t last_irqn;
  uint32_t bringup_loop_count;

  uint32_t cfsr;
  uint32_t hfsr;
  uint32_t dfsr;
  uint32_t afsr;
  uint32_t bfar;
  uint32_t mmfar;
  uint32_t icsr;
  uint32_t shcsr;
  uint32_t ipsr;
  uint32_t control;
  uint32_t msp;
  uint32_t psp;
} AppBootDiag_t;

extern volatile AppBootDiag_t g_app_boot_diag;

void App_BootDiag_SetStage(uint32_t stage);
void App_BootDiag_SetError(uint32_t code);
void App_BootDiag_RecordFault(uint32_t stage);

#ifdef __cplusplus
}
#endif

#endif /* APP_BOOT_DIAG_H */
