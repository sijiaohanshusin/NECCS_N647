#include "app_acoustic_srp.h"

#include "app_acoustic_npu.h"
#include "app_acoustic_synthetic.h"

#include "arm_math.h"
#include "stm32n6xx.h"

#include <stddef.h>
#include <string.h>

#define APP_ACOUSTIC_SRP_FINE_TOTAL_PER_TOP \
  (APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE * APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE)
#define APP_ACOUSTIC_SRP_RUNTIME_GRID_TOTAL \
  (APP_ACOUSTIC_IMAGING_COARSE_TOTAL + APP_ACOUSTIC_IMAGING_FINE_TOTAL)
#define APP_ACOUSTIC_SRP_PHAT_EPSILON       1.0e-6f
#define APP_ACOUSTIC_SRP_NEIGHBOR_RADIUS    1U
#define APP_ACOUSTIC_SRP_CANDIDATE_MIN_DIST_DEG 15.0f

typedef struct
{
  float sin_start;
  float cos_start;
  float sin_delta;
  float cos_delta;
} AppAcousticSrpPhaseStep_t;

typedef struct
{
  AppAcousticImagingPair_t pairs[APP_ACOUSTIC_SRP_MAX_PAIRS];
  float coarse_theta[APP_ACOUSTIC_IMAGING_COARSE_TOTAL];
  float coarse_phi[APP_ACOUSTIC_IMAGING_COARSE_TOTAL];
  float coarse_tdoa[APP_ACOUSTIC_IMAGING_COARSE_TOTAL * APP_ACOUSTIC_SRP_MAX_PAIRS];
  AppAcousticSrpPhaseStep_t coarse_phase[APP_ACOUSTIC_IMAGING_COARSE_TOTAL * APP_ACOUSTIC_SRP_MAX_PAIRS];
  AppAcousticSrpPhaseStep_t fine_phase[APP_ACOUSTIC_IMAGING_FINE_TOTAL * APP_ACOUSTIC_SRP_MAX_PAIRS];
  float window[APP_ACOUSTIC_SRP_MAX_NFFT];
  float time[APP_ACOUSTIC_SRP_MAX_CHANNELS * APP_ACOUSTIC_SRP_MAX_NFFT];
  float freq[APP_ACOUSTIC_SRP_MAX_CHANNELS * APP_ACOUSTIC_SRP_MAX_NFFT];
  float gcc[APP_ACOUSTIC_SRP_MAX_PAIRS * APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS * 2U];
  float srp_weight[APP_ACOUSTIC_SRP_MAX_PAIRS * APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS];
  float srp_power[APP_ACOUSTIC_SRP_RUNTIME_GRID_TOTAL];
  float smoothed_power[APP_ACOUSTIC_SRP_RUNTIME_GRID_TOTAL];
  float fine_theta[APP_ACOUSTIC_IMAGING_FINE_TOTAL];
  float fine_phi[APP_ACOUSTIC_IMAGING_FINE_TOTAL];
  float tau[APP_ACOUSTIC_SRP_MAX_PAIRS];
  float scratch_conj[APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS * 2U];
  float scratch_cross[APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS * 2U];
  float scratch_mag[APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS];
  arm_rfft_fast_instance_f32 rfft;
} AppAcousticSrpWorkspace_t;

typedef struct
{
  float selftest_planar[APP_ACOUSTIC_SRP_MAX_CHANNELS * APP_ACOUSTIC_SRP_MAX_FRAME_LEN];
} AppAcousticSrpSlowWorkspace_t;

static AppAcousticSrpWorkspace_t s_srp_workspace __attribute__((section(".SRP_FAST"), aligned(32)));
static AppAcousticSrpSlowWorkspace_t s_srp_slow_workspace __attribute__((section(".EXTRAM"), aligned(32)));

static float App_AcousticSrp_AbsF32(float value)
{
  return (value < 0.0f) ? -value : value;
}

static uint8_t App_AcousticSrp_IsFiniteF32(float value)
{
  return ((value == value) && (value < 1.0e30f) && (value > -1.0e30f)) ? 1U : 0U;
}

static float App_AcousticSrp_Clamp01(float value)
{
  if (value > 1.0f)
  {
    return 1.0f;
  }

  if (value < 0.0f)
  {
    return 0.0f;
  }

  return value;
}

static float App_AcousticSrp_MaxF32(float a, float b)
{
  return (a > b) ? a : b;
}

static uint32_t App_AcousticSrp_ChannelBit(uint32_t channel)
{
  return (channel < 32U) ? (1UL << channel) : 0U;
}

static uint8_t App_AcousticSrp_ChannelIsActive(const AppAcousticSrpContext_t *ctx,
                                               uint32_t channel)
{
  return ((ctx->active_channel_mask & App_AcousticSrp_ChannelBit(channel)) != 0U) ? 1U : 0U;
}

static uint32_t App_AcousticSrp_CycleNow(void)
{
#if defined(DWT)
  return DWT->CYCCNT;
#else
  return 0U;
#endif
}

static uint32_t App_AcousticSrp_CycleDelta(uint32_t start, uint32_t stop)
{
  return stop - start;
}

static void App_AcousticSrp_EnableCycleCounter(void)
{
#if defined(CoreDebug) && defined(DWT)
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
#endif
}

static uint8_t App_AcousticSrp_ConfigUsesContiguousBins(const AppAcousticImagingConfig_t *config)
{
  if ((config == NULL) || (config->active_bin_count == 0U))
  {
    return 0U;
  }

  for (uint32_t i = 0U; i < config->active_bin_count; i++)
  {
    if (config->active_bins[i] != (uint16_t)(config->active_bin_start + i))
    {
      return 0U;
    }
  }

  return 1U;
}

