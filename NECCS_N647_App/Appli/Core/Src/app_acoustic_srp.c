#include "app_acoustic_srp.h"

#include "app_acoustic_npu.h"
#include "app_acoustic_synthetic.h"

#include "arm_math.h"
#include "stm32n6xx.h"

#if defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE & 2U)
#include "arm_mve.h"
#endif

#include <math.h>
#include <stddef.h>
#include <string.h>

#define APP_ACOUSTIC_SRP_FINE_TOTAL_PER_TOP \
  (APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE * APP_ACOUSTIC_IMAGING_FINE_GRID_SIZE)
#define APP_ACOUSTIC_SRP_RUNTIME_GRID_TOTAL \
  (APP_ACOUSTIC_IMAGING_COARSE_TOTAL + APP_ACOUSTIC_IMAGING_FINE_TOTAL)
#define APP_ACOUSTIC_SRP_PHAT_EPSILON       1.0e-6f
#define APP_ACOUSTIC_SRP_NEIGHBOR_RADIUS    1U
#define APP_ACOUSTIC_SRP_CANDIDATE_MIN_DIST_DEG 15.0f
/* Lag-domain GCC engine (2026-07-19 rewrite): per-pair correlations are
 * evaluated only over the physically possible lag range (aperture/c) at 2x
 * the sample rate, via precomputed per-bin cos/sin rows. Grid steering then
 * reduces to one interpolated table lookup per pair.
 *   Wide32: |tau| <= 110.7mm/343 = 323 us -> +/-31 taps @96 kHz -> 36+margin
 *   Core16: |tau| <=  61.5mm/343 = 179 us -> +/-69 taps @384 kHz
 * Rows are power-of-two free; sized by APP_ACOUSTIC_SRP_MAX_LAGS. */
#define APP_ACOUSTIC_SRP_UPSAMPLE           2U
/* Worst case is Core16: coarse-corner TDOA (dx+dy can beat the baseline by
 * sqrt(2)) ~0.87*87mm/343 = 220us * 384 ktaps/s * 1.15 margin -> +/-101
 * taps -> 203 rows. */
#define APP_ACOUSTIC_SRP_MAX_LAGS           224U
/* Per-pair correlation rows: worst mode is QUALITY Wide32 240 pairs x 80
 * lags or Core16 120 x 148; bounded by pairs*MAX_LAGS. */
#define APP_ACOUSTIC_SRP_MAX_CORR_FLOATS    (APP_ACOUSTIC_SRP_MAX_PAIRS * APP_ACOUSTIC_SRP_MAX_LAGS)
/* Per-bin steering rows: cos+sin per (active bin x lag). Worst = Core16
 * 97 bins x 148 lags x 2. */
#define APP_ACOUSTIC_SRP_MAX_TABLE_FLOATS   (APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS * APP_ACOUSTIC_SRP_MAX_LAGS * 2U)
/* Cross-spectrum EMA across processed frames (Welch-style, pre-PHAT):
 * accum = beta*accum + cross. Coherent averaging lifts weak-source SNR and
 * dilutes single-frame sidelobe flicker; at ~15-25 SRP fps beta=0.5 spans
 * roughly 2-3 frames (~100 ms) which keeps claps detectable. */
#define APP_ACOUSTIC_SRP_CROSS_EMA_BETA     0.5f

/* Mode invariants the union sizing relies on. */
_Static_assert((APP_MIC_ARRAY_CORE16_MIC_COUNT * 512U) <=
               APP_ACOUSTIC_SRP_MAX_TIME_SAMPLES,
               "Core16 time staging exceeds workspace");
_Static_assert((APP_ACOUSTIC_IMAGING_WIDE32_QUALITY_PAIRS * 40U) <=
               APP_ACOUSTIC_SRP_MAX_PAIR_BINS,
               "Wide32 pair*bin staging exceeds workspace");
