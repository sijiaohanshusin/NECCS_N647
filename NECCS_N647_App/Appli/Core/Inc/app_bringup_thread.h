#ifndef APP_BRINGUP_THREAD_H
#define APP_BRINGUP_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "tx_api.h"

#define APP_BRINGUP_STATUS_MAGIC 0xB6476001UL

typedef enum
{
  APP_BRINGUP_MODULE_CLOCK = 0,
  APP_BRINGUP_MODULE_MEMORY,
  APP_BRINGUP_MODULE_DISPLAY,
  APP_BRINGUP_MODULE_TOUCH,
  APP_BRINGUP_MODULE_I2C,
  APP_BRINGUP_MODULE_CAMERA,
  APP_BRINGUP_MODULE_PCMD_RAW,
  APP_BRINGUP_MODULE_AUDIO_FRAME,
  APP_BRINGUP_MODULE_ACOUSTIC,
  APP_BRINGUP_MODULE_UI_OVERLAY,
  APP_BRINGUP_MODULE_MEDIA,
  APP_BRINGUP_MODULE_COUNT
} AppBringUpModule_t;

typedef struct
{
  uint32_t magic;
  uint32_t seq;
  uint32_t enabled_mask;
  uint32_t started_mask;
  uint32_t ready_mask;
  uint32_t failed_mask;
  uint32_t loop_count;
  uint32_t last_update_ms[APP_BRINGUP_MODULE_COUNT];
  uint32_t heartbeat[APP_BRINGUP_MODULE_COUNT];
  int32_t init_status[APP_BRINGUP_MODULE_COUNT];
  int32_t start_status[APP_BRINGUP_MODULE_COUNT];
  int32_t last_status[APP_BRINGUP_MODULE_COUNT];
} AppBringUpSnapshot_t;

extern volatile AppBringUpSnapshot_t g_app_bringup_snapshot;

void App_BringUpStatus_Reset(void);
void App_BringUpStatus_Enable(AppBringUpModule_t module);
void App_BringUpStatus_Start(AppBringUpModule_t module, int32_t status);
void App_BringUpStatus_Ready(AppBringUpModule_t module, int32_t status);
void App_BringUpStatus_Fail(AppBringUpModule_t module, int32_t status);
void App_BringUpStatus_Heartbeat(AppBringUpModule_t module, int32_t status);
void App_BringUpStatus_GetSnapshot(AppBringUpSnapshot_t *snapshot);

void App_BringUpThreadEntry(ULONG thread_input);

#ifdef __cplusplus
}
#endif

#endif /* APP_BRINGUP_THREAD_H */
