#include "app_beam_record.h"

#include "app_acoustic_imaging.h" /* speed of sound */
#include "app_mic_array.h"

#include "stm32n6xx.h"

#include <math.h>
#include <string.h>

#define APP_BEAM_CHANNELS        APP_MIC_ARRAY_PHYSICAL_MIC_COUNT
#define APP_BEAM_FRAME_LEN       APP_AUDIO_FRAME_DEFAULT_WIDE32_FRAME_LEN
/* 94 mm aperture at 48 kHz spans +/-6.6 samples of TDOA. */
#define APP_BEAM_MAX_DELAY       8
#define APP_BEAM_TAIL            (2 * APP_BEAM_MAX_DELAY)
/* Power of two, ~2.7 s at 48 kHz. The media thread drains every 200 ms,
 * but the WAV create right after start stalls it for up to ~1 s of FAT
 * churn (board 2026-07-13: a 682 ms ring overran 445 frames there). */
#define APP_BEAM_RING_SAMPLES    131072U
#define APP_BEAM_RING_MASK       (APP_BEAM_RING_SAMPLES - 1U)
/* Coherent gain restores the source amplitude (sum/32); headphone-level
 * sources sit far below full scale, so add fixed makeup on top. */
#define APP_BEAM_MAKEUP_GAIN     8.0f
#define APP_BEAM_RMS_EMA_ALPHA   0.25f

static volatile uint8_t s_active;
static volatile uint8_t s_recording;
static volatile int16_t s_steer_theta;
static volatile int16_t s_steer_phi;

/* Applied steering + per-channel ring taps (0..2*MAX_DELAY). */
static int16_t s_applied_theta = INT16_MIN;
static int16_t s_applied_phi = INT16_MIN;
static uint8_t s_tap[APP_BEAM_CHANNELS];

/* Last APP_BEAM_TAIL samples of the previous frame per channel. */
static float s_tail[APP_BEAM_CHANNELS][APP_BEAM_TAIL];

static int16_t s_ring[APP_BEAM_RING_SAMPLES]
    __attribute__((section(".EXTRAM"), aligned(32)));
static volatile uint32_t s_ring_wr;
static volatile uint32_t s_ring_rd;

static volatile uint32_t s_frames_fed;
static volatile uint32_t s_samples_produced;
static volatile uint32_t s_ring_overruns;
static float s_rms_ema;
static volatile int8_t s_rms_dbfs = -90;

#ifdef DEBUG
/* GDB hooks: force-steer the beam without touch input (remote UI testing).
 * Write theta/phi then set the request flag; 0x7FFF theta clears override. */
volatile int16_t g_app_beam_test_theta = 0x7FFF;
volatile int16_t g_app_beam_test_phi;
#endif

static void AppBeamRecord_RebuildTaps(int16_t theta_deg, int16_t phi_deg)
{
  const float deg2rad = 0.017453293f;
  const float theta = (float)theta_deg * deg2rad;
  const float phi = (float)phi_deg * deg2rad;
  const float ux = sinf(theta) * cosf(phi);
  const float uy = sinf(phi);

  for (uint32_t channel = 0U; channel < APP_BEAM_CHANNELS; channel++)
  {
    const AppMicArrayMic_t *mic =
        App_MicArray_GetModeMic(APP_MIC_ARRAY_MODE_WIDE32_48K, channel);
    int32_t delay = 0;

    if (mic != NULL)
    {
      /* Same sign convention as the SRP steering LUT (dy flipped: the mic
       * table's +y is opposite the camera's up). tau>0 = mic hears the
       * steered source EARLY, so its taps sit deeper in the past. */
      const float tau = (((float)mic->x_0p1mm * 0.0001f) * ux -
                         ((float)mic->y_0p1mm * 0.0001f) * uy) /
                        APP_ACOUSTIC_IMAGING_SPEED_OF_SOUND_MPS;

      delay = (int32_t)lroundf(tau * (float)APP_BEAM_RECORD_SAMPLE_RATE_HZ);
      if (delay > APP_BEAM_MAX_DELAY)
      {
        delay = APP_BEAM_MAX_DELAY;
      }
      if (delay < -APP_BEAM_MAX_DELAY)
      {
        delay = -APP_BEAM_MAX_DELAY;
      }
    }

    s_tap[channel] = (uint8_t)(delay + APP_BEAM_MAX_DELAY);
  }

  s_applied_theta = theta_deg;
  s_applied_phi = phi_deg;
}

void AppBeamRecord_SetActive(uint8_t active)
{
  if ((active != 0U) && (s_active == 0U))
  {
    memset(s_tail, 0, sizeof(s_tail));
    s_rms_ema = 0.0f;
    s_rms_dbfs = -90;
  }
  s_active = (active != 0U) ? 1U : 0U;
}

void AppBeamRecord_SetSteering(int16_t theta_deg, int16_t phi_deg)
{
  s_steer_theta = theta_deg;
  s_steer_phi = phi_deg;
}

void AppBeamRecord_SetRecording(uint8_t recording)
{
  if (recording != 0U)
  {
    /* Drop anything staged while aiming. */
    s_ring_rd = s_ring_wr;
    s_samples_produced = 0U;
    s_ring_overruns = 0U;
  }
  s_recording = (recording != 0U) ? 1U : 0U;
}

