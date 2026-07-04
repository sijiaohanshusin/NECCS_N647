/*
 * Optional STM32N6 Neural-ART coarse heatmap backend for acoustic imaging.
 *
 * This layer is intentionally thin: it owns quantized NPU I/O buffers and the
 * generated-network bridge, while the SRP runtime keeps preprocess, FFT, GCC,
 * fine search, and visualization ownership.
 */

#ifndef APP_ACOUSTIC_NPU_H
#define APP_ACOUSTIC_NPU_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "app_acoustic_imaging.h"

#ifndef APP_ACOUSTIC_NPU_HEATMAP_ENABLE
#define APP_ACOUSTIC_NPU_HEATMAP_ENABLE 0
#endif

#define APP_ACOUSTIC_NPU_HEATMAP_INPUT_ELEMENTS \
  (APP_ACOUSTIC_IMAGING_WIDE32_BALANCED_PAIRS * 40U * 2U)
#define APP_ACOUSTIC_NPU_HEATMAP_OUTPUT_ELEMENTS APP_ACOUSTIC_IMAGING_COARSE_TOTAL
#define APP_ACOUSTIC_NPU_HEATMAP_ALIGNMENT       32U

typedef struct
{
  uint32_t quantize_cycles;
  uint32_t cache_cycles;
  uint32_t inference_cycles;
  uint32_t output_cycles;
  uint32_t total_cycles;
} AppAcousticNpuHeatmapPerf_t;

typedef struct
{
  AppAcousticImagingStatus_t init_status;
  AppAcousticImagingStatus_t last_run_status;
  uint32_t run_count;
  uint32_t error_count;
  uint8_t initialized;
  uint8_t enabled;
} AppAcousticNpuHeatmapDiag_t;

AppAcousticImagingStatus_t App_AcousticNpuHeatmap_Init(const AppAcousticImagingConfig_t *config);

AppAcousticImagingStatus_t App_AcousticNpuHeatmap_RunCoarse(const AppAcousticImagingConfig_t *config,
                                                            const float *gcc,
                                                            uint32_t pair_count,
                                                            uint32_t active_bin_count,
                                                            float *coarse_power,
                                                            uint32_t coarse_capacity,
                                                            AppAcousticNpuHeatmapPerf_t *perf_out);

void App_AcousticNpuHeatmap_GetDiag(AppAcousticNpuHeatmapDiag_t *diag_out);

#ifdef __cplusplus
}
#endif

#endif /* APP_ACOUSTIC_NPU_H */