_Static_assert((APP_ACOUSTIC_IMAGING_CORE16_QUALITY_PAIRS * 97U) <=
               APP_ACOUSTIC_SRP_MAX_PAIR_BINS,
               "Core16 pair*bin staging exceeds workspace");
typedef struct
{
  AppAcousticImagingPair_t pairs[APP_ACOUSTIC_SRP_MAX_PAIRS];
  float coarse_theta[APP_ACOUSTIC_IMAGING_COARSE_TOTAL];
  float coarse_phi[APP_ACOUSTIC_IMAGING_COARSE_TOTAL];
  float coarse_tdoa[APP_ACOUSTIC_IMAGING_COARSE_TOTAL * APP_ACOUSTIC_SRP_MAX_PAIRS];
  float window[APP_ACOUSTIC_SRP_MAX_NFFT];
  float time[APP_ACOUSTIC_SRP_MAX_TIME_SAMPLES];
  float freq[APP_ACOUSTIC_SRP_MAX_TIME_SAMPLES];
  float gcc[APP_ACOUSTIC_SRP_MAX_PAIR_BINS * 2U];
  float cross_accum[APP_ACOUSTIC_SRP_MAX_PAIR_BINS * 2U];
  float srp_weight[APP_ACOUSTIC_SRP_MAX_PAIR_BINS];
  float srp_power[APP_ACOUSTIC_SRP_RUNTIME_GRID_TOTAL];
  float smoothed_power[APP_ACOUSTIC_SRP_RUNTIME_GRID_TOTAL];
  float fine_theta[APP_ACOUSTIC_IMAGING_FINE_TOTAL];
  float fine_phi[APP_ACOUSTIC_IMAGING_FINE_TOTAL];
  float tau[APP_ACOUSTIC_SRP_MAX_PAIRS];
  float scratch_conj[APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS * 2U];
  float scratch_cross[APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS * 2U];
  float scratch_mag[APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS];
  /* Lag-domain GCC engine state: per-pair correlation rows over the
   * physical lag range, plus the per-(bin x lag) steering tables used to
   * synthesize them from the whitened cross-spectra. */
  float corr[APP_ACOUSTIC_SRP_MAX_CORR_FLOATS];
  float lag_cos[APP_ACOUSTIC_SRP_MAX_TABLE_FLOATS / 2U];
  float lag_sin[APP_ACOUSTIC_SRP_MAX_TABLE_FLOATS / 2U];
  uint8_t pair_active[APP_ACOUSTIC_SRP_MAX_PAIRS];
  uint32_t lag_count;  /* taps per correlation row (odd, centered)         */
  float lag_center;    /* row index of lag 0                               */
  float lag_scale;     /* seconds -> taps (= fs * UPSAMPLE)                */
  uint8_t cross_valid;
  arm_rfft_fast_instance_f32 rfft;
} AppAcousticSrpWorkspace_t;

