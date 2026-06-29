/*
 * F32 CMSIS-DSP SRP-PHAT runtime for the N647 acoustic imaging route.
 *
 * This module consumes only AppAudioFrame_t planar F32 frames. It stays above
 * the hardware capture path, UI publishing, and ThreadX scheduling.
 */

#ifndef APP_ACOUSTIC_SRP_H
#define APP_ACOUSTIC_SRP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_acoustic_imaging.h"

#define APP_ACOUSTIC_SRP_MAX_CHANNELS          APP_MIC_ARRAY_PHYSICAL_MIC_COUNT
#define APP_ACOUSTIC_SRP_MAX_FRAME_LEN         APP_AUDIO_FRAME_DEFAULT_WIDE32_FRAME_LEN
#define APP_ACOUSTIC_SRP_MAX_NFFT              256U
#define APP_ACOUSTIC_SRP_MAX_PAIRS             APP_ACOUSTIC_IMAGING_WIDE32_BALANCED_PAIRS
#define APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS       40U
#define APP_ACOUSTIC_SRP_QUALITY_MIN           0.03f
#define APP_ACOUSTIC_SRP_ENERGY_MIN            0.02f

typedef enum
{
  APP_ACOUSTIC_BACKEND_F32_CMSIS = 0,
  APP_ACOUSTIC_BACKEND_F16_CMSIS = 1,
  APP_ACOUSTIC_BACKEND_NPU_HEATMAP = 2
} AppAcousticSrpBackend_t;

typedef struct
{
  uint32_t preprocess_cycles;
  uint32_t fft_cycles;
  uint32_t gcc_cycles;
  uint32_t coarse_cycles;
  uint32_t fine_cycles;
  uint32_t output_cycles;
  uint32_t total_cycles;
} AppAcousticSrpPerf_t;

typedef struct
{
  AppAcousticImagingConfig_t config;
  AppAcousticSrpBackend_t backend;
  AppAcousticSrpPerf_t perf;
  uint32_t pair_count;
  uint32_t active_bin_count;
  uint32_t grid_count;
  uint32_t processed_frames;
  uint8_t initialized;
} AppAcousticSrpContext_t;

AppAcousticImagingStatus_t App_AcousticSrp_Init(AppAcousticSrpContext_t *ctx,
                                                const AppAcousticImagingConfig_t *config,
                                                AppAcousticSrpBackend_t backend);

AppAcousticImagingStatus_t App_AcousticSrp_ProcessFrame(AppAcousticSrpContext_t *ctx,
                                                        const AppAudioFrame_t *frame,
                                                        AppAcousticImagingVisFrame_t *vis_frame);

void App_AcousticSrp_GetPerf(const AppAcousticSrpContext_t *ctx,
                             AppAcousticSrpPerf_t *perf_out);

AppAcousticImagingStatus_t App_AcousticSrp_RunSelfTest(AppAcousticSrpContext_t *ctx,
                                                       AppAcousticImagingVisFrame_t *vis_frame);

#ifdef __cplusplus
}
#endif

#endif /* APP_ACOUSTIC_SRP_H */
