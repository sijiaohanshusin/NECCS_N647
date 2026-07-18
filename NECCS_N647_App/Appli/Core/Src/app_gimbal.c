#include "app_gimbal.h"

#include "main.h"

#include "stm32n6xx.h"

/* ------------------------------------------------------------------ */
/* Pin map: defined once in the .ioc (GIMBAL_x / LASER_EN labels) and  */
/* consumed here via the generated main.h macros; audited against the  */
/* power-board + expansion-board netlists (see header). PE13/PE14      */
/* double as TIM1_CH3/CH4 on AF1 (ST N657 datasheet + NUCLEO example). */
/* ------------------------------------------------------------------ */
#define APP_GIMBAL_PAN_PORT        GIMBAL_PAN_PWM_GPIO_Port
#define APP_GIMBAL_PAN_PIN         GIMBAL_PAN_PWM_Pin    /* PE13 TIM1_CH3, FPC12/GPIO1 */
#define APP_GIMBAL_TILT_PORT       GIMBAL_TILT_PWM_GPIO_Port
#define APP_GIMBAL_TILT_PIN        GIMBAL_TILT_PWM_Pin   /* PE14 TIM1_CH4, FPC13/GPIO2 */
#define APP_GIMBAL_LASER_PORT      LASER_EN_GPIO_Port
#define APP_GIMBAL_LASER_PIN       LASER_EN_Pin          /* PC10, FPC11/ENB2 = U6 3.3 V LDO EN */
#define APP_GIMBAL_RELAY_PORT      GIMBAL_RELAY_EN_GPIO_Port
#define APP_GIMBAL_RELAY_PIN       GIMBAL_RELAY_EN_Pin   /* PD0, FPC15/GPIO4 (U8 only) */
#define APP_GIMBAL_PWR_PORT        GIMBAL_PWR_EN_GPIO_Port
#define APP_GIMBAL_PWR_PIN         GIMBAL_PWR_EN_Pin     /* PC7, FPC10/ENB1 (7.4 V buck) */
/* Level that turns the 7.4 V servo buck ON. Verify on hardware: assumes the
 * U5 buck runs when EN is driven high and stops when driven low. */
#define APP_GIMBAL_PWR_ON_LEVEL    GPIO_PIN_SET
#define APP_GIMBAL_PWR_OFF_LEVEL   GPIO_PIN_RESET
/* HAL tick runs at TX_TIMER_TICKS_PER_SECOND (100 Hz): one HAL "ms" is
 * 10 real milliseconds on this build. */
#define APP_GIMBAL_HAL_TICK_HZ     100U
/* Servo boot settle after raising the rail, before PWM starts: 5 HAL ticks
 * = 50 ms real. */
#define APP_GIMBAL_PWR_SETTLE_TICKS 5U

/* Servo timing: 50 Hz frame, 500..2500 us pulse (LD-1501MG/LD-3015MG). */
#define APP_GIMBAL_FRAME_US        20000U
#define APP_GIMBAL_PULSE_MIN_US    500U
#define APP_GIMBAL_PULSE_MAX_US    2500U
#define APP_GIMBAL_PULSE_CENTER_US 1500U
/* Slew limit: full 2000 us throw in ~2.5 s keeps the laser sweep smooth
 * and the servo current spikes bounded. */
#define APP_GIMBAL_SLEW_US_PER_S   800U

/* Per-axis acoustic-angle -> pulse mapping. us_per_deg is signed so an
 * axis can be inverted during calibration without rewiring. Values below
 * are 180-degree-servo defaults; REAL values come from on-hardware
 * calibration (point at known angles, adjust center/scale/limits). */
typedef struct
{
  int16_t center_us;
  int16_t us_per_deg_x10; /* 111 = 11.1 us/deg */
  int16_t min_us;
  int16_t max_us;
} AppGimbalAxisCal_t;

static AppGimbalAxisCal_t s_cal_pan = { 1500, 111, 600, 2400 };
static AppGimbalAxisCal_t s_cal_tilt = { 1500, -111, 900, 2100 };

