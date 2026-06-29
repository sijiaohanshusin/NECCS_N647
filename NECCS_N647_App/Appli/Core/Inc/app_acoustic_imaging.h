/*
 * Acoustic imaging route configuration and geometry helpers.
 *
 * The first firmware step is intentionally algorithm-framework only: profile
 * selection, microphone pair selection, coarse-grid TDOA generation, and
 * visualization frame contracts. FFT/GCC/SRP processing can build on this
 * without depending on PCMD/SAI/DMA details.
 */

#ifndef APP_ACOUSTIC_IMAGING_H
#define APP_ACOUSTIC_IMAGING_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_audio_frame.h"

#define APP_ACOUSTIC_IMAGING_SPEED_OF_SOUND_MPS        343.0f
#define APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE          9U
#define APP_ACOUSTIC_IMAGING_COARSE_TOTAL              (APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE * APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE)
#define APP_ACOUSTIC_IMAGING_FINE_TOP_K                3U
#define APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE            4U
#define APP_ACOUSTIC_IMAGING_FINE_TOTAL                (APP_ACOUSTIC_IMAGING_FINE_TOP_K * APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE * APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE)
#define APP_ACOUSTIC_IMAGING_GRID_TOTAL_MAX            (APP_ACOUSTIC_IMAGING_COARSE_TOTAL + APP_ACOUSTIC_IMAGING_FINE_TOTAL)
#define APP_ACOUSTIC_IMAGING_WIDE32_FAST_PAIRS         96U
#define APP_ACOUSTIC_IMAGING_WIDE32_BALANCED_PAIRS     160U
#define APP_ACOUSTIC_IMAGING_WIDE32_QUALITY_PAIRS      240U
#define APP_ACOUSTIC_IMAGING_CORE16_FAST_PAIRS         80U
#define APP_ACOUSTIC_IMAGING_CORE16_BALANCED_PAIRS     120U
#define APP_ACOUSTIC_IMAGING_CORE16_QUALITY_PAIRS      120U
#define APP_ACOUSTIC_IMAGING_PAIR_COUNT_MAX            APP_ACOUSTIC_IMAGING_WIDE32_QUALITY_PAIRS
#define APP_ACOUSTIC_IMAGING_VIS_CANDIDATE_MAX         3U

typedef enum
{
  APP_ACOUSTIC_IMAGING_OK = 0,
  APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT = -1,
  APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE = -2,
  APP_ACOUSTIC_IMAGING_CAPACITY_TOO_SMALL = -3,
  APP_ACOUSTIC_IMAGING_PROCESSING_FAILED = -4,
  APP_ACOUSTIC_IMAGING_SELF_TEST_FAILED = -5
} AppAcousticImagingStatus_t;

typedef enum
{
  APP_ACOUSTIC_IMAGING_PROFILE_FAST = 0,
  APP_ACOUSTIC_IMAGING_PROFILE_BALANCED = 1,
  APP_ACOUSTIC_IMAGING_PROFILE_QUALITY = 2
} AppAcousticImagingProfile_t;

typedef struct
{
  AppMicArrayMode_t mic_mode;
  AppAcousticImagingProfile_t profile;
  uint32_t sample_rate_hz;
  uint32_t channel_count;
  uint32_t frame_len;
  uint32_t nfft;
  uint16_t active_bin_start;
  uint16_t active_bin_end;
  uint16_t pair_count;
  uint8_t coarse_grid_size;
  int16_t coarse_angle_min_deg;
  int16_t coarse_angle_max_deg;
  uint8_t fine_top_k;
  uint8_t fine_grid_size;
  float fine_span_deg;
  uint8_t ui_target_fps;
  uint8_t ui_min_fps;
} AppAcousticImagingConfig_t;

typedef struct
{
  uint8_t mic_a;
  uint8_t mic_b;
  float dx_m;
  float dy_m;
  float baseline_sq_m2;
} AppAcousticImagingPair_t;

typedef struct
{
  float theta_deg;
  float phi_deg;
  float power;
  float quality;
  float contrast;
} AppAcousticImagingCandidate_t;

typedef struct
{
  float power[APP_ACOUSTIC_IMAGING_GRID_TOTAL_MAX];
  float theta_deg[APP_ACOUSTIC_IMAGING_GRID_TOTAL_MAX];
  float phi_deg[APP_ACOUSTIC_IMAGING_GRID_TOTAL_MAX];
  AppAcousticImagingCandidate_t candidate[APP_ACOUSTIC_IMAGING_VIS_CANDIDATE_MAX];
  uint32_t grid_count;
  uint32_t peak_idx;
  float peak_value;
  float quality;
  float contrast;
  AppAcousticImagingProfile_t active_profile;
  AppMicArrayMode_t mic_mode;
  uint32_t frame_seq;
  uint8_t candidate_count;
  uint8_t valid;
} AppAcousticImagingVisFrame_t;

AppAcousticImagingStatus_t App_AcousticImaging_GetDefaultConfig(AppMicArrayMode_t mode,
                                                                AppAcousticImagingProfile_t profile,
                                                                AppAcousticImagingConfig_t *config);

AppAcousticImagingStatus_t App_AcousticImaging_ValidateConfig(const AppAcousticImagingConfig_t *config);

const char *App_AcousticImaging_ProfileName(AppAcousticImagingProfile_t profile);

AppAcousticImagingStatus_t App_AcousticImaging_BuildPairSet(const AppAcousticImagingConfig_t *config,
                                                            AppAcousticImagingPair_t *pairs,
                                                            uint32_t pair_capacity,
                                                            uint32_t *pair_count);

AppAcousticImagingStatus_t App_AcousticImaging_FillCoarseGrid(const AppAcousticImagingConfig_t *config,
                                                              float *theta_deg,
                                                              float *phi_deg,
                                                              uint32_t grid_capacity,
                                                              uint32_t *grid_count);

AppAcousticImagingStatus_t App_AcousticImaging_FillCoarseTdoaLut(const AppAcousticImagingConfig_t *config,
                                                                 const AppAcousticImagingPair_t *pairs,
                                                                 uint32_t pair_count,
                                                                 float *tdoa_seconds,
                                                                 uint32_t tdoa_capacity,
                                                                 uint32_t *tdoa_count);

AppAcousticImagingStatus_t App_AcousticImaging_ValidateFrame(const AppAcousticImagingConfig_t *config,
                                                             const AppAudioFrame_t *frame);

void App_AcousticImaging_ClearVisFrame(AppAcousticImagingVisFrame_t *frame);

#ifdef __cplusplus
}
#endif

#endif /* APP_ACOUSTIC_IMAGING_H */
