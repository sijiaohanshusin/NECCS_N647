#include "app_acoustic_npu.h"

#include "stm32n6xx.h"

#include <stddef.h>
#include <string.h>

static int8_t s_npu_input[APP_ACOUSTIC_NPU_HEATMAP_INPUT_ELEMENTS]
    __attribute__((section(".EXTRAM"), aligned(APP_ACOUSTIC_NPU_HEATMAP_ALIGNMENT)));
static int8_t s_npu_output[APP_ACOUSTIC_NPU_HEATMAP_OUTPUT_ELEMENTS]
    __attribute__((section(".EXTRAM"), aligned(APP_ACOUSTIC_NPU_HEATMAP_ALIGNMENT)));

static AppAcousticNpuHeatmapDiag_t s_npu_diag =
{
  .init_status = APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE,
  .last_run_status = APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE,
  .enabled = APP_ACOUSTIC_NPU_HEATMAP_ENABLE
};

static uint32_t App_AcousticNpu_CycleNow(void)
{
#if defined(DWT)
  return DWT->CYCCNT;
#else
  return 0U;
#endif
}

static uint32_t App_AcousticNpu_CycleDelta(uint32_t start, uint32_t stop)
{
  return stop - start;
}

static int8_t App_AcousticNpu_QuantizePhat(float value)
{
  int32_t scaled;

  if (value > 1.0f)
  {
    value = 1.0f;
  }
  else if (value < -1.0f)
  {
    value = -1.0f;
  }

  scaled = (int32_t)(value * 127.0f);
  if (scaled > 127)
  {
    scaled = 127;
  }
  else if (scaled < -127)
  {
    scaled = -127;
  }

  return (int8_t)scaled;
}