static TIM_HandleTypeDef s_tim1;
static uint8_t s_initialized;
static uint8_t s_enabled;
static uint8_t s_power_on;
static uint8_t s_laser_on;
static uint8_t s_relay_on;
static volatile int16_t s_target_theta;
static volatile int16_t s_target_phi;
static uint16_t s_target_pan_us = APP_GIMBAL_PULSE_CENTER_US;
static uint16_t s_target_tilt_us = APP_GIMBAL_PULSE_CENTER_US;
static uint16_t s_live_pan_us = APP_GIMBAL_PULSE_CENTER_US;
static uint16_t s_live_tilt_us = APP_GIMBAL_PULSE_CENTER_US;
static uint32_t s_last_poll_ms;

#ifdef DEBUG
/* GDB bring-up hooks: theta/phi injection + laser/relay/enable bits.
 * request: 1=point 2=laser-toggle 3=relay-toggle 4=enable-toggle */
volatile uint32_t g_app_gimbal_test_request;
volatile int16_t g_app_gimbal_test_theta;
volatile int16_t g_app_gimbal_test_phi;
#endif

static uint16_t AppGimbal_ClampPulse(int32_t pulse_us, const AppGimbalAxisCal_t *cal)
{
  if (pulse_us < cal->min_us)
  {
    pulse_us = cal->min_us;
  }
  if (pulse_us > cal->max_us)
  {
    pulse_us = cal->max_us;
  }
  if (pulse_us < (int32_t)APP_GIMBAL_PULSE_MIN_US)
  {
    pulse_us = (int32_t)APP_GIMBAL_PULSE_MIN_US;
  }
  if (pulse_us > (int32_t)APP_GIMBAL_PULSE_MAX_US)
  {
    pulse_us = (int32_t)APP_GIMBAL_PULSE_MAX_US;
  }
  return (uint16_t)pulse_us;
}

int32_t AppGimbal_Init(void)
{
  GPIO_InitTypeDef gpio = {0};
  TIM_OC_InitTypeDef oc = {0};
  uint32_t timer_clk_hz;

  if (s_initialized != 0U)
  {
    return 0;
  }

  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_TIM1_CLK_ENABLE();

  /* Laser + relay + servo rail enable: plain outputs, all off. */
  HAL_GPIO_WritePin(APP_GIMBAL_LASER_PORT, APP_GIMBAL_LASER_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(APP_GIMBAL_RELAY_PORT, APP_GIMBAL_RELAY_PIN, GPIO_PIN_RESET);
  HAL_GPIO_WritePin(APP_GIMBAL_PWR_PORT, APP_GIMBAL_PWR_PIN, APP_GIMBAL_PWR_OFF_LEVEL);
  gpio.Mode = GPIO_MODE_OUTPUT_PP;
  gpio.Pull = GPIO_NOPULL;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  gpio.Pin = APP_GIMBAL_LASER_PIN;
  HAL_GPIO_Init(APP_GIMBAL_LASER_PORT, &gpio);
  gpio.Pin = APP_GIMBAL_RELAY_PIN;
  HAL_GPIO_Init(APP_GIMBAL_RELAY_PORT, &gpio);
  gpio.Pin = APP_GIMBAL_PWR_PIN;
  HAL_GPIO_Init(APP_GIMBAL_PWR_PORT, &gpio);

  /* Servo PWM pins. */
  gpio.Mode = GPIO_MODE_AF_PP;
  gpio.Alternate = GPIO_AF1_TIM1;
  gpio.Speed = GPIO_SPEED_FREQ_LOW;
  gpio.Pin = APP_GIMBAL_PAN_PIN;
  HAL_GPIO_Init(APP_GIMBAL_PAN_PORT, &gpio);
  gpio.Pin = APP_GIMBAL_TILT_PIN;
  HAL_GPIO_Init(APP_GIMBAL_TILT_PORT, &gpio);

  /* 1 MHz counter -> CCR is directly the pulse width in microseconds.
   * On N6 every TIM kernel clock is timg_ck = sysclk >> TIMPRE (there is no
   * classic 2x-APB rule); HAL_RCCEx_GetTIMGFreq() reads the live dividers. */
  timer_clk_hz = HAL_RCCEx_GetTIMGFreq();
  if (timer_clk_hz < 1000000U)
  {
    timer_clk_hz = 1000000U;
  }

  s_tim1.Instance = TIM1;
  s_tim1.Init.Prescaler = (timer_clk_hz / 1000000U) - 1U;
  s_tim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  s_tim1.Init.Period = APP_GIMBAL_FRAME_US - 1U;
  s_tim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  s_tim1.Init.RepetitionCounter = 0U;
  s_tim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_PWM_Init(&s_tim1) != HAL_OK)
  {
    return -1;
  }

  oc.OCMode = TIM_OCMODE_PWM1;
  oc.Pulse = APP_GIMBAL_PULSE_CENTER_US;
  oc.OCPolarity = TIM_OCPOLARITY_HIGH;
  oc.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  oc.OCFastMode = TIM_OCFAST_DISABLE;
  oc.OCIdleState = TIM_OCIDLESTATE_RESET;
  oc.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if ((HAL_TIM_PWM_ConfigChannel(&s_tim1, &oc, TIM_CHANNEL_3) != HAL_OK) ||
      (HAL_TIM_PWM_ConfigChannel(&s_tim1, &oc, TIM_CHANNEL_4) != HAL_OK))
  {
    return -1;
  }

  s_last_poll_ms = HAL_GetTick();
  s_initialized = 1U;
  return 0;
}