typedef struct
{
  float selftest_planar[APP_ACOUSTIC_SRP_MAX_TIME_SAMPLES];
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

  /* Per-mode frame geometry: Wide32 32ch x 256, Core16 16ch x 512. */
  if (config->mic_mode == APP_MIC_ARRAY_MODE_WIDE32_48K)
  {
    if ((config->frame_len != 256U) || (config->nfft != 256U))
    {
      return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
    }
  }
  else if (config->mic_mode == APP_MIC_ARRAY_MODE_CORE16_192K)
  {
    if ((config->frame_len != 512U) || (config->nfft != 512U))
    {
      return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
    }
  }
  else
  {
    return APP_ACOUSTIC_IMAGING_UNSUPPORTED_MODE;
  }

  if ((config->channel_count > APP_ACOUSTIC_SRP_MAX_CHANNELS) ||
      ((config->channel_count * config->nfft) > APP_ACOUSTIC_SRP_MAX_TIME_SAMPLES) ||
      (config->pair_count > APP_ACOUSTIC_SRP_MAX_PAIRS) ||
      (active_bins > APP_ACOUSTIC_SRP_MAX_ACTIVE_BINS) ||
      (((uint32_t)config->pair_count * active_bins) > APP_ACOUSTIC_SRP_MAX_PAIR_BINS) ||
      ((backend == APP_ACOUSTIC_BACKEND_NPU_HEATMAP) &&
       ((config->mic_mode != APP_MIC_ARRAY_MODE_WIDE32_48K) ||
        (config->profile != APP_ACOUSTIC_IMAGING_PROFILE_BALANCED) ||
        (config->pair_count != APP_ACOUSTIC_IMAGING_WIDE32_BALANCED_PAIRS) ||
        (active_bins != 40U) ||
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

    workspace->pair_active[pair] = pair_active;

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

  /* Active-set change invalidates the cross-spectrum history (stale phase
   * relations from a different mic set would leak into the average). */
  workspace->cross_valid = 0U;
}

/* --------------------------------------------------------------------- */
/* Lag-domain GCC engine: instead of rotating a phasor per grid x pair x
 * bin (~600k serially-dependent complex MACs, 88M cycles measured) or one
 * IRFFT per pair (35M cycles, IFFT variant tried 2026-07-19), synthesize
 * each pair's correlation ONLY over the physically possible lag window
 * (aperture/c, +/-31 taps at 2x rate for Wide32) from per-(bin x lag)
 * cos/sin tables. Grid steering is then one interpolated lookup per pair.
 * Per frame: pairs x bins x lags independent MACs (~0.6M flops, vector-
 * izable) + grid x pair lookups.                                          */
/* --------------------------------------------------------------------- */

/* Correlation value at a fractional lag (in taps, relative to lag 0 at
 * row center). Range is guaranteed by construction; clamp for safety. */
static inline float App_AcousticSrp_CorrLookup(const AppAcousticSrpWorkspace_t *workspace,
                                               const float *row,
                                               float lag_taps)
{
  float pos = workspace->lag_center + lag_taps;
  float floor_pos;
  float frac;
  uint32_t idx0;

  if (pos < 0.0f)
  {
    pos = 0.0f;
  }
  if (pos > ((float)workspace->lag_count - 1.001f))
  {
    pos = (float)workspace->lag_count - 1.001f;
  }

  floor_pos = floorf(pos);
  frac = pos - floor_pos;
  idx0 = (uint32_t)floor_pos;

  return row[idx0] + (frac * (row[idx0 + 1U] - row[idx0]));
}

/* Per-pair correlation rows r_p(l) = sum_bins Gre*cos(w_b*t_l) +
 * Gim*sin(w_b*t_l) from the weighted whitened spectrum (workspace->gcc). */
static void App_AcousticSrp_BuildCorrelations(const AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  const uint32_t lag_count = workspace->lag_count;

  for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
  {
    const float *gcc = &workspace->gcc[pair * ctx->active_bin_count * 2U];
    float *row = &workspace->corr[pair * lag_count];

    if (workspace->pair_active[pair] == 0U)
    {
      continue; /* row is never read: lookups skip inactive pairs */
    }

    memset(row, 0, lag_count * sizeof(float));
    for (uint32_t bin = 0U; bin < ctx->active_bin_count; bin++)
    {
      const float g_re = gcc[2U * bin];
      const float g_im = gcc[(2U * bin) + 1U];
      const float *crow = &workspace->lag_cos[bin * lag_count];
      const float *srow = &workspace->lag_sin[bin * lag_count];

#if defined(__ARM_FEATURE_MVE) && (__ARM_FEATURE_MVE & 2U)
      uint32_t lag = 0U;

      for (; (lag + 4U) <= lag_count; lag += 4U)
      {
        float32x4_t acc = vld1q_f32(&row[lag]);

        acc = vfmaq_n_f32(acc, vld1q_f32(&crow[lag]), g_re);
        acc = vfmaq_n_f32(acc, vld1q_f32(&srow[lag]), g_im);
        vst1q_f32(&row[lag], acc);
      }
      for (; lag < lag_count; lag++)
      {
        row[lag] += (g_re * crow[lag]) + (g_im * srow[lag]);
      }
#else
      for (uint32_t lag = 0U; lag < lag_count; lag++)
      {
        row[lag] += (g_re * crow[lag]) + (g_im * srow[lag]);
      }
#endif
    }
  }
}

/* Steering tables: angle(bin, lag) = 2*pi*f_bin*t_lag with t_lag spanning
 * the physical +/- max-baseline TDOA at UPSAMPLE x sample rate. */
static void App_AcousticSrp_BuildLagTables(AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  float max_abs_tau = 0.0f;
  uint32_t lag_half;

  /* Window from the ACTUAL steering LUT, not the baseline: at the +/-60 deg
   * grid corners tau = (dx*stcp + dy*sp)/c can exceed baseline/c by up to
   * sqrt(2) (both projections add). Undersizing clamps corner grid points
   * to the row edge and corrupts the coarse surface (self-test caught it:
   * synthetic peak landed 30 deg off). 1.15 margin covers the fine search
   * pushing 10 deg past the corners. */
  for (uint32_t i = 0U; i < (APP_ACOUSTIC_IMAGING_COARSE_TOTAL * ctx->pair_count); i++)
  {
    float abs_tau = App_AcousticSrp_AbsF32(workspace->coarse_tdoa[i]);

    if (abs_tau > max_abs_tau)
    {
      max_abs_tau = abs_tau;
    }
  }

  workspace->lag_scale = (float)ctx->config.sample_rate_hz * (float)APP_ACOUSTIC_SRP_UPSAMPLE;
  lag_half = (uint32_t)(max_abs_tau * 1.15f * workspace->lag_scale) + 4U;
  workspace->lag_count = (2U * lag_half) + 1U;
  if (workspace->lag_count > APP_ACOUSTIC_SRP_MAX_LAGS)
  {
    workspace->lag_count = APP_ACOUSTIC_SRP_MAX_LAGS;
    lag_half = (workspace->lag_count - 1U) / 2U;
  }
  workspace->lag_center = (float)lag_half;

  for (uint32_t bin = 0U; bin < ctx->active_bin_count; bin++)
  {
    /* 360 deg * f_bin * one-tap step: f_bin = abs_bin * fs / nfft and a
     * tap is 1 / (fs * UPSAMPLE) -> step_deg = 360 * abs_bin / (nfft * U). */
    const float step_deg = 360.0f * (float)ctx->config.active_bins[bin] /
                           ((float)ctx->config.nfft * (float)APP_ACOUSTIC_SRP_UPSAMPLE);
    float *crow = &workspace->lag_cos[bin * workspace->lag_count];
    float *srow = &workspace->lag_sin[bin * workspace->lag_count];

    for (uint32_t lag = 0U; lag < workspace->lag_count; lag++)
    {
      float angle_deg = step_deg * ((float)lag - workspace->lag_center);

      /* arm_sin_cos_f32 expects [-180, 180); wrap without fmodf. */
      while (angle_deg >= 180.0f)
      {
        angle_deg -= 360.0f;
      }
      while (angle_deg < -180.0f)
      {
        angle_deg += 360.0f;
      }
      arm_sin_cos_f32(angle_deg, &srow[lag], &crow[lag]);
    }
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

/* SRP power for one steering direction: per-pair correlation lookup at the
 * direction's TDOA. Replaces the per-bin phasor accumulation (the lag domain
 * already integrates all bins; weights were folded in before the IRFFT). */
static float App_AcousticSrp_SrpPowerAtTau(const AppAcousticSrpContext_t *ctx,
                                           const float *tau_seconds)
{
  const AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  const uint32_t lag_count = workspace->lag_count;
  const float lag_scale = workspace->lag_scale;
  float power = 0.0f;

  for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
  {
    if (workspace->pair_active[pair] == 0U)
    {
      continue;
    }

    power += App_AcousticSrp_CorrLookup(workspace,
                                        &workspace->corr[pair * lag_count],
                                        tau_seconds[pair] * lag_scale);
  }

  return power;
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

static void App_AcousticSrp_ComputeGccPhat(AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;
  const float beta = APP_ACOUSTIC_SRP_CROSS_EMA_BETA;
  const uint8_t history_valid = workspace->cross_valid;

  for (uint32_t pair = 0U; pair < ctx->pair_count; pair++)
  {
    uint32_t mic_a = workspace->pairs[pair].mic_a;
    uint32_t mic_b = workspace->pairs[pair].mic_b;
    const float *weight = &workspace->srp_weight[pair * ctx->active_bin_count];
    float *accum = &workspace->cross_accum[pair * ctx->active_bin_count * 2U];
    float *out = &workspace->gcc[pair * ctx->active_bin_count * 2U];

    if (workspace->pair_active[pair] == 0U)
    {
      memset(out, 0, ctx->active_bin_count * 2U * sizeof(float));
      memset(accum, 0, ctx->active_bin_count * 2U * sizeof(float));
      continue;
    }

    /* Raw cross-spectrum of the current frame into scratch_cross. */
    if (ctx->active_bins_contiguous != 0U)
    {
      const float *xi = &workspace->freq[(mic_a * ctx->config.nfft) + (2U * ctx->config.active_bin_start)];
      const float *xj = &workspace->freq[(mic_b * ctx->config.nfft) + (2U * ctx->config.active_bin_start)];

      arm_cmplx_conj_f32(xj, workspace->scratch_conj, ctx->active_bin_count);
      arm_cmplx_mult_cmplx_f32(xi, workspace->scratch_conj, workspace->scratch_cross, ctx->active_bin_count);
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

        workspace->scratch_cross[2U * bin] = (ar * br) + (ai * bi);
        workspace->scratch_cross[(2U * bin) + 1U] = (ai * br) - (ar * bi);
      }
    }

    /* Welch-style running average of the cross-spectrum BEFORE the PHAT
     * whitening: coherent phase adds up across frames, uncorrelated noise
     * and single-frame sidelobes average down. */
    if (history_valid != 0U)
    {
      for (uint32_t i = 0U; i < ctx->active_bin_count * 2U; i++)
      {
        accum[i] = (beta * accum[i]) + workspace->scratch_cross[i];
      }
    }
    else
    {
      memcpy(accum, workspace->scratch_cross,
             ctx->active_bin_count * 2U * sizeof(float));
    }

    /* PHAT whitening of the averaged spectrum, per-bin weight folded in
     * (the lag-domain search cannot apply per-bin weights anymore). */
    arm_cmplx_mag_f32(accum, workspace->scratch_mag, ctx->active_bin_count);
    for (uint32_t bin = 0U; bin < ctx->active_bin_count; bin++)
    {
      float w = weight[bin] / (workspace->scratch_mag[bin] + APP_ACOUSTIC_SRP_PHAT_EPSILON);

      out[2U * bin] = accum[2U * bin] * w;
      out[(2U * bin) + 1U] = accum[(2U * bin) + 1U] * w;
    }
  }

  workspace->cross_valid = 1U;
}

static void App_AcousticSrp_RunCoarseSearch(const AppAcousticSrpContext_t *ctx)
{
  AppAcousticSrpWorkspace_t *workspace = &s_srp_workspace;

  for (uint32_t grid = 0U; grid < APP_ACOUSTIC_IMAGING_COARSE_TOTAL; grid++)
  {
    workspace->srp_power[grid] =
        App_AcousticSrp_SrpPowerAtTau(ctx, &workspace->coarse_tdoa[grid * ctx->pair_count]);
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

        workspace->srp_power[APP_ACOUSTIC_IMAGING_COARSE_TOTAL + fine_idx] =
            App_AcousticSrp_SrpPowerAtTau(ctx, workspace->tau);
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

  workspace->cross_valid = 0U;

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
  App_AcousticSrp_BuildLagTables(ctx);
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
  AppAcousticSrpPerf_t perf;
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

  /* Stage timings accumulate in a local; ctx->perf is published once at the
   * end so concurrent snapshot readers never see a half-filled struct
   * (the old memset-then-fill pattern showed "total=0, coarse=32M" tears
   * on the system page). */
  memset(&perf, 0, sizeof(perf));
  total_start = App_AcousticSrp_CycleNow();

  t0 = total_start;
  status = App_AcousticSrp_Preprocess(ctx, frame);
  t1 = App_AcousticSrp_CycleNow();
  perf.preprocess_cycles = App_AcousticSrp_CycleDelta(t0, t1);
  if (status != APP_ACOUSTIC_IMAGING_OK)
  {
    return status;
  }

  t0 = t1;
  App_AcousticSrp_RunFft(ctx);
  t1 = App_AcousticSrp_CycleNow();
  perf.fft_cycles = App_AcousticSrp_CycleDelta(t0, t1);

  t0 = t1;
  App_AcousticSrp_ComputeGccPhat(ctx);
  t1 = App_AcousticSrp_CycleNow();
  perf.gcc_cycles = App_AcousticSrp_CycleDelta(t0, t1);

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
    perf.coarse_cycles = App_AcousticSrp_CycleDelta(t0, t1);
    perf.npu_quantize_cycles = npu_perf.quantize_cycles;
    perf.npu_cache_cycles = npu_perf.cache_cycles;
    perf.npu_inference_cycles = npu_perf.inference_cycles;
    perf.npu_output_cycles = npu_perf.output_cycles;
    if (status != APP_ACOUSTIC_IMAGING_OK)
    {
      return status;
    }
  }
  else
  {
    /* Correlation-row build replaces the per-grid phasor accumulation; its
     * cycles ride in npu_quantize_cycles (unused on the F32 path) so the
     * bench scripts can split it from the grid lookups. */
    App_AcousticSrp_BuildCorrelations(ctx);
    t1 = App_AcousticSrp_CycleNow();
    perf.npu_quantize_cycles = App_AcousticSrp_CycleDelta(t0, t1);
    App_AcousticSrp_RunCoarseSearch(ctx);
    t1 = App_AcousticSrp_CycleNow();
    perf.coarse_cycles = App_AcousticSrp_CycleDelta(t0, t1);
  }

  App_AcousticSrp_FindTopCoarseNms(s_srp_workspace.srp_power, top_idx, ctx->config.fine_top_k);

  t0 = App_AcousticSrp_CycleNow();
  App_AcousticSrp_RunFineSearch(ctx, top_idx);
  t1 = App_AcousticSrp_CycleNow();
  perf.fine_cycles = App_AcousticSrp_CycleDelta(t0, t1);

  t0 = t1;
  App_AcousticSrp_SmoothPower(ctx);
  ctx->processed_frames++;
  App_AcousticSrp_FillVisFrame(ctx, vis_frame);
  vis_frame->frame_seq = frame->seq;
  t1 = App_AcousticSrp_CycleNow();
  perf.output_cycles = App_AcousticSrp_CycleDelta(t0, t1);
  perf.total_cycles = App_AcousticSrp_CycleDelta(total_start, t1);
  ctx->perf = perf;

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

  /* Stride is the CONFIG frame length: channel_count x frame_len fills the
   * staging buffer exactly in both modes; the old MAX_FRAME_LEN stride
   * would overflow it for Wide32 once the max moved to 512. */
  status = App_AcousticSynthetic_FillPlaneWave(&config,
                                               expected_theta,
                                               expected_phi,
                                               2000.0f,
                                               0.5f,
                                               0.0f,
                                               0U,
                                               s_srp_slow_workspace.selftest_planar,
                                               config.frame_len,
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
                                                 config->frame_len,
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