static AppAcousticImagingStatus_t App_AcousticSrp_ValidateRuntimeConfig(const AppAcousticImagingConfig_t *config,
                                                                        AppAcousticSrpBackend_t backend)
{
  AppAcousticImagingStatus_t status;
  uint32_t active_bins;

  if (backend != APP_ACOUSTIC_BACKEND_F32_CMSIS)
  {
    if (backend != APP_ACOUSTIC_BACKEND_NPU_HEATMAP)
    {
      return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
    }
  }

  status = App_AcousticImaging_ValidateConfig(config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  active_bins = config->active_bin_count;
  if ((config->mic_mode != APP_MIC_ARRAY_MODE_WIDE32_48K) ||
      (config->channel_count > APP_ACOUSTIC_SRP_MAX_CHANNELS) ||
      (config->frame_len != APP_ACOUSTIC_SRP_MAX_FRAME_LEN) ||
      (config->nfft != APP_ACOUSTIC_SRP_MAX_NFFT) ||
      (config->pair_count > APP_ACOUSTIC_SRP_MAX_PAIRS) ||
      (active_bins > APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS) ||
      ((backend == APP_ACOUSTIC_BACKEND_NPU_HEATMAP) &&
       ((config->profile != APP_ACOUSTIC_IMAGING_PROFILE_BALANCED) ||
        (config->pair_count != APP_ACOUSTIC_IMAGING_WIDE32_BALANCED_PAIRS) ||
        (active_bins != APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS) ||
        (App_AcousticSrp_ConfigUsesContiguousBins(config) == 0U))))
  {
    return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

static void App_AcousticSrp_FillWindow(AppAcousticSrpWorkspace_t *workspace, uint32_t frame_len)
{
  for (uint32_t i = 0U; i < frame_len; i++)
  {
    float sin_v;
    float cos_v;
    float phase_deg = 360.0f * (float)i / (float)(frame_len - 1U);

    arm_sin_cos_f32(phase_deg, &sin_v, &cos_v);
    (void)sin_v;
    workspace->window[i] = 0.5f - (0.5f * cos_v);
  }
}

static float App_AcousticSrp_FrequencyPairWeight(const AppAcousticImagingPair_t *pair,
                                                 float frequency_hz,
                                                 float speed_of_sound_mps,
                                                 AppAcousticImagingAlgorithm_t algorithm)
{
  float half_wavelength;
  float alias_ratio;

  if ((pair == NULL) || (frequency_hz <= 0.0f))
  {
    return 0.0f;
  }

  half_wavelength = speed_of_sound_mps / (2.0f * frequency_hz);
  alias_ratio = pair->baseline_m / App_AcousticSrp_MaxF32(half_wavelength, 1.0e-6f);

  if (algorithm == APP_ACOUSTIC_IMAGING_ALGO_WIDE32_HF_HINT)
  {
    float aperture_weight = (pair->baseline_m < 0.012f) ? (pair->baseline_m / 0.012f) : 1.0f;
    if (alias_ratio <= 1.0f)
    {
      return App_AcousticSrp_MaxF32(0.15f, aperture_weight);
    }
    return App_AcousticSrp_MaxF32(0.05f, aperture_weight / (alias_ratio * alias_ratio));
  }

  if (alias_ratio <= 1.0f)
  {
    return 1.0f;
  }
  if (alias_ratio <= 2.0f)
  {
    return 0.65f;
  }
  return 0.35f;
}

static void App_AcousticSrp_RebuildWeights(AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  const float delta_f_hz = (float)ctx->config.sample_rate_hz / (float)ctx->config.nfft;

  ctx->weight_sum = 0.0f;
  for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
  {
    uint8_t pair_active =
        ((App_AcousticSrp_ChannelIsActive(ctx, workspace->pairs[pair].mic_a) != 0U) &&
         (App_AcousticSrp_ChannelIsActive(ctx, workspace->pairs[pair].mic_b) != 0U)) ? 1U : 0U;

    for (uint32_t bin = 0U; bin < ctx->active_bin_count; bin++)
    {
      float weight = 0.0f;

      if (pair_active != 0U)
      {
        float frequency_hz = delta_f_hz * (float)ctx->config.active_bins[bin];
        weight = App_AcousticSrp_FrequencyPairWeight(&workspace->pairs[pair],
                                                     frequency_hz,
                                                     ctx->config.speed_of_sound_mps,
                                                     ctx->config.algorithm);
      }

      workspace->srp_weight[(pair * ctx->active_bin_count) + bin] = weight;
      ctx->weight_sum += weight;
    }
  }
}

static void App_AcousticSrp_StartPhaseFromDelta(uint32_t start_bin,
                                                float sin_delta,
                                                float cos_delta,
                                                float *sin_start,
                                                float *cos_start)
{
  float sin_acc = 0.0f;
  float cos_acc = 1.0f;

  for (uint32_t bin = 0U; bin < start_bin; bin++)
  {
    float next_sin = (sin_acc * cos_delta) + (cos_acc * sin_delta);
    float next_cos = (cos_acc * cos_delta) - (sin_acc * sin_delta);

    sin_acc = next_sin;
    cos_acc = next_cos;
  }

  *sin_start = sin_acc;
  *cos_start = cos_acc;
}

static void App_AcousticSrp_BuildPhaseSteps(const AppAcousticSrpContext_t *ctx,
                                            const float *tau_seconds,
                                            AppAcousticSrpPhaseStep_t *phase)
{
  const float delta_f_hz = (float)ctx->config.sample_rate_hz / (float)ctx->config.nfft;

  for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
  {
    float d_phi_deg = 360.0f * delta_f_hz * tau_seconds[pair];

    arm_sin_cos_f32(d_phi_deg, &phase[pair].sin_delta, &phase[pair].cos_delta);
    App_AcousticSrp_StartPhaseFromDelta(ctx->config.active_bins[0],
                                        phase[pair].sin_delta,
                                        phase[pair].cos_delta,
                                        &phase[pair].sin_start,
                                        &phase[pair].cos_start);
  }
}

static void App_AcousticSrp_RebuildCoarsePhase(AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;

  for (uint32_t grid = 0U; grid < APP_ACOUSTIC_IMAGING_COARSE_TOTAL; grid++)
  {
    App_AcousticSrp_BuildPhaseSteps(ctx,
                                    &workspace->coarse_tdoa[grid * ctx->pair_count],
                                    &workspace->coarse_phase[grid * APP_ACOUSTIC_SRP_MAX_PAIRS]);
  }
}

static void App_AcousticSrp_UpdateActiveMask(AppAcousticSrpContext_t *ctx,
                                             const AppAudioFrame_t *frame)
{
  uint32_t new_mask = ctx->config.channel_mask & ~ctx->config.bad_channel_mask;

  if (frame != NULL)
  {
    new_mask &= frame->channel_valid_mask;
    new_mask &= ~frame->channel_suspect_mask;
  }

  if (new_mask != ctx->active_channel_mask)
  {
    ctx->active_channel_mask = new_mask;
    App_AcousticSrp_RebuildWeights(ctx);
  }
}

static uint8_t App_AcousticSrp_CoarseIdxIsNeighbor(uint32_t a, uint32_t b)
{
  uint32_t ai = a / APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE;
  uint32_t ap = a % APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE;
  uint32_t bi = b / APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE;
  uint32_t bp = b % APP_ACOUSTIC_IMAGING_COARSE_GRID_SIZE;
  uint32_t dti = (ai > bi) ? (ai - bi) : (bi - ai);
  uint32_t dpi = (ap > bp) ? (ap - bp) : (bp - ap);

  return ((dti <= APP_ACOUSTIC_SRP_NEIGHBOR_RADIUS) &&
          (dpi <= APP_ACOUSTIC_SRP_NEIGHBOR_RADIUS)) ? 1U : 0U;
}

static void App_AcousticSrp_FindTopCoarseNms(const float *power,
                                             uint32_t *top_idx,
                                             uint32_t top_k)
{
  uint8_t used[APP_ACOUSTIC_IMAGING_COARSE_TOTAL] = { 0U };
  uint32_t chosen = 0U;

  for (uint32_t slot = 0U; slot < top_k; slot++)
  {
    float best_value = -1.0e30f;
    uint32_t best_idx = 0U;
    uint8_t found = 0U;

    for (uint32_t idx = 0U; idx < APP_ACOUSTIC_IMAGING_COARSE_TOTAL; idx++)
    {
      uint8_t allow = 1U;

      if (used[idx] != 0U)
      {
        continue;
      }

      for (uint32_t prior = 0U; prior < chosen; prior++)
      {
        if (App_AcousticSrp_CoarseIdxIsNeighbor(idx, top_idx[prior]) != 0U)
        {
          allow = 0U;
          break;
        }
      }

      if ((allow != 0U) && (power[idx] > best_value))
      {
        best_value = power[idx];
        best_idx = idx;
        found = 1U;
      }
    }

    if (found == 0U)
    {
      for (uint32_t idx = 0U; idx < APP_ACOUSTIC_IMAGING_COARSE_TOTAL; idx++)
      {
        if ((used[idx] == 0U) && (power[idx] > best_value))
        {
          best_value = power[idx];
          best_idx = idx;
          found = 1U;
        }
      }
    }

    top_idx[slot] = best_idx;
    if (found != 0U)
    {
      used[best_idx] = 1U;
      chosen++;
    }
  }
}

static void App_AcousticSrp_GetGridAngle(const AppAcousticSrpWorkspace_t *workspace,
                                         uint32_t idx,
                                         float *theta_deg,
                                         float *phi_deg)
{
  if (idx < APP_ACOUSTIC_IMAGING_COARSE_TOTAL)
  {
    *theta_deg = workspace->coarse_theta[idx];
    *phi_deg = workspace->coarse_phi[idx];
    return;
  }

  idx -= APP_ACOUSTIC_IMAGING_COARSE_TOTAL;
  if (idx < APP_ACOUSTIC_IMAGING_FINE_TOTAL)
  {
    *theta_deg = workspace->fine_theta[idx];
    *phi_deg = workspace->fine_phi[idx];
    return;
  }

  *theta_deg = 0.0f;
  *phi_deg = 0.0f;
}

static float App_AcousticSrp_SecondMaxAll(const AppAcousticSrpWorkspace_t *workspace,
                                          uint32_t grid_count,
                                          uint32_t max_idx)
{
  float second_max = -1.0e30f;

  for (uint32_t idx = 0U; idx < grid_count; idx++)
  {
    if ((idx != max_idx) && (workspace->srp_power[idx] > second_max))
    {
      second_max = workspace->srp_power[idx];
    }
  }

  return second_max;
}

static float App_AcousticSrp_SecondMaxExcludingNeighbor(const AppAcousticSrpWorkspace_t *workspace,
                                                        uint32_t grid_count,
                                                        uint32_t max_idx)
{
  float max_theta;
  float max_phi;
  float second_max = -1.0e30f;
  uint8_t found = 0U;

  App_AcousticSrp_GetGridAngle(workspace, max_idx, &max_theta, &max_phi);

  for (uint32_t idx = 0U; idx < grid_count; idx++)
  {
    float theta;
    float phi;

    if (idx == max_idx)
    {
      continue;
    }

    App_AcousticSrp_GetGridAngle(workspace, idx, &theta, &phi);
    if ((App_AcousticSrp_AbsF32(theta - max_theta) <= 5.0f) &&
        (App_AcousticSrp_AbsF32(phi - max_phi) <= 5.0f))
    {
      continue;
    }

    if (workspace->srp_power[idx] > second_max)
    {
      second_max = workspace->srp_power[idx];
      found = 1U;
    }
  }

  if (found == 0U)
  {
    return App_AcousticSrp_SecondMaxAll(workspace, grid_count, max_idx);
  }

  return second_max;
}

static float App_AcousticSrp_AccumulatePhasePointContiguous(const AppAcousticSrpContext_t *ctx,
                                                            const AppAcousticSrpPhaseStep_t *phase)
{
  const AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  float power = 0.0f;

  for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
  {
    const float *gcc = &workspace->gcc[pair * ctx->active_bin_count * 2U];
    const float *weight = &workspace->srp_weight[pair * ctx->active_bin_count];
    float sin_d = phase[pair].sin_delta;
    float cos_d = phase[pair].cos_delta;
    float sin_phi = phase[pair].sin_start;
    float cos_phi = phase[pair].cos_start;
    float pair_sum = 0.0f;
    uint32_t bin = ctx->active_bin_count;

#define APP_ACOUSTIC_SRP_ACCUMULATE_BIN()                                      \
    do                                                                         \
    {                                                                          \
      float c_new;                                                             \
      float s_new;                                                             \
      pair_sum += (*weight++) * ((gcc[0] * cos_phi) + (gcc[1] * sin_phi));     \
      gcc += 2U;                                                               \
      c_new = (cos_phi * cos_d) - (sin_phi * sin_d);                           \
      s_new = (sin_phi * cos_d) + (cos_phi * sin_d);                           \
      cos_phi = c_new;                                                         \
      sin_phi = s_new;                                                         \
    } while (0)

    while (bin >= 4U)
    {
      APP_ACOUSTIC_SRP_ACCUMULATE_BIN();
      APP_ACOUSTIC_SRP_ACCUMULATE_BIN();
      APP_ACOUSTIC_SRP_ACCUMULATE_BIN();
      APP_ACOUSTIC_SRP_ACCUMULATE_BIN();
      bin -= 4U;
    }

    while (bin > 0U)
    {
      APP_ACOUSTIC_SRP_ACCUMULATE_BIN();
      bin--;
    }

#undef APP_ACOUSTIC_SRP_ACCUMULATE_BIN

    power += pair_sum;
  }

  return power;
}

static void App_AcousticSrp_AdvancePhase(float *sin_phi,
                                         float *cos_phi,
                                         float sin_d,
                                         float cos_d,
                                         uint16_t step_count)
{
  for (uint32_t step = 0U; step < step_count; step++)
  {
    float c_new = ((*cos_phi) * cos_d) - ((*sin_phi) * sin_d);
    float s_new = ((*sin_phi) * cos_d) + ((*cos_phi) * sin_d);

    *cos_phi = c_new;
    *sin_phi = s_new;
  }
}

static float App_AcousticSrp_AccumulatePhasePointSparse(const AppAcousticSrpContext_t *ctx,
                                                        const AppAcousticSrpPhaseStep_t *phase)
{
  const AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  float power = 0.0f;

  for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
  {
    const float *gcc = &workspace->gcc[pair * ctx->active_bin_count * 2U];
    const float *weight = &workspace->srp_weight[pair * ctx->active_bin_count];
    float sin_d = phase[pair].sin_delta;
    float cos_d = phase[pair].cos_delta;
    float sin_phi = phase[pair].sin_start;
    float cos_phi = phase[pair].cos_start;
    float pair_sum = 0.0f;

    for (uint32_t bin = 0U; bin < ctx->active_bin_count; bin++)
    {
      pair_sum += weight[bin] * ((gcc[2U * bin] * cos_phi) + (gcc[(2U * bin) + 1U] * sin_phi));

      if ((bin + 1U) < ctx->active_bin_count)
      {
        uint16_t gap = (uint16_t)(ctx->config.active_bins[bin + 1U] - ctx->config.active_bins[bin]);
        App_AcousticSrp_AdvancePhase(&sin_phi, &cos_phi, sin_d, cos_d, gap);
      }
    }

    power += pair_sum;
  }

  return power;
}

static float App_AcousticSrp_AccumulatePhasePoint(const AppAcousticSrpContext_t *ctx,
                                                  const AppAcousticSrpPhaseStep_t *phase)
{
  if (ctx->active_bins_contiguous != 0U)
  {
    return App_AcousticSrp_AccumulatePhasePointContiguous(ctx, phase);
  }

  return App_AcousticSrp_AccumulatePhasePointSparse(ctx, phase);
}

static AppAcousticImagingStatus_t App_AcousticSrp_Preprocess(AppAcousticSrpContext_t *ctx,
                                                             const AppAudioFrame_t *frame)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;

  for (uint32_t channel = 0U; channel < ctx->config.channel_count; channel++)
  {
    const float *src = App_AudioFrame_GetChannelF32(frame, channel);
    float *time = &workspace->time[channel * ctx->config.nfft];
    float *freq = &workspace->freq[channel * ctx->config.nfft];
    float mean = 0.0f;

    if (src == NULL)
    {
      return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
    }

    if (App_AcousticSrp_ChannelIsActive(ctx, channel) == 0U)
    {
      memset(time, 0, ctx->config.nfft * sizeof(float));
      memset(freq, 0, ctx->config.nfft * sizeof(float));
      continue;
    }

    memcpy(time, src, ctx->config.frame_len * sizeof(float));

    arm_mean_f32(time, ctx->config.frame_len, &mean);
    arm_offset_f32(time, -mean, time, ctx->config.frame_len);
    arm_mult_f32(time, workspace->window, time, ctx->config.frame_len);
    (void)freq;
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

static void App_AcousticSrp_RunFft(const AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;

  for (uint32_t channel = 0U; channel < ctx->config.channel_count; channel++)
  {
    float *time = &workspace->time[channel * ctx->config.nfft];
    float *freq = &workspace->freq[channel * ctx->config.nfft];

    if (App_AcousticSrp_ChannelIsActive(ctx, channel) != 0U)
    {
      arm_rfft_fast_f32(&workspace->rfft, time, freq, 0U);
    }
    else
    {
      memset(freq, 0, ctx->config.nfft * sizeof(float));
    }
  }
}

uint32_t App_AcousticSrp_GetRefSpectrum(const AppAcousticSrpContext_t *ctx,
                                        float *mags,
                                        uint32_t mag_count)
{
  const AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  const float *freq = NULL;
  uint32_t half;

  if ((ctx == NULL) || (mags == NULL) || (mag_count == 0U) || (ctx->initialized == 0U))
  {
    return 0U;
  }

  /* First active channel's spectrum from the frame just processed (the
   * service is single-threaded, so the workspace still holds it). */
  for (uint32_t channel = 0U; channel < ctx->config.channel_count; channel++)
  {
    if (App_AcousticSrp_ChannelIsActive(ctx, channel) != 0U)
    {
      freq = &workspace->freq[channel * ctx->config.nfft];
      break;
    }
  }
  if (freq == NULL)
  {
    return 0U;
  }

  half = ctx->config.nfft / 2U;
  if (mag_count > half)
  {
    mag_count = half;
  }

  /* CMSIS rfft packed layout: [X0.re, XN/2.re, X1.re, X1.im, ...].
   * |re| + |im| is a fine magnitude proxy for a display spectrum. */
  mags[0] = App_AcousticSrp_AbsF32(freq[0]);
  for (uint32_t bin = 1U; bin < mag_count; bin++)
  {
    mags[bin] = App_AcousticSrp_AbsF32(freq[bin * 2U]) +
                App_AcousticSrp_AbsF32(freq[(bin * 2U) + 1U]);
  }

  return mag_count;
}

static uint8_t App_AcousticSrp_PairHasWeight(const AppAcousticSrpContext_t *ctx,
                                             uint32_t pair_index)
{
  const AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;

  for (uint32_t bin = 0U; bin < ctx->active_bin_count; bin++)
  {
    if (workspace->srp_weight[(pair_index * ctx->active_bin_count) + bin] > 0.0f)
    {
      return 1U;
    }
  }

  return 0U;
}

static void App_AcousticSrp_ComputeGccPhat(const AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;

  for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
  {
    uint32_t mic_a = workspace->pairs[pair].mic_a;
    uint32_t mic_b = workspace->pairs[pair].mic_b;
    float *out = &workspace->gcc[pair * ctx->active_bin_count * 2U];

    if (App_AcousticSrp_PairHasWeight(ctx, pair) == 0U)
    {
      memset(out, 0, ctx->active_bin_count * 2U * sizeof(float));
      continue;
    }

    if (ctx->active_bins_contiguous != 0U)
    {
      const float *xi = &workspace->freq[(mic_a * ctx->config.nfft) + (2U * ctx->config.active_bin_start)];
      const float *xj = &workspace->freq[(mic_b * ctx->config.nfft) + (2U * ctx->config.active_bin_start)];

      arm_cmplx_conj_f32(xj, workspace->scratch_conj, ctx->active_bin_count);
      arm_cmplx_mult_cmplx_f32(xi, workspace->scratch_conj, workspace->scratch_cross, ctx->active_bin_count);
      arm_cmplx_mag_f32(workspace->scratch_cross, workspace->scratch_mag, ctx->active_bin_count);

      for (uint32_t bin = 0U; bin < ctx->active_bin_count; bin++)
      {
        float inv_mag = 1.0f / (workspace->scratch_mag[bin] + APP_ACOUSTIC_SRP_PHAT_EPSILON);

        out[2U * bin] = workspace->scratch_cross[2U * bin] * inv_mag;
        out[(2U * bin) + 1U] = workspace->scratch_cross[(2U * bin) + 1U] * inv_mag;
      }
    }
    else
    {
      const float *freq_a = &workspace->freq[mic_a * ctx->config.nfft];
      const float *freq_b = &workspace->freq[mic_b * ctx->config.nfft];

      for (uint32_t bin = 0U; bin < ctx->active_bin_count; bin++)
      {
        uint32_t fft_index = 2U * (uint32_t)ctx->config.active_bins[bin];
        float ar = freq_a[fft_index];
        float ai = freq_a[fft_index + 1U];
        float br = freq_b[fft_index];
        float bi = freq_b[fft_index + 1U];
        float cross_re = (ar * br) + (ai * bi);
        float cross_im = (ai * br) - (ar * bi);
        float mag;
        float inv_mag;

        arm_sqrt_f32((cross_re * cross_re) + (cross_im * cross_im), &mag);
        inv_mag = 1.0f / (mag + APP_ACOUSTIC_SRP_PHAT_EPSILON);
        out[2U * bin] = cross_re * inv_mag;
        out[(2U * bin) + 1U] = cross_im * inv_mag;
      }
    }
  }
}

static void App_AcousticSrp_RunCoarseSearch(const AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;

  for (uint32_t grid = 0U; grid < APP_ACOUSTIC_IMAGING_COARSE_TOTAL; grid++)
  {
    workspace->srp_power[grid] =
        App_AcousticSrp_AccumulatePhasePoint(ctx, &workspace->coarse_phase[grid * APP_ACOUSTIC_SRP_MAX_PAIRS]);
  }
}

static void App_AcousticSrp_RunFineSearch(const AppAcousticSrpContext_t *ctx,
                                          const uint32_t *top_idx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  float fine_step = (2.0f * ctx->config.fine_span_deg) / (float)ctx->config.fine_grid_size;
  float inv_c = 1.0f / ctx->config.speed_of_sound_mps;

  for (uint32_t top = 0U; top < ctx->config.fine_top_k; top++)
  {
    uint32_t coarse_idx = top_idx[top];
    float center_theta = workspace->coarse_theta[coarse_idx];
    float center_phi = workspace->coarse_phi[coarse_idx];

    for (uint32_t fi = 0U; fi < ctx->config.fine_grid_size; fi++)
    {
      float theta = center_theta + (-ctx->config.fine_span_deg + (((float)fi + 0.5f) * fine_step));
      float sin_theta;
      float cos_theta_unused;

      arm_sin_cos_f32(theta, &sin_theta, &cos_theta_unused);
      (void)cos_theta_unused;

      for (uint32_t fj = 0U; fj < ctx->config.fine_grid_size; fj++)
      {
        uint32_t local_idx = (fi * ctx->config.fine_grid_size) + fj;
        uint32_t fine_idx = (top * APP_ACOUSTIC_SRP_FINE_TOTAL_PER_TOP) + local_idx;
        float phi = center_phi + (-ctx->config.fine_span_deg + (((float)fj + 0.5f) * fine_step));
        float sin_phi;
        float cos_phi;
        float sin_theta_cos_phi;

        arm_sin_cos_f32(phi, &sin_phi, &cos_phi);
        sin_theta_cos_phi = sin_theta * cos_phi;

        workspace->fine_theta[fine_idx] = theta;
        workspace->fine_phi[fine_idx] = phi;

        for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
        {
          workspace->tau[pair] =
              ((workspace->pairs[pair].dx_m * sin_theta_cos_phi) +
               (workspace->pairs[pair].dy_m * sin_phi)) * inv_c;
        }

        App_AcousticSrp_BuildPhaseSteps(ctx,
                                        workspace->tau,
                                        &workspace->fine_phase[fine_idx * APP_ACOUSTIC_SRP_MAX_PAIRS]);
        workspace->srp_power[APP_ACOUSTIC_IMAGING_COARSE_TOTAL + fine_idx] =
            App_AcousticSrp_AccumulatePhasePoint(ctx,
                                                 &workspace->fine_phase[fine_idx * APP_ACOUSTIC_SRP_MAX_PAIRS]);
      }
    }
  }
}

static void App_AcousticSrp_SmoothPower(AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  float alpha = ctx->config.smoothing_alpha;

  if (alpha <= 0.0f)
  {
    ctx->smoothing_valid = 0U;
    return;
  }

  if (ctx->smoothing_valid == 0U)
  {
    for (uint32_t idx = 0U; idx < ctx->grid_count; idx++)
    {
      workspace->smoothed_power[idx] = workspace->srp_power[idx];
    }
    ctx->smoothing_valid = 1U;
    return;
  }

  for (uint32_t idx = 0U; idx < ctx->grid_count; idx++)
  {
    workspace->smoothed_power[idx] =
        (alpha * workspace->smoothed_power[idx]) + ((1.0f - alpha) * workspace->srp_power[idx]);
    workspace->srp_power[idx] = workspace->smoothed_power[idx];
  }
}

static uint8_t App_AcousticSrp_IsCandidateTooClose(const AppAcousticImagingVisFrame_t *vis_frame,
                                                   uint32_t candidate_count,
                                                   float theta,
                                                   float phi)
{
  for (uint32_t idx = 0U; idx < candidate_count; idx++)
  {
    float dt = theta - vis_frame->candidate[idx].theta_deg;
    float dp = phi - vis_frame->candidate[idx].phi_deg;

    if (((dt * dt) + (dp * dp)) <
        (APP_ACOUSTIC_SRP_CANDIDATE_MIN_DIST_DEG * APP_ACOUSTIC_SRP_CANDIDATE_MIN_DIST_DEG))
    {
      return 1U;
    }
  }

  return 0U;
}

static void App_AcousticSrp_FillCandidates(const AppAcousticSrpContext_t *ctx,
                                           AppAcousticImagingVisFrame_t *vis_frame)
{
  const AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  uint8_t used[APP_ACOUSTIC_SRP_RUNTIME_GRID_TOTAL] = { 0U };
  uint32_t count = 0U;

  for (uint32_t slot = 0U; slot < APP_ACOUSTIC_IMAGING_VIS_CANDIDATE_MAX; slot++)
  {
    float best_value = -1.0e30f;
    float best_theta = 0.0f;
    float best_phi = 0.0f;
    uint32_t best_idx = 0U;
    uint8_t found = 0U;

    for (uint32_t idx = 0U; idx < ctx->grid_count; idx++)
    {
      float theta;
      float phi;

      if ((used[idx] != 0U) || (workspace->srp_power[idx] <= best_value))
      {
        continue;
      }

      App_AcousticSrp_GetGridAngle(workspace, idx, &theta, &phi);
      if (App_AcousticSrp_IsCandidateTooClose(vis_frame, count, theta, phi) != 0U)
      {
        continue;
      }

      best_value = workspace->srp_power[idx];
      best_theta = theta;
      best_phi = phi;
      best_idx = idx;
      found = 1U;
    }

    if (found == 0U)
    {
      break;
    }

    used[best_idx] = 1U;
    vis_frame->candidate[count].theta_deg = best_theta;
    vis_frame->candidate[count].phi_deg = best_phi;
    vis_frame->candidate[count].power = best_value;
    vis_frame->candidate[count].quality =
        App_AcousticSrp_Clamp01(best_value / App_AcousticSrp_MaxF32(ctx->weight_sum, 1.0e-6f));
    vis_frame->candidate[count].contrast = 0.0f;
    count++;
  }

  vis_frame->candidate_count = (uint8_t)count;
}

static void App_AcousticSrp_FillVisFrame(AppAcousticSrpContext_t *ctx,
                                         AppAcousticImagingVisFrame_t *vis_frame)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  float max_value;
  float second_max_raw;
  float second_max_quality;
  uint32_t max_idx;

  App_AcousticImaging_ClearVisFrame(vis_frame);
  vis_frame->algorithm = ctx->config.algorithm;
  vis_frame->active_profile = ctx->config.profile;
  vis_frame->mic_mode = ctx->config.mic_mode;
  vis_frame->frame_seq = ctx->processed_frames;
  vis_frame->grid_count = ctx->grid_count;

  for (uint32_t idx = 0U; idx < ctx->grid_count; idx++)
  {
    vis_frame->power[idx] = workspace->srp_power[idx];
    App_AcousticSrp_GetGridAngle(workspace, idx, &vis_frame->theta_deg[idx], &vis_frame->phi_deg[idx]);
  }

  arm_max_f32(workspace->srp_power, ctx->grid_count, &max_value, &max_idx);
  vis_frame->peak_idx = max_idx;
  vis_frame->peak_value = max_value;

  if ((App_AcousticSrp_IsFiniteF32(max_value) == 0U) || (max_idx >= ctx->grid_count))
  {
    vis_frame->quality = 0.0f;
    vis_frame->contrast = 0.0f;
    vis_frame->candidate_count = 0U;
    vis_frame->valid = 0U;
    return;
  }

  second_max_raw = App_AcousticSrp_SecondMaxAll(workspace, ctx->grid_count, max_idx);
  second_max_quality = App_AcousticSrp_SecondMaxExcludingNeighbor(workspace, ctx->grid_count, max_idx);

  vis_frame->contrast = (max_value - second_max_raw) / (App_AcousticSrp_AbsF32(max_value) + 1.0e-6f);
  vis_frame->quality = (max_value - second_max_quality) / (App_AcousticSrp_AbsF32(max_value) + 1.0e-6f);

  App_AcousticSrp_FillCandidates(ctx, vis_frame);
  if (vis_frame->candidate_count > 0U)
  {
    vis_frame->candidate[0].quality = vis_frame->quality;
    vis_frame->candidate[0].contrast = vis_frame->contrast;
  }

  vis_frame->valid =
      ((vis_frame->quality >= ctx->config.quality_min) &&
       (App_AcousticSrp_Clamp01(max_value / App_AcousticSrp_MaxF32(ctx->weight_sum, 1.0e-6f)) >=
        ctx->config.energy_min)) ? 1U : 0U;
}

AppAcousticImagingStatus_t App_AcousticSrp_Init(AppAcousticSrpContext_t *ctx,
                                                const AppAcousticImagingConfig_t *config,
                                                AppAcousticSrpBackend_t backend)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  AppAcousticImagingStatus_t status;
  uint32_t pair_count = 0U;
  uint32_t grid_count = 0U;
  uint32_t tdoa_count = 0U;

  if ((ctx == NULL) || (config == NULL))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  status = App_AcousticSrp_ValidateRuntimeConfig(config, backend);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  memset(workspace, 0, sizeof(*workspace));
  memset(ctx, 0, sizeof(*ctx));

  status = App_AcousticImaging_BuildPairSet(config,
                                            workspace->pairs,
                                            APP_ACOUSTIC_SRP_MAX_PAIRS,
                                            &pair_count);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  status = App_AcousticImaging_FillCoarseGrid(config,
                                              workspace->coarse_theta,
                                              workspace->coarse_phi,
                                              APP_ACOUSTIC_IMAGING_COARSE_TOTAL,
                                              &grid_count);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  status = App_AcousticImaging_FillCoarseTdoaLut(config,
                                                 workspace->pairs,
                                                 pair_count,
                                                 workspace->coarse_tdoa,
                                                 APP_ACOUSTIC_IMAGING_COARSE_TOTAL * APP_ACOUSTIC_SRP_MAX_PAIRS,
                                                 &tdoa_count);
  if ((status != APP_ACOUSTIC_IMAGING_OK) ||
      (tdoa_count != (APP_ACOUSTIC_IMAGING_COARSE_TOTAL * pair_count)))
  {
    return APP_ACOUSTIC_IMAGING_PROCESSING_FAILED;
  }

  App_AcousticSrp_FillWindow(workspace, config->frame_len);
  if (arm_rfft_fast_init_f32(&workspace->rfft, (uint16_t)config->nfft) != ARM_MATH_SUCCESS)
  {
    return APP_ACOUSTIC_IMAGING_PROCESSING_FAILED;
  }

  if (backend == APP_ACOUSTIC_BACKEND_NPU_HEATMAP)
  {
    status = App_AcousticNpuHeatmap_Init(config);
    if (status != APP_ACOUSTIC_IMAGING_OK)
    {
      return status;
    }
  }

  App_AcousticSrp_EnableCycleCounter();

  ctx->config = *config;
  ctx->backend = backend;
  ctx->pair_count = pair_count;
  ctx->active_bin_count = config->active_bin_count;
  ctx->active_bins_contiguous = App_AcousticSrp_ConfigUsesContiguousBins(config);
  ctx->grid_count = APP_ACOUSTIC_SRP_RUNTIME_GRID_TOTAL;
  ctx->active_channel_mask = config->channel_mask & ~config->bad_channel_mask;
  ctx->smoothing_valid = 0U;
  App_AcousticSrp_RebuildWeights(ctx);
  App_AcousticSrp_RebuildCoarsePhase(ctx);
  if (ctx->weight_sum <= 0.0f)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }
  ctx->initialized = 1U;

  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t App_AcousticSrp_ProcessFrame(AppAcousticSrpContext_t *ctx,
                                                        const AppAudioFrame_t *frame,
                                                        AppAcousticImagingVisFrame_t *vis_frame)
{
  AppAcousticImagingStatus_t status;
  uint32_t top_idx[APP_ACOUSTIC_IMAGING_FINE_TOP_K] = { 0U };
  uint32_t t0;
  uint32_t t1;
  uint32_t total_start;

  if ((ctx == NULL) || (frame == NULL) || (vis_frame == NULL) || (ctx->initialized == 0U))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  if (frame->format != APP_AUDIO_FRAME_FORMAT_PLANAR_F32)
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  status = App_AcousticImaging_ValidateFrame(&ctx->config, frame);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  App_AcousticSrp_UpdateActiveMask(ctx, frame);
  if (ctx->weight_sum <= 0.0f)
  {
    return APP_ACOUSTIC_IMAGING_PROCESSING_FAILED;
  }

  memset(&ctx->perf, 0, sizeof(ctx->perf));
  total_start = App_AcousticSrp_CycleNow();

  t0 = total_start;
  status = App_AcousticSrp_Preprocess(ctx, frame);
  t1 = App_AcousticSrp_CycleNow();
  ctx->perf.preprocess_cycles = App_AcousticSrp_CycleDelta(t0, t1);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  t0 = t1;
  App_AcousticSrp_RunFft(ctx);
  t1 = App_AcousticSrp_CycleNow();
  ctx->perf.fft_cycles = App_AcousticSrp_CycleDelta(t0, t1);

  t0 = t1;
  App_AcousticSrp_ComputeGccPhat(ctx);
  t1 = App_AcousticSrp_CycleNow();
  ctx->perf.gcc_cycles = App_AcousticSrp_CycleDelta(t0, t1);

  t0 = t1;
  if (ctx->backend == APP_ACOUSTIC_BACKEND_NPU_HEATMAP)
  {
    AppAcousticNpuHeatmapPerf_t npu_perf;

    status = App_AcousticNpuHeatmap_RunCoarse(&ctx->config,
                                              s_srp_workspace.gcc,
                                              ctx->pair_count,
                                              ctx->active_bin_count,
                                              s_srp_workspace.srp_power,
                                              APP_ACOUSTIC_IMAGING_COARSE_TOTAL,
                                              &npu_perf);
    t1 = App_AcousticSrp_CycleNow();
    ctx->perf.coarse_cycles = App_AcousticSrp_CycleDelta(t0, t1);
    ctx->perf.npu_quantize_cycles = npu_perf.quantize_cycles;
    ctx->perf.npu_cache_cycles = npu_perf.cache_cycles;
    ctx->perf.npu_inference_cycles = npu_perf.inference_cycles;
    ctx->perf.npu_output_cycles = npu_perf.output_cycles;
    if (status != APP_ACOUSTIC_IMAGING_OK)
    {
      return status;
    }
  }
  else
  {
    App_AcousticSrp_RunCoarseSearch(ctx);
    t1 = App_AcousticSrp_CycleNow();
    ctx->perf.coarse_cycles = App_AcousticSrp_CycleDelta(t0, t1);
  }

  App_AcousticSrp_FindTopCoarseNms(s_srp_workspace.srp_power, top_idx, ctx->config.fine_top_k);

  t0 = App_AcousticSrp_CycleNow();
  App_AcousticSrp_RunFineSearch(ctx, top_idx);
  t1 = App_AcousticSrp_CycleNow();
  ctx->perf.fine_cycles = App_AcousticSrp_CycleDelta(t0, t1);

  t0 = t1;
  App_AcousticSrp_SmoothPower(ctx);
  ctx->processed_frames++;
  App_AcousticSrp_FillVisFrame(ctx, vis_frame);
  vis_frame->frame_seq = frame->seq;
  t1 = App_AcousticSrp_CycleNow();
  ctx->perf.output_cycles = App_AcousticSrp_CycleDelta(t0, t1);
  ctx->perf.total_cycles = App_AcousticSrp_CycleDelta(total_start, t1);

  return APP_ACOUSTIC_IMAGING_OK;
}

void App_AcousticSrp_GetPerf(const AppAcousticSrpContext_t *ctx,
                             AppAcousticSrpPerf_t *perf_out)
{
  if ((ctx == NULL) || (perf_out == NULL))
  {
    return;
  }

  *perf_out = ctx->perf;
}

static void App_AcousticSrp_UpdateMaxPerf(AppAcousticSrpPerf_t *max_perf,
                                          const AppAcousticSrpPerf_t *perf)
{
  if (perf->preprocess_cycles > max_perf->preprocess_cycles)
  {
    max_perf->preprocess_cycles = perf->preprocess_cycles;
  }
  if (perf->fft_cycles > max_perf->fft_cycles)
  {
    max_perf->fft_cycles = perf->fft_cycles;
  }
  if (perf->gcc_cycles > max_perf->gcc_cycles)
  {
    max_perf->gcc_cycles = perf->gcc_cycles;
  }
  if (perf->coarse_cycles > max_perf->coarse_cycles)
  {
    max_perf->coarse_cycles = perf->coarse_cycles;
  }
  if (perf->npu_quantize_cycles > max_perf->npu_quantize_cycles)
  {
    max_perf->npu_quantize_cycles = perf->npu_quantize_cycles;
  }
  if (perf->npu_cache_cycles > max_perf->npu_cache_cycles)
  {
    max_perf->npu_cache_cycles = perf->npu_cache_cycles;
  }
  if (perf->npu_inference_cycles > max_perf->npu_inference_cycles)
  {
    max_perf->npu_inference_cycles = perf->npu_inference_cycles;
  }
  if (perf->npu_output_cycles > max_perf->npu_output_cycles)
  {
    max_perf->npu_output_cycles = perf->npu_output_cycles;
  }
  if (perf->fine_cycles > max_perf->fine_cycles)
  {
    max_perf->fine_cycles = perf->fine_cycles;
  }
  if (perf->output_cycles > max_perf->output_cycles)
  {
    max_perf->output_cycles = perf->output_cycles;
  }
  if (perf->total_cycles > max_perf->total_cycles)
  {
    max_perf->total_cycles = perf->total_cycles;
  }
}

static AppAcousticImagingStatus_t App_AcousticSrp_RunOneSyntheticCheck(AppAcousticSrpContext_t *ctx,
                                                                       AppAcousticImagingProfile_t profile,
                                                                       AppAcousticImagingVisFrame_t *vis_frame)
{
  AppAcousticImagingConfig_t config;
  AppAudioFrame_t frame;
  AppAcousticImagingStatus_t status;
  const float expected_theta = 15.0f;
  const float expected_phi = -15.0f;
  float dtheta;
  float dphi;

  if ((ctx == NULL) || (vis_frame == NULL))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  status = App_AcousticImaging_GetDefaultConfig(APP_MIC_ARRAY_MODE_WIDE32_48K,
                                                profile,
                                                &config);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  status = App_AcousticSrp_Init(ctx, &config, APP_ACOUSTIC_BACKEND_F32_CMSIS);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  status = App_AcousticSynthetic_FillPlaneWave(&config,
                                               expected_theta,
                                               expected_phi,
                                               2000.0f,
                                               0.5f,
                                               0.0f,
                                               0U,
                                               s_srp_slow_workspace.selftest_planar,
                                               APP_ACOUSTIC_SRP_MAX_FRAME_LEN,
                                               &frame);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  status = App_AcousticSrp_ProcessFrame(ctx, &frame, vis_frame);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  if ((vis_frame->valid == 0U) || (vis_frame->candidate_count == 0U))
  {
    return APP_ACOUSTIC_IMAGING_SELF_TEST_FAILED;
  }

  dtheta = App_AcousticSrp_AbsF32(vis_frame->candidate[0].theta_deg - expected_theta);
  dphi = App_AcousticSrp_AbsF32(vis_frame->candidate[0].phi_deg - expected_phi);
  if (((dtheta * dtheta) + (dphi * dphi)) > (15.0f * 15.0f))
  {
    return APP_ACOUSTIC_IMAGING_SELF_TEST_FAILED;
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t App_AcousticSrp_RunSelfTest(AppAcousticSrpContext_t *ctx,
                                                       AppAcousticImagingVisFrame_t *vis_frame)
{
  static const AppAcousticImagingProfile_t profiles[] =
  {
    APP_ACOUSTIC_IMAGING_PROFILE_FAST,
    APP_ACOUSTIC_IMAGING_PROFILE_BALANCED,
    APP_ACOUSTIC_IMAGING_PROFILE_QUALITY
  };

  if ((ctx == NULL) || (vis_frame == NULL))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  for (uint32_t i = 0U; i < (sizeof(profiles) / sizeof(profiles[0])); i++)
  {
    AppAcousticImagingStatus_t status = App_AcousticSrp_RunOneSyntheticCheck(ctx,
                                                                             profiles[i],
                                                                             vis_frame);
    if (status != APP_ACOUSTIC_IMAGING_OK)
    {
      return status;
    }
  }

  return APP_ACOUSTIC_IMAGING_OK;
}

AppAcousticImagingStatus_t App_AcousticSrp_RunSyntheticBenchmark(AppAcousticSrpContext_t *ctx,
                                                                 const AppAcousticImagingConfig_t *config,
                                                                 uint32_t frame_count,
                                                                 uint32_t cpu_hz,
                                                                 AppAcousticSrpBenchmarkResult_t *result)
{
  AppAudioFrame_t frame;
  AppAcousticImagingVisFrame_t vis_frame;
  AppAcousticImagingStatus_t status;
  uint64_t sum_preprocess = 0ULL;
  uint64_t sum_fft = 0ULL;
  uint64_t sum_gcc = 0ULL;
  uint64_t sum_coarse = 0ULL;
  uint64_t sum_fine = 0ULL;
  uint64_t sum_output = 0ULL;
  uint64_t sum_total = 0ULL;

  if ((ctx == NULL) || (config == NULL) || (result == NULL) || (frame_count == 0U))
  {
    return APP_ACOUSTIC_IMAGING_INVALID_ARGUMENT;
  }

  memset(result, 0, sizeof(*result));
  result->requested_frames = frame_count;
  result->last_status = APP_ACOUSTIC_IMAGING_OK;

  status = App_AcousticSrp_Init(ctx, config, APP_ACOUSTIC_BACKEND_F32_CMSIS);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    result->last_status = status;
    return status;
  }

  for (uint32_t seq = 0U; seq < frame_count; seq++)
  {
    status = App_AcousticSynthetic_FillPlaneWave(config,
                                                 15.0f,
                                                 -15.0f,
                                                 2000.0f,
                                                 0.5f,
                                                 0.04f,
                                                 seq,
                                                 s_srp_slow_workspace.selftest_planar,
                                                 APP_ACOUSTIC_SRP_MAX_FRAME_LEN,
                                                 &frame);
    if (status == APP_ACOUSTIC_IMAGING_OK)
    {
      status = App_AcousticSrp_ProcessFrame(ctx, &frame, &vis_frame);
    }

    result->last_status = status;
    if (status != APP_ACOUSTIC_IMAGING_OK)
    {
      result->failed_frames++;
      continue;
    }

    result->processed_frames++;
    result->last_vis_frame = vis_frame;
    sum_preprocess += ctx->perf.preprocess_cycles;
    sum_fft += ctx->perf.fft_cycles;
    sum_gcc += ctx->perf.gcc_cycles;
    sum_coarse += ctx->perf.coarse_cycles;
    sum_fine += ctx->perf.fine_cycles;
    sum_output += ctx->perf.output_cycles;
    sum_total += ctx->perf.total_cycles;
    App_AcousticSrp_UpdateMaxPerf(&result->max_perf, &ctx->perf);
  }

  if (result->processed_frames == 0U)
  {
    return result->last_status;
  }

  result->avg_perf.preprocess_cycles = (uint32_t)(sum_preprocess / result->processed_frames);
  result->avg_perf.fft_cycles = (uint32_t)(sum_fft / result->processed_frames);
  result->avg_perf.gcc_cycles = (uint32_t)(sum_gcc / result->processed_frames);
  result->avg_perf.coarse_cycles = (uint32_t)(sum_coarse / result->processed_frames);
  result->avg_perf.fine_cycles = (uint32_t)(sum_fine / result->processed_frames);
  result->avg_perf.output_cycles = (uint32_t)(sum_output / result->processed_frames);
  result->avg_perf.total_cycles = (uint32_t)(sum_total / result->processed_frames);

  if ((cpu_hz != 0U) && (result->avg_perf.total_cycles != 0U))
  {
    result->effective_fps_q8 =
        (uint32_t)(((uint64_t)cpu_hz * 256ULL) / (uint64_t)result->avg_perf.total_cycles);
  }

  return APP_ACOUSTIC_IMAGING_OK;
}