/* The 7.4 V rail feeds BOTH the servos and (through the power-board U6 LDO,
 * whose input hangs on that rail) the laser 3.3 V. Keep it up while either
 * user is active; settle only on an off->on edge. */
static void AppGimbal_RailAcquire(void)
{
  if (s_power_on == 0U)
  {
    HAL_GPIO_WritePin(APP_GIMBAL_PWR_PORT, APP_GIMBAL_PWR_PIN, APP_GIMBAL_PWR_ON_LEVEL);
    s_power_on = 1U;
    HAL_Delay(APP_GIMBAL_PWR_SETTLE_TICKS);
  }
}

static void AppGimbal_RailReleaseIfIdle(void)
{
  if ((s_enabled == 0U) && (s_laser_on == 0U) && (s_power_on != 0U))
  {
    HAL_GPIO_WritePin(APP_GIMBAL_PWR_PORT, APP_GIMBAL_PWR_PIN, APP_GIMBAL_PWR_OFF_LEVEL);
    s_power_on = 0U;
  }
}

void AppGimbal_SetEnabled(uint8_t enabled)
{
  if (s_initialized == 0U)
  {
    if (AppGimbal_Init() != 0)
    {
      return;
    }
  }

  if ((enabled != 0U) && (s_enabled == 0U))
  {
    /* Rail first (power-board ENB1 -> 7.4 V buck), settle, then PWM so the
     * servos never see pulses while their supply is still ramping. */
    AppGimbal_RailAcquire();
    (void)HAL_TIM_PWM_Start(&s_tim1, TIM_CHANNEL_3);
    (void)HAL_TIM_PWM_Start(&s_tim1, TIM_CHANNEL_4);
    s_enabled = 1U;
  }
  else if ((enabled == 0U) && (s_enabled != 0U))
  {
    (void)HAL_TIM_PWM_Stop(&s_tim1, TIM_CHANNEL_3);
    (void)HAL_TIM_PWM_Stop(&s_tim1, TIM_CHANNEL_4);
    s_enabled = 0U;
    /* Rail off only when the laser is not riding on it (U6 LDO input). */
    AppGimbal_RailReleaseIfIdle();
  }
}

void AppGimbal_PointAt(int16_t theta_deg, int16_t phi_deg)
{
  int32_t pan;
  int32_t tilt;

  s_target_theta = theta_deg;
  s_target_phi = phi_deg;

  pan = (int32_t)s_cal_pan.center_us +
        (((int32_t)theta_deg * (int32_t)s_cal_pan.us_per_deg_x10) / 10);
  tilt = (int32_t)s_cal_tilt.center_us +
         (((int32_t)phi_deg * (int32_t)s_cal_tilt.us_per_deg_x10) / 10);

  s_target_pan_us = AppGimbal_ClampPulse(pan, &s_cal_pan);
  s_target_tilt_us = AppGimbal_ClampPulse(tilt, &s_cal_tilt);
}

