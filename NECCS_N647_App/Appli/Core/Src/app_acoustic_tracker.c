#include "app_acoustic_tracker.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* Tuning. Angles in degrees, times in seconds unless suffixed.        */
/* ------------------------------------------------------------------ */

/* Association gate: an estimate farther than this from the prediction is
 * treated as a candidate JUMP, not an update. 25 deg > the 15 deg coarse
 * grid pitch, so grid-quantization wobble always associates. */
#define TRK_GATE_DEG                 25.0f
/* Two candidate-jump estimates must agree within this to re-seed. */
#define TRK_JUMP_AGREE_DEG           20.0f
/* Measurement noise: sigma = BASE + SCALE / max(q,1); q=2 -> ~10 deg,
 * q=5 -> ~7 deg, q>=10 -> ~5.4 deg. Squared into R. */
#define TRK_MEAS_SIGMA_BASE_DEG      4.0f
#define TRK_MEAS_SIGMA_SCALE         12.0f
/* Process noise: white angular acceleration driving the CV model. A hand-
 * held source pan is <200 deg/s^2; higher values track faster but jitter. */
#define TRK_ACCEL_SIGMA_DPS2         150.0f
/* Velocity damping while coasting (no measurement), per second. */
#define TRK_COAST_VEL_DECAY_PER_S    2.0f
/* Confidence integrator: attack per valid estimate is quality-scaled up to
 * this cap; decay is per second so it is rate-independent. */
#define TRK_CONF_ATTACK_MAX          0.45f
#define TRK_CONF_ATTACK_PER_Q        0.12f  /* per quality_pct point        */
#define TRK_CONF_DECAY_PER_S         1.1f   /* full->off in ~0.9 s of silence */
#define TRK_CONF_ON_THRESHOLD        0.30f
#define TRK_CONF_OFF_THRESHOLD      0.12f
/* Transient fast path: one estimate at/above this quality seeds the track
 * and forces the display on immediately (claps, door slams). */
#define TRK_FAST_QUALITY_PCT         6U
#define TRK_FAST_CONFIDENCE          0.85f
/* Kalman covariance bounds. */
#define TRK_P_INIT                   400.0f  /* (20 deg)^2 */
#define TRK_P_MAX                    2500.0f

typedef struct
{
  float pos;
  float vel;
  /* covariance [p00 p01; p01 p11] */
  float p00;
  float p01;
  float p11;
} TrkAxis_t;

typedef struct
{
  TrkAxis_t theta;
  TrkAxis_t phi;
  float confidence;
  uint8_t display;
  uint8_t seeded;
  /* pending jump candidate */
  uint8_t jump_pending;
  float jump_theta;
  float jump_phi;
} TrkTracker_t;

static TrkTracker_t s_trk;

static float Trk_AbsF32(float v)
{
  return (v < 0.0f) ? -v : v;
}

static float Trk_Clamp(float v, float lo, float hi)
{
  if (v < lo)
  {
    return lo;
  }
  if (v > hi)
  {
    return hi;
  }
  return v;
}

static void Trk_AxisSeed(TrkAxis_t *axis, float pos)
{
  axis->pos = pos;
  axis->vel = 0.0f;
  axis->p00 = TRK_P_INIT;
  axis->p01 = 0.0f;
  axis->p11 = TRK_P_INIT;
}

/* Constant-velocity predict step. */
static void Trk_AxisPredict(TrkAxis_t *axis, float dt)
{
  const float q_accel = TRK_ACCEL_SIGMA_DPS2 * TRK_ACCEL_SIGMA_DPS2;
  const float dt2 = dt * dt;
  const float dt3 = dt2 * dt;
  const float dt4 = dt2 * dt2;
  float p00;
  float p01;
  float p11;

  axis->pos += axis->vel * dt;

  p00 = axis->p00 + (2.0f * dt * axis->p01) + (dt2 * axis->p11) + (0.25f * dt4 * q_accel);
  p01 = axis->p01 + (dt * axis->p11) + (0.5f * dt3 * q_accel);
  p11 = axis->p11 + (dt2 * q_accel);

  axis->p00 = Trk_Clamp(p00, 0.0f, TRK_P_MAX);
  axis->p01 = Trk_Clamp(p01, -TRK_P_MAX, TRK_P_MAX);
  axis->p11 = Trk_Clamp(p11, 0.0f, TRK_P_MAX);
}

/* Position-measurement update step. */
static void Trk_AxisUpdate(TrkAxis_t *axis, float meas, float r)
{
  const float s = axis->p00 + r;
  const float k0 = (s > 1.0e-6f) ? (axis->p00 / s) : 0.0f;
  const float k1 = (s > 1.0e-6f) ? (axis->p01 / s) : 0.0f;
  const float innov = meas - axis->pos;
  const float p00 = axis->p00;
  const float p01 = axis->p01;

  axis->pos += k0 * innov;
  axis->vel += k1 * innov;
  axis->p00 = (1.0f - k0) * p00;
  axis->p01 = (1.0f - k0) * p01;
  axis->p11 = axis->p11 - (k1 * p01);
}

void AppAcousticTracker_Reset(void)
{
  memset(&s_trk, 0, sizeof(s_trk));
}