uint32_t AppBeamRecord_Read(int16_t *dst, uint32_t max_samples)
{
  uint32_t count = 0U;

  if (dst == NULL)
  {
    return 0U;
  }

  while ((count < max_samples) && (s_ring_rd != s_ring_wr))
  {
    dst[count] = s_ring[s_ring_rd & APP_BEAM_RING_MASK];
    s_ring_rd++;
    count++;
  }

  return count;
}

void AppBeamRecord_FeedFrame(const AppAudioFrame_t *frame)
{
  /* Static: 1 KB accumulator would crowd the capture thread stack. Only the
   * capture thread runs this function. */
  static float acc[APP_BEAM_FRAME_LEN];
  uint32_t frame_len;

  if ((s_active == 0U) ||
      (frame == NULL) ||
      (frame->mic_mode != APP_MIC_ARRAY_MODE_WIDE32_48K) ||
      (frame->planar_f32 == NULL) ||
      (frame->frame_len == 0U) ||
      (frame->frame_len > APP_BEAM_FRAME_LEN))
  {
    return;
  }

#ifdef DEBUG
  if (g_app_beam_test_theta != 0x7FFF)
  {
    s_steer_theta = g_app_beam_test_theta;
    s_steer_phi = g_app_beam_test_phi;
  }
#endif

  if ((s_steer_theta != s_applied_theta) || (s_steer_phi != s_applied_phi))
  {
    AppBeamRecord_RebuildTaps(s_steer_theta, s_steer_phi);
  }

  frame_len = frame->frame_len;
  memset(acc, 0, frame_len * sizeof(float));

  for (uint32_t channel = 0U; channel < APP_BEAM_CHANNELS; channel++)
  {
    const float *src = App_AudioFrame_GetChannelF32(frame, channel);
    const uint32_t tap = s_tap[channel]; /* 0..APP_BEAM_TAIL */
    float *tail = s_tail[channel];
    float mean = 0.0f;

    if (src == NULL)
    {
      continue;
    }

    /* Per-channel DC removal: mic DC offsets otherwise sum COHERENTLY
     * across 32 channels while acoustic noise sums incoherently - the DC
     * ridge alone put the beam floor at -60 dBFS (board 2026-07-13). */
    for (uint32_t i = 0U; i < frame_len; i++)
    {
      mean += src[i];
    }
    mean /= (float)frame_len;

    /* Aligned stream = [tail | src]; output sample i reads index
     * (APP_BEAM_TAIL + i - tap), i.e. the first `tap` samples come from the
     * previous frame's tail, the rest straight from this frame. The tail
     * samples are stored DC-free already. */
    for (uint32_t i = 0U; i < tap; i++)
    {
      acc[i] += tail[(APP_BEAM_TAIL - tap) + i];
    }
    for (uint32_t i = tap; i < frame_len; i++)
    {
      acc[i] += src[i - tap] - mean;
    }

    for (uint32_t i = 0U; i < APP_BEAM_TAIL; i++)
    {
      tail[i] = src[(frame_len - APP_BEAM_TAIL) + i] - mean;
    }
  }

  {
    const float gain = APP_BEAM_MAKEUP_GAIN / (float)APP_BEAM_CHANNELS;
    float sum_sq = 0.0f;
    const uint8_t recording = s_recording;
    uint32_t dropped = 0U;

    for (uint32_t i = 0U; i < frame_len; i++)
    {
      float v = acc[i] * gain;

      if (v > 0.999f)
      {
        v = 0.999f;
      }
      else if (v < -0.999f)
      {
        v = -0.999f;
      }
      sum_sq += v * v;

      if (recording != 0U)
      {
        if ((s_ring_wr - s_ring_rd) < APP_BEAM_RING_SAMPLES)
        {
          s_ring[s_ring_wr & APP_BEAM_RING_MASK] = (int16_t)(v * 32767.0f);
          s_ring_wr++;
        }
        else
        {
          dropped++;
        }
      }
    }

    if (recording != 0U)
    {
      s_samples_produced += frame_len - dropped;
      if (dropped != 0U)
      {
        s_ring_overruns++;
      }
    }

    {
      const float rms = sqrtf(sum_sq / (float)frame_len);

      s_rms_ema += APP_BEAM_RMS_EMA_ALPHA * (rms - s_rms_ema);
      if (s_rms_ema > 1.0e-5f)
      {
        float db = 20.0f * log10f(s_rms_ema);

        if (db < -90.0f)
        {
          db = -90.0f;
        }
        if (db > 0.0f)
        {
          db = 0.0f;
        }
        s_rms_dbfs = (int8_t)db;
      }
      else
      {
        s_rms_dbfs = -90;
      }
    }
  }

  s_frames_fed++;
}

void AppBeamRecord_GetSnapshot(AppBeamRecordSnapshot_t *snapshot)
{
  uint32_t primask;

  if (snapshot == NULL)
  {
    return;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  snapshot->active = s_active;
  snapshot->recording = s_recording;
  snapshot->theta_deg = s_steer_theta;
  snapshot->phi_deg = s_steer_phi;
  snapshot->rms_dbfs = s_rms_dbfs;
  snapshot->frames_fed = s_frames_fed;
  snapshot->samples_produced = s_samples_produced;
  snapshot->ring_overruns = s_ring_overruns;
  if (primask == 0U)
  {
    __enable_irq();
  }
}