void AppGimbal_SetLaser(uint8_t on)
{
  if (s_initialized == 0U)
  {
    if (AppGimbal_Init() != 0)
    {
      return;
    }
  }
  if (on != 0U)
  {
    /* Laser = PC10/ENB2 gating the U6 3.3 V LDO, whose INPUT is the 7.4 V
     * rail: raise the rail first or the LDO has nothing to regulate. */
    AppGimbal_RailAcquire();
    s_laser_on = 1U;
    HAL_GPIO_WritePin(APP_GIMBAL_LASER_PORT, APP_GIMBAL_LASER_PIN, GPIO_PIN_SET);
  }
  else
  {
    HAL_GPIO_WritePin(APP_GIMBAL_LASER_PORT, APP_GIMBAL_LASER_PIN, GPIO_PIN_RESET);
    s_laser_on = 0U;
    AppGimbal_RailReleaseIfIdle();
  }
}

void AppGimbal_SetRelay(uint8_t on)
{
  if (s_initialized == 0U)
  {
    if (AppGimbal_Init() != 0)
    {
      return;
    }
  }
  s_relay_on = (on != 0U) ? 1U : 0U;
  HAL_GPIO_WritePin(APP_GIMBAL_RELAY_PORT, APP_GIMBAL_RELAY_PIN,
                    (s_relay_on != 0U) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static uint16_t AppGimbal_Slew(uint16_t live, uint16_t target, uint32_t max_step)
{
  if (live < target)
  {
    const uint32_t diff = (uint32_t)(target - live);

    return (uint16_t)(live + ((diff < max_step) ? diff : max_step));
  }
  if (live > target)
  {
    const uint32_t diff = (uint32_t)(live - target);

    return (uint16_t)(live - ((diff < max_step) ? diff : max_step));
  }
  return live;
}

void AppGimbal_Poll(void)
{
  uint32_t now;
  uint32_t elapsed_ms;
  uint32_t max_step;

#ifdef DEBUG
  /* Test hooks run before the init gate: Set* helpers lazy-init, so a GDB
   * poke is enough to bring the module up on an otherwise idle build. */
  if (g_app_gimbal_test_request != 0U)
  {
    const uint32_t request = g_app_gimbal_test_request;

    g_app_gimbal_test_request = 0U;
    switch (request)
    {
    case 1U:
      AppGimbal_SetEnabled(1U);
      AppGimbal_PointAt(g_app_gimbal_test_theta, g_app_gimbal_test_phi);
      break;
    case 2U:
      AppGimbal_SetLaser((s_laser_on != 0U) ? 0U : 1U);
      break;
    case 3U:
      AppGimbal_SetRelay((s_relay_on != 0U) ? 0U : 1U);
      break;
    case 4U:
      AppGimbal_SetEnabled((s_enabled != 0U) ? 0U : 1U);
      break;
    default:
      break;
    }
  }
#endif

  if (s_initialized == 0U)
  {
    return;
  }

  now = HAL_GetTick();
  elapsed_ms = now - s_last_poll_ms;
  if (elapsed_ms == 0U)
  {
    return;
  }
  s_last_poll_ms = now;

  /* HAL_GetTick counts 100 Hz ticks, not milliseconds: divide by the tick
   * rate (board-measured: the old /1000 made the slew 10x slower than the
   * configured 800 us/s). */
  max_step = (APP_GIMBAL_SLEW_US_PER_S * elapsed_ms) / APP_GIMBAL_HAL_TICK_HZ;
  if (max_step == 0U)
  {
    max_step = 1U;
  }

  s_live_pan_us = AppGimbal_Slew(s_live_pan_us, s_target_pan_us, max_step);
  s_live_tilt_us = AppGimbal_Slew(s_live_tilt_us, s_target_tilt_us, max_step);

  if (s_enabled != 0U)
  {
    __HAL_TIM_SET_COMPARE(&s_tim1, TIM_CHANNEL_3, s_live_pan_us);
    __HAL_TIM_SET_COMPARE(&s_tim1, TIM_CHANNEL_4, s_live_tilt_us);
  }
}

void AppGimbal_GetSnapshot(AppGimbalSnapshot_t *snapshot)
{
  uint32_t primask;

  if (snapshot == NULL)
  {
    return;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  snapshot->initialized = s_initialized;
  snapshot->enabled = s_enabled;
  snapshot->power_on = s_power_on;
  snapshot->laser_on = s_laser_on;
  snapshot->relay_on = s_relay_on;
  snapshot->target_theta = s_target_theta;
  snapshot->target_phi = s_target_phi;
  snapshot->pan_pulse_us = s_live_pan_us;
  snapshot->tilt_pulse_us = s_live_tilt_us;
  if (primask == 0U)
  {
    __enable_irq();
  }
}