void AppAcousticTracker_Update(uint8_t valid,
                               float theta_deg,
                               float phi_deg,
                               uint8_t quality_pct,
                               uint32_t dt_ms)
{
  float dt = (float)dt_ms * 0.001f;

  if (dt <= 0.0f)
  {
    dt = 0.001f;
  }
  if (dt > 0.5f)
  {
    dt = 0.5f; /* scheduler hiccup: don't let covariance explode */
  }

  if (s_trk.seeded != 0U)
  {
    Trk_AxisPredict(&s_trk.theta, dt);
    Trk_AxisPredict(&s_trk.phi, dt);
    if (valid == 0U)
    {
      /* Coasting: bleed velocity so a lost source drifts to a stop
       * instead of sliding off-screen. */
      float decay = 1.0f - (TRK_COAST_VEL_DECAY_PER_S * dt);

      decay = Trk_Clamp(decay, 0.0f, 1.0f);
      s_trk.theta.vel *= decay;
      s_trk.phi.vel *= decay;
    }
  }

  if (valid != 0U)
  {
    const float sigma = TRK_MEAS_SIGMA_BASE_DEG +
                        (TRK_MEAS_SIGMA_SCALE / (float)((quality_pct != 0U) ? quality_pct : 1U));
    const float r = sigma * sigma;
    const uint8_t fast = (quality_pct >= TRK_FAST_QUALITY_PCT) ? 1U : 0U;
    float conf_gain;

    if (s_trk.seeded == 0U)
    {
      /* First detection seeds directly: the confidence gate (not a frame-
       * agreement rule) is what keeps lone noise blips off the screen. */
      Trk_AxisSeed(&s_trk.theta, theta_deg);
      Trk_AxisSeed(&s_trk.phi, phi_deg);
      s_trk.seeded = 1U;
      s_trk.jump_pending = 0U;
    }
    else
    {
      const float d_theta = Trk_AbsF32(theta_deg - s_trk.theta.pos);
      const float d_phi = Trk_AbsF32(phi_deg - s_trk.phi.pos);

      if ((d_theta <= TRK_GATE_DEG) && (d_phi <= TRK_GATE_DEG))
      {
        Trk_AxisUpdate(&s_trk.theta, theta_deg, r);
        Trk_AxisUpdate(&s_trk.phi, phi_deg, r);
        s_trk.jump_pending = 0U;
      }
      else if (fast != 0U)
      {
        /* Strong transient far from the track: trust it immediately. */
        Trk_AxisSeed(&s_trk.theta, theta_deg);
        Trk_AxisSeed(&s_trk.phi, phi_deg);
        s_trk.jump_pending = 0U;
      }
      else if ((s_trk.jump_pending != 0U) &&
               (Trk_AbsF32(theta_deg - s_trk.jump_theta) <= TRK_JUMP_AGREE_DEG) &&
               (Trk_AbsF32(phi_deg - s_trk.jump_phi) <= TRK_JUMP_AGREE_DEG))
      {
        /* Two consecutive estimates agree on a NEW position: the source
         * really moved. At >=15 est/s this costs ~70-130 ms, not the
         * 300-600 ms the old 60 Hz-tick rule cost at 3-7 SRP fps. */
        Trk_AxisSeed(&s_trk.theta, 0.5f * (theta_deg + s_trk.jump_theta));
        Trk_AxisSeed(&s_trk.phi, 0.5f * (phi_deg + s_trk.jump_phi));
        s_trk.jump_pending = 0U;
      }
      else
      {
        s_trk.jump_pending = 1U;
        s_trk.jump_theta = theta_deg;
        s_trk.jump_phi = phi_deg;
        /* outlier: no state update this frame */
      }
    }

    conf_gain = TRK_CONF_ATTACK_PER_Q * (float)quality_pct;
    conf_gain = Trk_Clamp(conf_gain, 0.0f, TRK_CONF_ATTACK_MAX);
    s_trk.confidence = Trk_Clamp(s_trk.confidence + conf_gain, 0.0f, 1.0f);
    if (fast != 0U)
    {
      s_trk.confidence = Trk_Clamp(s_trk.confidence, TRK_FAST_CONFIDENCE, 1.0f);
    }
  }
  else
  {
    s_trk.confidence = Trk_Clamp(s_trk.confidence - (TRK_CONF_DECAY_PER_S * dt), 0.0f, 1.0f);
  }

  /* Display hysteresis replaces the old hard 2 s hold: on at 0.30 (two
   * ordinary valid frames or one transient), off at 0.12 (~0.5 s after the
   * last evidence, sliding - not snapping - out). */
  if (s_trk.display == 0U)
  {
    if (s_trk.confidence >= TRK_CONF_ON_THRESHOLD)
    {
      s_trk.display = 1U;
    }
  }
  else
  {
    if (s_trk.confidence < TRK_CONF_OFF_THRESHOLD)
    {
      s_trk.display = 0U;
    }
  }

  if ((s_trk.display == 0U) && (s_trk.confidence <= 0.0f))
  {
    s_trk.seeded = 0U; /* fully forgotten: next detection seeds fresh */
    s_trk.jump_pending = 0U;
  }
}

void AppAcousticTracker_GetState(AppAcousticTrackerState_t *state)
{
  if (state == NULL)
  {
    return;
  }

  state->theta_deg = s_trk.theta.pos;
  state->phi_deg = s_trk.phi.pos;
  state->vel_theta_dps = s_trk.theta.vel;
  state->vel_phi_dps = s_trk.phi.vel;
  state->confidence = s_trk.confidence;
  state->display = s_trk.display;
  state->seeded = s_trk.seeded;
}
