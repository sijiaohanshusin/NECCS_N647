/*
 * Board-level PCMD3180 + SAI capture entry for the N647 Wide32 array.
 */

#ifndef APP_PCMD_CAPTURE_H
#define APP_PCMD_CAPTURE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_audio_frame.h"
#include "PCMD3180/pcmd3180.h"
#include "tx_api.h"

#define APP_PCMD_CAPTURE_DEVICE_COUNT       PCMD3180_ARRAY_DEVICE_COUNT
#define APP_PCMD_CAPTURE_BUS_COUNT          APP_MIC_ARRAY_BUS_COUNT
#define APP_PCMD_CAPTURE_WIDE32_FRAME_LEN   APP_AUDIO_FRAME_DEFAULT_WIDE32_FRAME_LEN
#define APP_PCMD_CAPTURE_FRAME_RING_COUNT   3U

#ifndef APP_PCMD_DIAG_UI_ENABLE
#define APP_PCMD_DIAG_UI_ENABLE             0U
#endif

#define APP_PCMD_CAPTURE_RAW_FLAG_CONFIG_OK  (1UL << 0)
#define APP_PCMD_CAPTURE_RAW_FLAG_DMA_SYNC   (1UL << 1)
#define APP_PCMD_CAPTURE_RAW_FLAG_NONZERO    (1UL << 2)
#define APP_PCMD_CAPTURE_RAW_FLAG_LOW_NOISE  (1UL << 3)

typedef enum
{
  APP_PCMD_CAPTURE_OK = 0,
  APP_PCMD_CAPTURE_INVALID_ARGUMENT = -1,
  APP_PCMD_CAPTURE_UNSUPPORTED_MODE = -2,
  APP_PCMD_CAPTURE_BUSY = -3,
  APP_PCMD_CAPTURE_HAL_ERROR = -4,
  APP_PCMD_CAPTURE_PCMD_ERROR = -5,
  APP_PCMD_CAPTURE_THREADX_ERROR = -6
} AppPcmdCaptureStatus_t;

typedef struct
{
  uint8_t initialized;
  uint8_t started;
  uint8_t latest_frame_valid;
  uint8_t debug_ui_enabled;
  uint8_t raw_audio_valid;
  uint8_t raw_active_slot_count;
  AppMicArrayMode_t mode;
  uint32_t sample_rate_hz;
  uint32_t expected_bclk_hz;
  uint16_t frame_len;
  uint8_t slots_per_bus;
  uint8_t device_present_mask;
  uint8_t device_config_ok_mask;
  uint8_t device_status_ok_mask;
  uint8_t reserved0;
  int32_t init_status;
  int32_t start_status;
  int32_t dma_a_status;
  int32_t dma_b_status;
  uint32_t sai_a_half_count;
  uint32_t sai_a_full_count;
  uint32_t sai_a_error_count;
  uint32_t sai_a_last_error;
  uint32_t sai_b_half_count;
  uint32_t sai_b_full_count;
  uint32_t sai_b_error_count;
  uint32_t sai_b_last_error;
  uint32_t dma_a_error;
  uint32_t dma_b_error;
  uint32_t dma_rearm_count;
  uint32_t dma_rearm_error_count;
  uint32_t published_frames;
  uint32_t dropped_halves;
  uint32_t sync_miss_count;
  uint32_t latest_seq;
  uint32_t stale_event_flush_count;
  uint32_t raw_quality_flags;
  uint16_t published_fps_x10;
  int8_t raw_peak_dbfs;
  int8_t raw_avg_dbfs;
  uint16_t slot_level[APP_PCMD_CAPTURE_BUS_COUNT][APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS];
  uint16_t slot_level_raw[APP_PCMD_CAPTURE_BUS_COUNT][APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS];
  int16_t slot_dc_level[APP_PCMD_CAPTURE_BUS_COUNT][APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS];
  int16_t slot_last_sample[APP_PCMD_CAPTURE_BUS_COUNT][APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS];
  int8_t slot_dbfs[APP_PCMD_CAPTURE_BUS_COUNT][APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS];
  uint8_t mic_level[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT];
  int8_t mic_dbfs[APP_MIC_ARRAY_PHYSICAL_MIC_COUNT];
  int32_t device_config_status[APP_PCMD_CAPTURE_DEVICE_COUNT];
  int32_t device_status_status[APP_PCMD_CAPTURE_DEVICE_COUNT];
  PCMD3180_StatusSnapshotTypeDef device_status[APP_PCMD_CAPTURE_DEVICE_COUNT];
} AppPcmdCaptureSnapshot_t;

extern volatile uint32_t g_app_pcmd_debug_ui_enable;

AppPcmdCaptureStatus_t AppPcmdCapture_Init(AppMicArrayMode_t mode);

AppPcmdCaptureStatus_t AppPcmdCapture_Start(void);

AppPcmdCaptureStatus_t AppPcmdCapture_Poll(ULONG wait_ticks);

void AppPcmdCapture_ThreadEntry(ULONG thread_input);

void AppPcmdCapture_GetSnapshot(AppPcmdCaptureSnapshot_t *snapshot);

uint8_t AppPcmdCapture_GetLatestAudioFrame(AppAudioFrame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* APP_PCMD_CAPTURE_H */
