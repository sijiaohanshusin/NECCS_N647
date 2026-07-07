/*
 * Realtime acoustic imaging service for the N647 Wide32 capture path.
 */

#ifndef APP_ACOUSTIC_SERVICE_H
#define APP_ACOUSTIC_SERVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_acoustic_srp.h"
#include "tx_api.h"

#define APP_ACOUSTIC_SERVICE_HEAT_COUNT        APP_ACOUSTIC_IMAGING_COARSE_TOTAL
#define APP_ACOUSTIC_SERVICE_CAMERA_HFOV_DEG  77.0f
#define APP_ACOUSTIC_SERVICE_CAMERA_VFOV_DEG  61.1f

typedef enum
{
  APP_ACOUSTIC_SERVICE_STATUS_OK = 0,
  APP_ACOUSTIC_SERVICE_STATUS_WAIT_FRAME = -100,
  APP_ACOUSTIC_SERVICE_STATUS_COPY_FAILED = -101
} AppAcousticServiceStatus_t;

typedef struct
{
  uint8_t initialized;
  uint8_t running;
  uint8_t valid;
  uint8_t auto_degraded;
  AppAcousticImagingRunMode_t active_mode;
  AppAcousticImagingRunMode_t requested_mode;
  AppAcousticImagingProfile_t active_profile;
  AppAcousticImagingProfile_t requested_profile;
  AppAcousticImagingBinPolicy_t active_bin_policy;
  AppAcousticImagingBinPolicy_t requested_bin_policy;
  AppAcousticSrpBackend_t backend;
  int32_t last_status;
  int32_t service_status;
  uint32_t input_seq;
  uint32_t output_seq;
  uint32_t processed_frames;
  uint32_t skipped_frames;
  uint32_t failed_frames;
  uint32_t degraded_count;
  uint32_t active_channel_mask;
  uint32_t pair_count;
  uint32_t grid_count;
  uint16_t active_bin_count;
  uint16_t srp_ms_x100;
  uint16_t fps_x10;
  int16_t theta_deg;
  int16_t phi_deg;
  uint8_t quality_pct;
  uint8_t contrast_pct;
  uint8_t peak_index;
  uint8_t heat[APP_ACOUSTIC_SERVICE_HEAT_COUNT];
  uint8_t perf_load[5];
  AppAcousticSrpPerf_t perf;
} AppAcousticServiceSnapshot_t;

extern volatile uint32_t g_app_acoustic_service_disable_auto_degrade;

AppAcousticImagingStatus_t AppAcousticService_Init(void);

void AppAcousticService_ThreadEntry(ULONG thread_input);

void AppAcousticService_GetSnapshot(AppAcousticServiceSnapshot_t *snapshot);

AppAcousticImagingStatus_t AppAcousticService_SetProfile(AppAcousticImagingProfile_t profile);

AppAcousticImagingStatus_t AppAcousticService_SetMode(AppAcousticImagingRunMode_t mode);

AppAcousticImagingStatus_t AppAcousticService_SetBinPolicy(AppAcousticImagingBinPolicy_t policy);

#ifdef __cplusplus
}
#endif

#endif /* APP_ACOUSTIC_SERVICE_H */