static AppAcousticImagingStatus_t App_AcousticNpu_ValidateConfig(const AppAcousticImagingConfig_t *config)
{
  AppAcousticImagingStatus_t status;

  status = App_AcousticImaging_ValidateConfig(config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  if ((config->mic_mode != APP_MIC_ARRAY_MODE_WIDE32_48K) ||
      (config->profile != APP_ACOUSTIC_IMAGING_PROFILE_BALANCED) ||
      (config->pair_count != APP_ACOUSTIC_IMAGING_WIDE32_BALANCED_PAIRS) ||
      (config->active_bin_count != 40U) ||
      (config->active_bin_start != 3U) ||
      (config->active_bin_end != 42U))
  {
    return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
  }

  for (uint32_t i = 0U; i < config->active_bin_count; i++)
  {
    if (config->active_bins[i] != (uint16_t)(3U + i))
    {
      return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
    }
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

__attribute__((weak)) AppAcousticImagingStatus_t App_AcousticNpuGenerated_Init(void)
{
  return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
}

__attribute__((weak)) AppAcousticImagingStatus_t App_AcousticNpuGenerated_Run(const int8_t *input,
                                                                              uint32_t input_count,
                                                                              int8_t *output,
                                                                              uint32_t output_count,
                                                                              AppAcousticNpuHeatmapPerf_t *perf)
{
  (void)input;
  (void)input_count;
  (void)output;
  (void)output_count;
  (void)perf;
  return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
}

AppAcousticImagingStatus_t App_AcousticNpuHeatmap_Init(const AppAcousticImagingConfig_t *config)
{
  AppAcousticImagingStatus_t status;

  memset(s_npu_input, 0, sizeof(s_npu_input));
  memset(s_npu_output, 0, sizeof(s_npu_output));

  s_npu_diag.enabled = APP_ACOUSTIC_NPU_HEATMAP_ENABLE;
  s_npu_diag.initialized = 0U;
  s_npu_diag.run_count = 0U;
  s_npu_diag.error_count = 0U;

  if (APP_ACOUSTIC_NPU_HEATMAP_ENABLE == 0)
  {
    s_npu_diag.init_status = APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
    s_npu_diag.last_run_status = APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
    return s_npu_diag.init_status;
  }

  status = App_AcousticNpu_ValidateConfig(config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    s_npu_diag.init_status = status;
    s_npu_diag.last_run_status = status;
    return status;
  }

  status = App_AcousticNpuGenerated_Init();
  s_npu_diag.init_status = status;
  s_npu_diag.last_run_status = status;
  s_npu_diag.initialized = (status == APP_ACOUSTIC_IMAGING_OK) ? 1U : 0U;

  return status;
}

AppAcousticImagingStatus_t App_AcousticNpuHeatmap_RunCoarse(const AppAcousticImagingConfig_t *config,
                                                            const float *gcc,
                                                            uint32_t pair_count,
                                                            uint32_t active_bin_count,
                                                            float *coarse_power,
                                                            uint32_t coarse_capacity,
                                                            AppAcousticNpuHeatmapPerf_t *perf_out)
{
  AppAcousticNpuHeatmapPerf_t perf = { 0U, 0U, 0U, 0U, 0U };
  AppAcousticImagingStatus_t status;
  uint32_t total_start;
  uint32_t t0;
  uint32_t t1;

  if ((config == NULL) || (gcc == NULL) || (coarse_power == NULL) ||
      (coarse_capacity < APP_ACOUSTIC_IMAGING_COARSE_TOTAL))
  {
    status = APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
    goto done;
  }

  if ((APP_ACOUSTIC_NPU_HEATMAP_ENABLE == 0) || (s_npu_diag.initialized == 0U))
  {
    status = APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
    goto done;
  }

  status = App_AcousticNpu_ValidateConfig(config);
  if ((status != APP_ACOUSTIC_IMAGING_OK) ||
      (pair_count != config->pair_count) ||
      (active_bin_count != 40U))
  {
    if (status == APP_ACOUSTIC_IMAGING_OK)
    {
      status = APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
    }
    goto done;
  }

  total_start = App_AcousticNpu_CycleNow();

  t0 = total_start;
  for (uint32_t i = 0U; i < APP_ACOUSTIC_NPU_HEATMAP_INPUT_ELEMENTS; i++)
  {
    s_npu_input[i] = App_AcousticNpu_QuantizePhat(gcc[i]);
  }
  t1 = App_AcousticNpu_CycleNow();
  perf.quantize_cycles = App_AcousticNpu_CycleDelta(t0, t1);

  t0 = t1;
  status = App_AcousticNpuGenerated_Run(s_npu_input,
                                        APP_ACOUSTIC_NPU_HEATMAP_INPUT_ELEMENTS,
                                        s_npu_output,
                                        APP_ACOUSTIC_NPU_HEATMAP_OUTPUT_ELEMENTS,
                                        &perf);
  t1 = App_AcousticNpu_CycleNow();
  if (perf.inference_cycles == 0U)
  {
    perf.inference_cycles = App_AcousticNpu_CycleDelta(t0, t1);
  }

  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    goto done_with_total;
  }

  t0 = t1;
  for (uint32_t grid = 0U; grid < APP_ACOUSTIC_IMAGING_COARSE_TOTAL; grid++)
  {
    coarse_power[grid] =
        ((float)s_npu_output[grid] / 127.0f) * (float)(pair_count * active_bin_count);
  }
  t1 = App_AcousticNpu_CycleNow();
  perf.output_cycles = App_AcousticNpu_CycleDelta(t0, t1);

done_with_total:
  perf.total_cycles = App_AcousticNpu_CycleDelta(total_start, App_AcousticNpu_CycleNow());

done:
  if (perf_out != NULL)
  {
    *perf_out = perf;
  }

  s_npu_diag.last_run_status = status;
  if (status == APP_ACOUSTIC_IMAGING_OK)
  {
    s_npu_diag.run_count++;
  }
  else
  {
    s_npu_diag.error_count++;
  }

  return status;
}

void App_AcousticNpuHeatmap_GetDiag(AppAcousticNpuHeatmapDiag_t *diag_out)
{
  if (diag_out == NULL)
  {
    return;
  }

  *diag_out = s_npu_diag;
}
