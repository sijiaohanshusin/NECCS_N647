#include "app_pcmd_capture.h"

#include <stddef.h>
#include <string.h>

#include "main.h"
#include "app_bringup_thread.h"
#include "app_camera.h"
#include "app_i2c2_bus.h"
#include "PCMD3180/pcmd3180_hal.h"

#define APP_PCMD_CAPTURE_SLOTS_PER_BUS       APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS
#define APP_PCMD_CAPTURE_FRAME_LEN           APP_PCMD_CAPTURE_WIDE32_FRAME_LEN
#define APP_PCMD_CAPTURE_DMA_HALVES          2U
#define APP_PCMD_CAPTURE_DMA_WORDS_PER_HALF  (APP_PCMD_CAPTURE_SLOTS_PER_BUS * APP_PCMD_CAPTURE_FRAME_LEN)
#define APP_PCMD_CAPTURE_DMA_WORDS           (APP_PCMD_CAPTURE_DMA_WORDS_PER_HALF * APP_PCMD_CAPTURE_DMA_HALVES)
#define APP_PCMD_CAPTURE_AUDIO_SCALE         APP_MIC_ARRAY_Q15_TO_FLOAT_SCALE
#define APP_PCMD_CAPTURE_I2C_TIMEOUT_MS      100U
/* Bring-up timing: these are the values validated on this board (P7 run,
 * 149 fps publish). A faster budget (20/5/150/200 ms) was tried on
 * 2026-07-09 and broke TDM output on the second device of each bus while
 * I2C config/status still read back clean - do not shrink these again
 * without re-validating raw slot data on hardware. */
#define APP_PCMD_CAPTURE_RESET_LOW_MS        100U
#define APP_PCMD_CAPTURE_RESET_SETTLE_MS     10U
#define APP_PCMD_CAPTURE_CLOCK_SETTLE_MS     1000U
#define APP_PCMD_CAPTURE_CONFIG_LOCK_MS      3000U
#define APP_PCMD_CAPTURE_RETRY_DELAY_MS      500U
#define APP_PCMD_CAPTURE_CONFIG_ATTEMPTS     3U
#define APP_PCMD_CAPTURE_CONFIG_RETRY_MS     20U
#define APP_PCMD_CAPTURE_POST_CONFIG_SETTLE_MS  1000U
#define APP_PCMD_CAPTURE_POST_CONFIG_DISCARD_HALVES  16U
/* Full-array bring-up attempts before the partial-array failsafe engages. */
#define APP_PCMD_CAPTURE_FULL_ATTEMPTS       3U
#define APP_PCMD_CAPTURE_POST_CONFIG_STATUS_KICK  1U
#define APP_PCMD_CAPTURE_KEEP_SW_I2C_ACTIVE  0U
#define APP_PCMD_CAPTURE_EVENT_HALF0         0x00000001UL
#define APP_PCMD_CAPTURE_EVENT_HALF1         0x00000002UL
#define APP_PCMD_CAPTURE_EVENT_MASK          (APP_PCMD_CAPTURE_EVENT_HALF0 | APP_PCMD_CAPTURE_EVENT_HALF1)
/* Config-window priority (above TouchGFX) vs steady-state priority (the
 * audio rung of the ladder in app_threadx.c). See ThreadEntry. */
#define APP_PCMD_CAPTURE_THREAD_BOOST_PRIORITY 4U
#define APP_PCMD_CAPTURE_THREAD_RUN_PRIORITY   12U
#define APP_PCMD_CAPTURE_RAIL_ABS_LEVEL      32760
#define APP_PCMD_CAPTURE_RAIL_FAULT_X10      1U
#define APP_PCMD_CAPTURE_HIGH_FLOOR_DBFS     (-30)

#ifndef APP_PCMD_SDOUT_BCLK_MARGIN_FIX
#define APP_PCMD_SDOUT_BCLK_MARGIN_FIX       1U
#endif

#ifndef APP_PCMD_CAPTURE_I2C_BACKEND
#define APP_PCMD_CAPTURE_I2C_BACKEND         APP_PCMD_CAPTURE_I2C_BACKEND_SW
#endif

#ifndef APP_PCMD_CAPTURE_STATUS_READ_ON_START
#define APP_PCMD_CAPTURE_STATUS_READ_ON_START 0U
#endif

#ifndef APP_PCMD_CAPTURE_STATUS_POLL_MS
#define APP_PCMD_CAPTURE_STATUS_POLL_MS      0U
#endif

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;

volatile uint32_t g_app_pcmd_debug_ui_enable = APP_PCMD_DIAG_UI_ENABLE;

static int16_t s_bus_a_rx[APP_PCMD_CAPTURE_DMA_WORDS]
    __attribute__((section(".noncacheable"), aligned(32)));
static int16_t s_bus_b_rx[APP_PCMD_CAPTURE_DMA_WORDS]
    __attribute__((section(".noncacheable"), aligned(32)));
static float s_frame_samples[APP_PCMD_CAPTURE_FRAME_RING_COUNT]
                            [APP_MIC_ARRAY_PHYSICAL_MIC_COUNT * APP_PCMD_CAPTURE_FRAME_LEN]
    __attribute__((section(".EXTRAM"), aligned(32)));

static PCMD3180_HAL_BusContextTypeDef s_hal_context;
static PCMD3180_BusTypeDef s_pcmd_bus;
static PCMD3180_HandleTypeDef s_pcmd_handles[APP_PCMD_CAPTURE_DEVICE_COUNT];
static PCMD3180_ConfigTypeDef s_pcmd_configs[APP_PCMD_CAPTURE_DEVICE_COUNT];
static PCMD3180_ArrayModeConfigTypeDef s_mode_config;
static AppAudioFrame_t s_frame_ring[APP_PCMD_CAPTURE_FRAME_RING_COUNT];
static AppPcmdCaptureSnapshot_t s_snapshot;
static TX_EVENT_FLAGS_GROUP s_event_flags;
static uint8_t s_event_flags_created;
static volatile uint8_t s_half_ready_mask[APP_PCMD_CAPTURE_DMA_HALVES];
static volatile uint32_t s_frame_seq;
static volatile uint8_t s_latest_frame_index;
static volatile uint32_t s_raw_slot_abs_sum[APP_PCMD_CAPTURE_BUS_COUNT][APP_PCMD_CAPTURE_SLOTS_PER_BUS];
static volatile int32_t s_raw_slot_dc_sum[APP_PCMD_CAPTURE_BUS_COUNT][APP_PCMD_CAPTURE_SLOTS_PER_BUS];
static volatile uint32_t s_raw_slot_count[APP_PCMD_CAPTURE_BUS_COUNT][APP_PCMD_CAPTURE_SLOTS_PER_BUS];
static volatile int16_t s_raw_slot_last_sample[APP_PCMD_CAPTURE_BUS_COUNT][APP_PCMD_CAPTURE_SLOTS_PER_BUS];
static volatile uint32_t s_raw_rail_sample_count;
static volatile uint32_t s_raw_total_sample_count;
static volatile uint8_t s_raw_accum_enabled;
static volatile uint16_t s_dma_discard_halves_remaining[APP_PCMD_CAPTURE_BUS_COUNT];
/* Failsafe: after several full-array bring-up attempts fail to configure all
 * four devices, accept whatever subset came up and stream from those. A
 * stable partial array (SRP still localises from the healthy channels) beats
 * an endless all-or-nothing reconfig loop when one device is marginal. */
static uint8_t s_pcmd_allow_partial;
/* Set once the devices are configured and verified over I2C. The mic array
 * only needs I2C during configuration - audio then flows over SAI/TDM - so
 * the bus is frozen afterwards and frame stalls recover via DMA-only
 * restart, never by re-touching I2C. */
static uint8_t s_pcmd_config_frozen;

static void AppPcmdCapture_UpdateMicLevels(void);
static void AppPcmdCapture_DeviceStatusKick(uint32_t device_index);
static void AppPcmdCapture_StopDma(void);
static HAL_StatusTypeDef AppPcmdCapture_StartSaiDma(uint16_t discard_halves,
                                                    uint8_t slave_first);

static uint8_t AppPcmdCapture_ClampPercent(uint32_t value)
{
  return (value > 100U) ? 100U : (uint8_t)value;
}

static int8_t AppPcmdCapture_LevelToDbfs(uint16_t level)
{
  typedef struct
  {
    uint16_t level;
    int8_t dbfs;
  } LevelPoint_t;
  static const LevelPoint_t table[] =
  {
    { 32700U,   0 },
    { 26000U,  -2 },
    { 20600U,  -4 },
    { 16384U,  -6 },
    { 10362U, -10 },
    {  6540U, -14 },
    {  4125U, -18 },
    {  3277U, -20 },
    {  2067U, -24 },
    {  1304U, -28 },
    {   823U, -32 },
    {   519U, -36 },
    {   327U, -40 },
    {   206U, -44 },
    {   130U, -48 },
    {    65U, -54 },
    {    33U, -60 },
    {    16U, -66 },
    {     8U, -72 },
    {     4U, -78 },
    {     2U, -84 },
    {     1U, -90 }
  };

  for (uint32_t i = 0U; i < (sizeof(table) / sizeof(table[0])); i++)
  {
    if (level >= table[i].level)
    {
      return table[i].dbfs;
    }
  }

  return -90;
}

static void AppPcmdCapture_ClearRawAccumulator(void)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  memset((void *)s_raw_slot_abs_sum, 0, sizeof(s_raw_slot_abs_sum));
  memset((void *)s_raw_slot_dc_sum, 0, sizeof(s_raw_slot_dc_sum));
  memset((void *)s_raw_slot_count, 0, sizeof(s_raw_slot_count));
  memset((void *)s_raw_slot_last_sample, 0, sizeof(s_raw_slot_last_sample));
  s_raw_rail_sample_count = 0U;
  s_raw_total_sample_count = 0U;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

static uint8_t AppPcmdCapture_IsDeviceStatusExpected(uint32_t device_index,
                                                     const PCMD3180_StatusSnapshotTypeDef *status)
{
  uint8_t expected_pdmclk;
  uint8_t expected_mask;
  uint8_t expected_out_mask;

  if ((status == NULL) || (device_index >= APP_PCMD_CAPTURE_DEVICE_COUNT))
  {
    return 0U;
  }

  expected_mask = s_pcmd_configs[device_index].input_channel_mask;
  expected_out_mask = s_pcmd_configs[device_index].output_channel_mask;
  expected_pdmclk =
      (uint8_t)(PCMD3180_PDMCLK_CFG_RESET_MASK |
                ((uint8_t)s_pcmd_configs[device_index].pdmclk_divider & 0x03U));

  for (uint32_t channel = 0U; channel < PCMD3180_ARRAY_MAX_MICS_PER_DEV; channel++)
  {
    const uint8_t expected_slot =
        (uint8_t)(s_mode_config.devices[device_index].start_slot + channel);
    if (status->asi_ch_slot[channel] != expected_slot)
    {
      return 0U;
    }
  }

  if ((status->sleep_cfg != PCMD3180_SLEEP_CFG_WAKE) ||
      (status->asi_cfg0 != (uint8_t)((((uint8_t)s_pcmd_configs[device_index].slot_width & 0x03U) << 4) |
                                     ((s_pcmd_configs[device_index].invert_fsync == 0U) ? 0U : 0x08U) |
                                     ((s_pcmd_configs[device_index].invert_bclk == 0U) ? 0U : 0x04U) |
                                     0x01U)) ||
      (status->asi_cfg1 != s_pcmd_configs[device_index].tdm_tx_offset) ||
      (status->asi_cfg2 != 0U) ||
      (status->mst_cfg0 != 0U) ||
      (status->mst_cfg1 != 0U) ||
      (status->clk_src != 0U) ||
      (status->pdmclk_cfg != expected_pdmclk) ||
      (status->pdmin_cfg != s_pcmd_configs[device_index].pdmin_edge_mask) ||
      (status->gpo_cfg0 != PCMD3180_GPO_CFG_PDMCLK_OUTPUT) ||
      (status->gpo_cfg1 != PCMD3180_GPO_CFG_PDMCLK_OUTPUT) ||
      (status->gpo_cfg2 != PCMD3180_GPO_CFG_PDMCLK_OUTPUT) ||
      (status->gpo_cfg3 != PCMD3180_GPO_CFG_PDMCLK_OUTPUT) ||
      (status->gpi_cfg0 != PCMD3180_GPI_CFG0_DEFAULT) ||
      (status->gpi_cfg1 != PCMD3180_GPI_CFG1_DEFAULT) ||
      (status->in_ch_en != expected_mask) ||
      (status->asi_out_ch_en != expected_out_mask))
  {
    return 0U;
  }

  if ((status->pwr_cfg & PCMD3180_PWR_PDM_AND_PLL) != PCMD3180_PWR_PDM_AND_PLL)
  {
    return 0U;
  }

  return 1U;
}

static uint8_t AppPcmdCapture_DbfsToPercent(int8_t dbfs)
{
  int32_t value = (int32_t)dbfs;

  if (value <= -60)
  {
    return 0U;
  }
  if (value >= 0)
  {
    return 100U;
  }

  return (uint8_t)(((value + 60) * 100) / 60);
}

static uint64_t AppPcmdCapture_TimestampUs(void)
{
  return ((uint64_t)HAL_GetTick()) * 1000ULL;
}

static ULONG AppPcmdCapture_MsToTicks(uint32_t delay_ms)
{
  uint64_t ticks;

  if (delay_ms == 0U)
  {
    return 0U;
  }

  ticks = ((uint64_t)delay_ms * (uint64_t)TX_TIMER_TICKS_PER_SECOND) + 999ULL;
  ticks /= 1000ULL;
  if (ticks == 0ULL)
  {
    ticks = 1ULL;
  }
  if (ticks > 0xFFFFFFFFULL)
  {
    ticks = 0xFFFFFFFFULL;
  }

  return (ULONG)ticks;
}

static void AppPcmdCapture_DelayMs(uint32_t delay_ms)
{
  ULONG ticks;

  if (delay_ms == 0U)
  {
    return;
  }

  if (tx_thread_identify() == TX_NULL)
  {
    HAL_Delay(delay_ms);
    return;
  }

  ticks = AppPcmdCapture_MsToTicks(delay_ms);
  if (ticks != 0U)
  {
    (void)tx_thread_sleep(ticks);
  }
}

static AppPcmdCaptureStatus_t AppPcmdCapture_EnsureEventGroup(void)
{
  if (s_event_flags_created != 0U)
  {
    return APP_PCMD_CAPTURE_OK;
  }

  if (tx_event_flags_create(&s_event_flags, (CHAR *)"pcmd_capture_events") != TX_SUCCESS)
  {
    return APP_PCMD_CAPTURE_THREADX_ERROR;
  }

  s_event_flags_created = 1U;
  return APP_PCMD_CAPTURE_OK;
}

static PCMD3180_ArrayModeTypeDef AppPcmdCapture_ToPcmdMode(AppMicArrayMode_t mode)
{
  if (mode == APP_MIC_ARRAY_MODE_CORE16_192K)
  {
    return PCMD3180_ARRAY_MODE_CORE16_192K;
  }

  return PCMD3180_ARRAY_MODE_32CH_48K;
}

static uint8_t AppPcmdCapture_IsSupportedMode(AppMicArrayMode_t mode)
{
  return (mode == APP_MIC_ARRAY_MODE_WIDE32_48K) ? 1U : 0U;
}

static void AppPcmdCapture_ClearRuntime(void)
{
  memset(&s_snapshot, 0, sizeof(s_snapshot));
  s_raw_accum_enabled = 0U;
  AppPcmdCapture_ClearRawAccumulator();
  for (uint32_t i = 0U; i < APP_PCMD_CAPTURE_DMA_HALVES; i++)
  {
    s_half_ready_mask[i] = 0U;
  }
  memset(s_frame_ring, 0, sizeof(s_frame_ring));
  s_frame_seq = 0U;
  s_latest_frame_index = 0U;
  for (uint32_t bus = 0U; bus < APP_PCMD_CAPTURE_BUS_COUNT; bus++)
  {
    for (uint32_t slot = 0U; slot < APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS; slot++)
    {
      s_snapshot.slot_dbfs[bus][slot] = -90;
    }
  }
  for (uint32_t mic = 0U; mic < APP_MIC_ARRAY_PHYSICAL_MIC_COUNT; mic++)
  {
    s_snapshot.mic_dbfs[mic] = -90;
  }
  for (uint32_t index = 0U; index < APP_PCMD_CAPTURE_DEVICE_COUNT; index++)
  {
    s_snapshot.device_address_status[index] = (int32_t)PCMD3180_ERROR;
    s_snapshot.device_probe_status[index] = (int32_t)PCMD3180_ERROR;
    s_snapshot.device_config_status[index] = (int32_t)PCMD3180_ERROR;
    s_snapshot.device_status_status[index] = (int32_t)PCMD3180_ERROR;
  }
  s_snapshot.raw_peak_dbfs = -90;
  s_snapshot.raw_avg_dbfs = -90;
}

static void AppPcmdCapture_ClearPendingDmaEvents(void)
{
  ULONG events;
  uint32_t primask;

  primask = __get_PRIMASK();
  __disable_irq();
  s_half_ready_mask[0] = 0U;
  s_half_ready_mask[1] = 0U;
  if (primask == 0U)
  {
    __enable_irq();
  }

  if (s_event_flags_created != 0U)
  {
    while (tx_event_flags_get(&s_event_flags,
                              APP_PCMD_CAPTURE_EVENT_MASK,
                              TX_OR_CLEAR,
                              &events,
                              TX_NO_WAIT) == TX_SUCCESS)
    {
      s_snapshot.stale_event_flush_count++;
    }
  }

  s_snapshot.sai_a_half_count = 0U;
  s_snapshot.sai_a_full_count = 0U;
  s_snapshot.sai_b_half_count = 0U;
  s_snapshot.sai_b_full_count = 0U;
  s_snapshot.dropped_halves = 0U;
  s_snapshot.sync_miss_count = 0U;
  s_snapshot.published_frames = 0U;
  s_snapshot.published_fps_x10 = 0U;
  s_snapshot.latest_seq = 0U;
  s_snapshot.latest_frame_valid = 0U;
  s_snapshot.raw_audio_valid = 0U;
  s_snapshot.raw_quality_flags = 0U;
  s_snapshot.raw_peak_dbfs = -90;
  s_snapshot.raw_avg_dbfs = -90;
  s_snapshot.raw_active_slot_count = 0U;
  s_snapshot.raw_rail_sample_count = 0U;
  s_snapshot.raw_total_sample_count = 0U;
  s_snapshot.raw_rail_percent_x10 = 0U;
  s_frame_seq = 0U;
  s_latest_frame_index = 0U;
  AppPcmdCapture_ClearRawAccumulator();
}

static void AppPcmdCapture_MarkHalfReady(uint8_t bus, uint8_t half)
{
  const uint8_t bus_mask = (uint8_t)(1U << bus);

  if ((half >= APP_PCMD_CAPTURE_DMA_HALVES) || (bus >= APP_PCMD_CAPTURE_BUS_COUNT))
  {
    return;
  }

  if ((s_half_ready_mask[half] & bus_mask) != 0U)
  {
    s_snapshot.dropped_halves++;
  }
  s_half_ready_mask[half] = (uint8_t)(s_half_ready_mask[half] | bus_mask);

  if (s_half_ready_mask[half] == 0x03U)
  {
    if (s_event_flags_created != 0U)
    {
      (void)tx_event_flags_set(&s_event_flags,
                               (half == 0U) ? APP_PCMD_CAPTURE_EVENT_HALF0 : APP_PCMD_CAPTURE_EVENT_HALF1,
                               TX_OR);
    }
  }
}

static uint8_t AppPcmdCapture_TakeHalf(uint8_t half)
{
  uint8_t ready;
  uint32_t primask;

  if (half >= APP_PCMD_CAPTURE_DMA_HALVES)
  {
    return 0U;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  ready = (s_half_ready_mask[half] == 0x03U) ? 1U : 0U;
  if (ready != 0U)
  {
    s_half_ready_mask[half] = 0U;
  }
  if (primask == 0U)
  {
    __enable_irq();
  }

  return ready;
}

static void AppPcmdCapture_AccumulateRawLevels(uint8_t bus,
                                               const int16_t *interleaved,
                                               uint32_t word_count)
{
  int32_t dc_sum[APP_PCMD_CAPTURE_SLOTS_PER_BUS] = { 0 };
  uint32_t ac_sum[APP_PCMD_CAPTURE_SLOTS_PER_BUS] = { 0U };
  uint16_t count[APP_PCMD_CAPTURE_SLOTS_PER_BUS] = { 0U };
  int16_t last_sample[APP_PCMD_CAPTURE_SLOTS_PER_BUS] = { 0 };
  uint32_t rail_count = 0U;

  if ((bus >= APP_PCMD_CAPTURE_BUS_COUNT) || (interleaved == NULL) || (word_count == 0U))
  {
    return;
  }

  for (uint32_t word = 0U; word < word_count; word++)
  {
    const uint32_t slot = word % APP_PCMD_CAPTURE_SLOTS_PER_BUS;
    const int16_t sample = App_MicArray_DecodePcmdTdmSample(interleaved[word]);
    const int32_t sample_abs = (sample < 0) ? -(int32_t)sample : (int32_t)sample;

    dc_sum[slot] += sample;
    count[slot]++;
    last_sample[slot] = sample;
    if (sample_abs >= APP_PCMD_CAPTURE_RAIL_ABS_LEVEL)
    {
      rail_count++;
    }
  }

  for (uint32_t word = 0U; word < word_count; word++)
  {
    const uint32_t slot = word % APP_PCMD_CAPTURE_SLOTS_PER_BUS;
    const int32_t sample = (int32_t)App_MicArray_DecodePcmdTdmSample(interleaved[word]);
    const int32_t mean = (count[slot] == 0U) ? 0 : (dc_sum[slot] / (int32_t)count[slot]);
    const int32_t ac_sample = sample - mean;
    ac_sum[slot] += (uint32_t)((ac_sample < 0) ? -ac_sample : ac_sample);
  }

  for (uint32_t slot = 0U; slot < APP_PCMD_CAPTURE_SLOTS_PER_BUS; slot++)
  {
    if (count[slot] == 0U)
    {
      continue;
    }

    if ((0xFFFFFFFFUL - s_raw_slot_abs_sum[bus][slot]) > ac_sum[slot])
    {
      s_raw_slot_abs_sum[bus][slot] += ac_sum[slot];
    }
    else
    {
      s_raw_slot_abs_sum[bus][slot] = 0xFFFFFFFFUL;
    }

    s_raw_slot_dc_sum[bus][slot] += dc_sum[slot];
    if ((0xFFFFFFFFUL - s_raw_slot_count[bus][slot]) > (uint32_t)count[slot])
    {
      s_raw_slot_count[bus][slot] += (uint32_t)count[slot];
    }
    else
    {
      s_raw_slot_count[bus][slot] = 0xFFFFFFFFUL;
    }
    s_raw_slot_last_sample[bus][slot] = last_sample[slot];
  }

  if ((0xFFFFFFFFUL - s_raw_rail_sample_count) > rail_count)
  {
    s_raw_rail_sample_count += rail_count;
  }
  else
  {
    s_raw_rail_sample_count = 0xFFFFFFFFUL;
  }
  if ((0xFFFFFFFFUL - s_raw_total_sample_count) > word_count)
  {
    s_raw_total_sample_count += word_count;
  }
  else
  {
    s_raw_total_sample_count = 0xFFFFFFFFUL;
  }
}

static void AppPcmdCapture_FlushRawLevels(void)
{
  uint32_t abs_sum[APP_PCMD_CAPTURE_BUS_COUNT][APP_PCMD_CAPTURE_SLOTS_PER_BUS];
  int32_t dc_sum[APP_PCMD_CAPTURE_BUS_COUNT][APP_PCMD_CAPTURE_SLOTS_PER_BUS];
  uint32_t count[APP_PCMD_CAPTURE_BUS_COUNT][APP_PCMD_CAPTURE_SLOTS_PER_BUS];
  int16_t last_sample[APP_PCMD_CAPTURE_BUS_COUNT][APP_PCMD_CAPTURE_SLOTS_PER_BUS];
  uint32_t rail_count;
  uint32_t total_count;
  uint32_t primask;

  primask = __get_PRIMASK();
  __disable_irq();
  for (uint32_t bus = 0U; bus < APP_PCMD_CAPTURE_BUS_COUNT; bus++)
  {
    for (uint32_t slot = 0U; slot < APP_PCMD_CAPTURE_SLOTS_PER_BUS; slot++)
    {
      abs_sum[bus][slot] = s_raw_slot_abs_sum[bus][slot];
      dc_sum[bus][slot] = s_raw_slot_dc_sum[bus][slot];
      count[bus][slot] = s_raw_slot_count[bus][slot];
      last_sample[bus][slot] = s_raw_slot_last_sample[bus][slot];
      s_raw_slot_abs_sum[bus][slot] = 0U;
      s_raw_slot_dc_sum[bus][slot] = 0;
      s_raw_slot_count[bus][slot] = 0U;
    }
  }
  rail_count = s_raw_rail_sample_count;
  total_count = s_raw_total_sample_count;
  s_raw_rail_sample_count = 0U;
  s_raw_total_sample_count = 0U;
  if (primask == 0U)
  {
    __enable_irq();
  }

  if (total_count == 0U)
  {
    return;
  }

  s_snapshot.raw_rail_sample_count = rail_count;
  s_snapshot.raw_total_sample_count = total_count;
  for (uint32_t bus = 0U; bus < APP_PCMD_CAPTURE_BUS_COUNT; bus++)
  {
    for (uint32_t slot = 0U; slot < APP_PCMD_CAPTURE_SLOTS_PER_BUS; slot++)
    {
      uint32_t raw_level = 0U;
      int16_t dc_level = 0;
      int8_t dbfs;

      if (count[bus][slot] != 0U)
      {
        raw_level = abs_sum[bus][slot] / count[bus][slot];
        dc_level = (int16_t)(dc_sum[bus][slot] / (int32_t)count[bus][slot]);
      }
      if (raw_level > 65535U)
      {
        raw_level = 65535U;
      }

      dbfs = AppPcmdCapture_LevelToDbfs((uint16_t)raw_level);
      s_snapshot.slot_level_raw[bus][slot] = (uint16_t)raw_level;
      s_snapshot.slot_dc_level[bus][slot] = dc_level;
      s_snapshot.slot_last_sample[bus][slot] = last_sample[bus][slot];
      s_snapshot.slot_dbfs[bus][slot] = dbfs;
      s_snapshot.slot_level[bus][slot] = AppPcmdCapture_DbfsToPercent(dbfs);
    }
  }

  AppPcmdCapture_UpdateMicLevels();
}

static void AppPcmdCapture_UpdateMicLevels(void)
{
  int32_t dbfs_sum = 0;
  int8_t peak_dbfs = -90;
  uint32_t active_slots = 0U;

  for (uint32_t channel = 0U; channel < APP_MIC_ARRAY_PHYSICAL_MIC_COUNT; channel++)
  {
    AppMicArraySource_t source;
    int8_t dbfs = -90;

    if ((App_MicArray_GetSource(APP_MIC_ARRAY_MODE_WIDE32_48K, channel, &source) == APP_MIC_ARRAY_OK) &&
        (source.bus < APP_PCMD_CAPTURE_BUS_COUNT) &&
        (source.slot < APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS))
    {
      dbfs = s_snapshot.slot_dbfs[source.bus][source.slot];
    }

    s_snapshot.mic_dbfs[channel] = dbfs;
    s_snapshot.mic_level[channel] = AppPcmdCapture_ClampPercent(AppPcmdCapture_DbfsToPercent(dbfs));
    dbfs_sum += dbfs;
    if (dbfs > peak_dbfs)
    {
      peak_dbfs = dbfs;
    }
    if (dbfs > -90)
    {
      active_slots++;
    }
  }

  s_snapshot.raw_peak_dbfs = peak_dbfs;
  s_snapshot.raw_avg_dbfs =
      (int8_t)(dbfs_sum / (int32_t)APP_MIC_ARRAY_PHYSICAL_MIC_COUNT);
  s_snapshot.raw_active_slot_count = (active_slots > 255U) ? 255U : (uint8_t)active_slots;
  s_snapshot.raw_rail_percent_x10 =
      (s_snapshot.raw_total_sample_count == 0U) ? 0U :
      (uint16_t)(((uint64_t)s_snapshot.raw_rail_sample_count * 1000ULL) /
                 (uint64_t)s_snapshot.raw_total_sample_count);
  s_snapshot.raw_quality_flags = 0U;
  /* CONFIG_OK gates publishing (via raw_audio_valid). Normally require all
   * four devices; once the partial-array failsafe is armed, any configured
   * device keeps the healthy channels streaming. */
  if ((s_snapshot.device_config_ok_mask == 0x0FU) ||
      ((s_pcmd_allow_partial != 0U) && (s_snapshot.device_config_ok_mask != 0U)))
  {
    s_snapshot.raw_quality_flags |= APP_PCMD_CAPTURE_RAW_FLAG_CONFIG_OK;
  }
  if ((s_snapshot.sai_a_half_count != 0U) &&
      (s_snapshot.sai_b_half_count != 0U))
  {
    s_snapshot.raw_quality_flags |= APP_PCMD_CAPTURE_RAW_FLAG_DMA_SYNC;
  }
  if (active_slots != 0U)
  {
    s_snapshot.raw_quality_flags |= APP_PCMD_CAPTURE_RAW_FLAG_NONZERO;
  }
  if (peak_dbfs <= -50)
  {
    s_snapshot.raw_quality_flags |= APP_PCMD_CAPTURE_RAW_FLAG_LOW_NOISE;
  }
  if (s_snapshot.raw_rail_percent_x10 >= APP_PCMD_CAPTURE_RAIL_FAULT_X10)
  {
    s_snapshot.raw_quality_flags |= APP_PCMD_CAPTURE_RAW_FLAG_RAIL_FAULT;
  }
  if (s_snapshot.raw_avg_dbfs > APP_PCMD_CAPTURE_HIGH_FLOOR_DBFS)
  {
    s_snapshot.raw_quality_flags |= APP_PCMD_CAPTURE_RAW_FLAG_HIGH_FLOOR;
  }
  /* RAIL_FAULT is a per-frame data-quality indicator, NOT a validity gate.
   * On-board finding (2026-07-09, Rev B): some mic slots sit DC-pinned near
   * full scale (every sample trips the rail counter). Reconfiguring cannot
   * un-rail a stuck mic, and gating validity on it starved the whole
   * pipeline. SRP localises fine from the healthy channels, so publish
   * whenever the pipeline is configured and in sync. */
  s_snapshot.raw_audio_valid =
      ((s_snapshot.raw_quality_flags & (APP_PCMD_CAPTURE_RAW_FLAG_CONFIG_OK |
                                        APP_PCMD_CAPTURE_RAW_FLAG_DMA_SYNC)) ==
       (APP_PCMD_CAPTURE_RAW_FLAG_CONFIG_OK |
        APP_PCMD_CAPTURE_RAW_FLAG_DMA_SYNC)) ? 1U : 0U;
}

static void AppPcmdCapture_UpdateFrameRate(uint32_t now_ms)
{
  static uint32_t last_ms;
  static uint32_t last_published;
  const uint32_t elapsed = now_ms - last_ms;

  if (last_ms == 0U)
  {
    last_ms = now_ms;
    last_published = s_snapshot.published_frames;
    return;
  }

  if (elapsed >= 1000U)
  {
    const uint32_t delta = s_snapshot.published_frames - last_published;
    s_snapshot.published_fps_x10 = (uint16_t)((delta * 10000U) / elapsed);
    last_ms = now_ms;
    last_published = s_snapshot.published_frames;
  }
}

static void AppPcmdCapture_ProcessHalf(uint8_t half)
{
  const uint32_t base = ((uint32_t)half) * APP_PCMD_CAPTURE_DMA_WORDS_PER_HALF;
  const int16_t *bus_a = &s_bus_a_rx[base];
  const int16_t *bus_b = &s_bus_b_rx[base];
  const uint32_t next_seq = s_frame_seq + 1U;
  const uint8_t frame_index = (uint8_t)(next_seq % APP_PCMD_CAPTURE_FRAME_RING_COUNT);
  AppAudioFrame_t *frame = &s_frame_ring[frame_index];
  float *samples = s_frame_samples[frame_index];

  if (App_AudioFrame_FromTdmI16PlanarF32(frame,
                                        APP_MIC_ARRAY_MODE_WIDE32_48K,
                                        bus_a,
                                        bus_b,
                                        APP_PCMD_CAPTURE_FRAME_LEN,
                                        samples,
                                        APP_PCMD_CAPTURE_FRAME_LEN,
                                        next_seq,
                                        AppPcmdCapture_TimestampUs(),
                                        APP_PCMD_CAPTURE_AUDIO_SCALE) != APP_AUDIO_FRAME_OK)
  {
    s_snapshot.sync_miss_count++;
    return;
  }

  frame->drop_count = s_snapshot.dropped_halves;
  frame->error_count = s_snapshot.sai_a_error_count + s_snapshot.sai_b_error_count;
  AppPcmdCapture_UpdateMicLevels();
  if (s_snapshot.raw_audio_valid == 0U)
  {
    s_snapshot.sync_miss_count++;
    s_snapshot.latest_frame_valid = 0U;
    App_BringUpStatus_Fail(APP_BRINGUP_MODULE_AUDIO_FRAME, (int32_t)s_snapshot.raw_quality_flags);
    return;
  }

  s_frame_seq = next_seq;
  s_latest_frame_index = frame_index;
  s_snapshot.latest_seq = next_seq;
  s_snapshot.latest_frame_valid = 1U;
  s_snapshot.published_frames++;
  App_BringUpStatus_Ready(APP_BRINGUP_MODULE_AUDIO_FRAME, 0);
  App_BringUpStatus_Heartbeat(APP_BRINGUP_MODULE_PCMD_RAW, 0);
  AppPcmdCapture_UpdateFrameRate(HAL_GetTick());
}

static void AppPcmdCapture_ReadDeviceStatus(void)
{
  for (uint32_t index = 0U; index < APP_PCMD_CAPTURE_DEVICE_COUNT; index++)
  {
    const uint8_t device_mask = (uint8_t)(1U << index);
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_ReadStatus(&s_pcmd_handles[index], &s_snapshot.device_status[index]);
    s_snapshot.device_status_status[index] = (int32_t)status;
    if ((status == PCMD3180_OK) &&
        (AppPcmdCapture_IsDeviceStatusExpected(index, &s_snapshot.device_status[index]) != 0U))
    {
      s_snapshot.device_status_ok_mask = (uint8_t)(s_snapshot.device_status_ok_mask | device_mask);
    }
    else
    {
      if (status == PCMD3180_OK)
      {
        s_snapshot.device_status_status[index] = (int32_t)PCMD3180_VERIFY_ERROR;
      }
      s_snapshot.device_status_ok_mask = (uint8_t)(s_snapshot.device_status_ok_mask & (uint8_t)~device_mask);
    }
  }
}

static AppPcmdCaptureStatus_t AppPcmdCapture_ConfigPcmdDevices(void)
{
  AppPcmdCaptureStatus_t result = APP_PCMD_CAPTURE_OK;

  s_snapshot.device_present_mask = 0U;
  s_snapshot.device_config_ok_mask = 0U;
  s_snapshot.device_status_ok_mask = 0U;

  for (uint32_t index = 0U; index < APP_PCMD_CAPTURE_DEVICE_COUNT; index++)
  {
    const uint8_t device_mask = (uint8_t)(1U << index);
    PCMD3180_StatusTypeDef address_status;
    PCMD3180_StatusTypeDef probe_status;
    PCMD3180_StatusTypeDef config_status;
    PCMD3180_StatusTypeDef status_status = PCMD3180_ERROR;
    uint8_t configured = 0U;

    s_snapshot.device_address_status[index] = (int32_t)PCMD3180_ERROR;
    s_snapshot.device_probe_status[index] = (int32_t)PCMD3180_ERROR;
    s_snapshot.device_config_status[index] = (int32_t)PCMD3180_ERROR;
    s_snapshot.device_status_status[index] = (int32_t)PCMD3180_ERROR;

    address_status = PCMD3180_HAL_ProbeAddress(&s_hal_context,
                                               s_pcmd_handles[index].address7);
    s_snapshot.device_address_status[index] = (int32_t)address_status;
    if (address_status == PCMD3180_OK)
    {
      s_snapshot.device_present_mask = (uint8_t)(s_snapshot.device_present_mask | (uint8_t)(1U << index));
    }

    /*
     * Keep address/probe reads diagnostic-only, matching the validated PCMD
     * branch. The TI/H7 bring-up sequence starts with writes after clocks are
     * stable; a transient read miss must not skip the real configuration pass.
     */
    probe_status = PCMD3180_Probe(&s_pcmd_handles[index]);
    s_snapshot.device_probe_status[index] = (int32_t)probe_status;
    if (probe_status == PCMD3180_OK)
    {
      s_snapshot.device_present_mask = (uint8_t)(s_snapshot.device_present_mask | (uint8_t)(1U << index));
    }

    for (uint32_t attempt = 0U; attempt < APP_PCMD_CAPTURE_CONFIG_ATTEMPTS; attempt++)
    {
      config_status = PCMD3180_Configure(&s_pcmd_handles[index], &s_pcmd_configs[index]);
      s_snapshot.device_config_status[index] = (int32_t)config_status;

      if (config_status == PCMD3180_OK)
      {
        PCMD3180_StatusSnapshotTypeDef *status = &s_snapshot.device_status[index];

        memset(status, 0, sizeof(*status));
        for (uint32_t channel = 0U; channel < PCMD3180_ARRAY_MAX_MICS_PER_DEV; channel++)
        {
          status->asi_ch_slot[channel] =
              (uint8_t)(s_mode_config.devices[index].start_slot + channel);
        }
        status->pwr_cfg = PCMD3180_PWR_PDM_AND_PLL |
                          (uint8_t)((s_pcmd_configs[index].enable_micbias == 0U) ?
                                    0U : PCMD3180_PWR_MICBIAS);
        status->sleep_cfg = PCMD3180_SLEEP_CFG_WAKE;
        status->asi_cfg0 = (uint8_t)((((uint8_t)s_pcmd_configs[index].slot_width & 0x03U) << 4) |
                                     ((s_pcmd_configs[index].invert_fsync == 0U) ? 0U : 0x08U) |
                                     ((s_pcmd_configs[index].invert_bclk == 0U) ? 0U : 0x04U) |
                                     0x01U);
        status->asi_cfg1 = s_pcmd_configs[index].tdm_tx_offset;
        status->asi_cfg2 = 0U;
        status->mst_cfg0 = 0U;
        status->mst_cfg1 = 0U;
        status->clk_src = 0U;
        status->pdmclk_cfg = (uint8_t)(PCMD3180_PDMCLK_CFG_RESET_MASK |
                                       ((uint8_t)s_pcmd_configs[index].pdmclk_divider & 0x03U));
        status->pdmin_cfg = s_pcmd_configs[index].pdmin_edge_mask;
        status->gpo_cfg0 = PCMD3180_GPO_CFG_PDMCLK_OUTPUT;
        status->gpo_cfg1 = PCMD3180_GPO_CFG_PDMCLK_OUTPUT;
        status->gpo_cfg2 = PCMD3180_GPO_CFG_PDMCLK_OUTPUT;
        status->gpo_cfg3 = PCMD3180_GPO_CFG_PDMCLK_OUTPUT;
        status->gpi_cfg0 = PCMD3180_GPI_CFG0_DEFAULT;
        status->gpi_cfg1 = PCMD3180_GPI_CFG1_DEFAULT;
        status->in_ch_en = s_pcmd_configs[index].input_channel_mask;
        status->asi_out_ch_en = s_pcmd_configs[index].output_channel_mask;

        s_snapshot.device_present_mask = (uint8_t)(s_snapshot.device_present_mask | device_mask);
        s_snapshot.device_config_ok_mask = (uint8_t)(s_snapshot.device_config_ok_mask | device_mask);
        s_snapshot.device_status_ok_mask = (uint8_t)(s_snapshot.device_status_ok_mask | device_mask);
#if (APP_PCMD_CAPTURE_STATUS_READ_ON_START != 0U)
        status_status = PCMD3180_ReadStatus(&s_pcmd_handles[index],
                                            &s_snapshot.device_status[index]);
        if ((status_status == PCMD3180_OK) &&
            (AppPcmdCapture_IsDeviceStatusExpected(index, &s_snapshot.device_status[index]) == 0U))
        {
          status_status = PCMD3180_VERIFY_ERROR;
        }
        if (status_status != PCMD3180_OK)
        {
          s_snapshot.device_status_ok_mask = (uint8_t)(s_snapshot.device_status_ok_mask & (uint8_t)~device_mask);
        }
#else
        status_status = PCMD3180_OK;
#endif
        configured = 1U;
        break;
      }
      else
      {
        status_status = config_status;
      }

      if ((attempt + 1U) < APP_PCMD_CAPTURE_CONFIG_ATTEMPTS)
      {
        AppPcmdCapture_DelayMs(APP_PCMD_CAPTURE_CONFIG_RETRY_MS);
      }
    }

    s_snapshot.device_status_status[index] = (int32_t)status_status;
    if (configured == 0U)
    {
      if (s_snapshot.device_config_status[index] == (int32_t)PCMD3180_OK)
      {
        s_snapshot.device_config_status[index] = (int32_t)PCMD3180_VERIFY_ERROR;
      }
      s_snapshot.device_config_ok_mask = (uint8_t)(s_snapshot.device_config_ok_mask & (uint8_t)~device_mask);
      s_snapshot.device_status_ok_mask = (uint8_t)(s_snapshot.device_status_ok_mask & (uint8_t)~device_mask);
      result = APP_PCMD_CAPTURE_PCMD_ERROR;
    }
  }

  if (result == APP_PCMD_CAPTURE_OK)
  {
    static const uint8_t activate_order[APP_PCMD_CAPTURE_DEVICE_COUNT] =
    {
      1U, 0U, 3U, 2U
    };

    for (uint32_t order = 0U; order < APP_PCMD_CAPTURE_DEVICE_COUNT; order++)
    {
      const uint32_t index = activate_order[order];
      const uint8_t device_mask = (uint8_t)(1U << index);
      PCMD3180_StatusTypeDef activate_status =
          PCMD3180_Activate(&s_pcmd_handles[index], &s_pcmd_configs[index]);

      if (activate_status != PCMD3180_OK)
      {
        s_snapshot.device_config_status[index] = (int32_t)activate_status;
        s_snapshot.device_config_ok_mask = (uint8_t)(s_snapshot.device_config_ok_mask & (uint8_t)~device_mask);
        s_snapshot.device_status_ok_mask = (uint8_t)(s_snapshot.device_status_ok_mask & (uint8_t)~device_mask);
        result = APP_PCMD_CAPTURE_PCMD_ERROR;
      }
      else
      {
        AppPcmdCapture_DeviceStatusKick(index);
      }
    }
  }

  return result;
}

static void AppPcmdCapture_DeviceStatusKick(uint32_t device_index)
{
#if (APP_PCMD_CAPTURE_POST_CONFIG_STATUS_KICK != 0U)
  PCMD3180_StatusSnapshotTypeDef scratch;

  if (device_index < APP_PCMD_CAPTURE_DEVICE_COUNT)
  {
    memset(&scratch, 0, sizeof(scratch));
    (void)PCMD3180_ReadStatus(&s_pcmd_handles[device_index], &scratch);
  }
#else
  (void)device_index;
#endif
}

static void AppPcmdCapture_SelectI2CBackend(uint8_t backend)
{
  const uint8_t use_software =
      (backend == APP_PCMD_CAPTURE_I2C_BACKEND_SW) ? 1U : 0U;

  PCMD3180_HAL_SetSoftwareI2CEnabled(&s_hal_context, use_software);
  s_snapshot.i2c_backend_active = use_software ?
      APP_PCMD_CAPTURE_I2C_BACKEND_SW :
      APP_PCMD_CAPTURE_I2C_BACKEND_HAL;
}

static AppPcmdCaptureStatus_t AppPcmdCapture_ResetAndConfigurePcmd(uint8_t backend)
{
  PCMD3180_StatusTypeDef pcmd_status;
  AppPcmdCaptureStatus_t config_status;
  const uint8_t use_software =
      (backend == APP_PCMD_CAPTURE_I2C_BACKEND_SW) ? 1U : 0U;

  AppPcmdCapture_SelectI2CBackend(backend);
  if (use_software != 0U)
  {
    PCMD3180_HAL_PrepareSoftwareI2C(&s_hal_context);
  }

  pcmd_status = PCMD3180_HardwareReset(&s_pcmd_handles[0],
                                       APP_PCMD_CAPTURE_RESET_LOW_MS,
                                       APP_PCMD_CAPTURE_RESET_SETTLE_MS);
  if (pcmd_status != PCMD3180_OK)
  {
    s_snapshot.start_status = (int32_t)pcmd_status;
    if ((use_software != 0U) && (APP_PCMD_CAPTURE_KEEP_SW_I2C_ACTIVE == 0U))
    {
      PCMD3180_HAL_ReleaseSoftwareI2C(&s_hal_context);
    }
    return APP_PCMD_CAPTURE_PCMD_ERROR;
  }

  config_status = AppPcmdCapture_ConfigPcmdDevices();
  if ((use_software != 0U) && (APP_PCMD_CAPTURE_KEEP_SW_I2C_ACTIVE == 0U))
  {
    PCMD3180_HAL_ReleaseSoftwareI2C(&s_hal_context);
  }

  return config_status;
}

static void AppPcmdCapture_RecordSaiErrors(void)
{
  s_snapshot.sai_a_last_error = HAL_SAI_GetError(&hsai_BlockA1);
  s_snapshot.sai_b_last_error = HAL_SAI_GetError(&hsai_BlockB1);
  s_snapshot.dma_a_error = HAL_DMA_GetError(&handle_GPDMA1_Channel0);
  s_snapshot.dma_b_error = HAL_DMA_GetError(&handle_GPDMA1_Channel1);
}

static void AppPcmdCapture_RequestRestart(uint32_t reason)
{
  s_snapshot.start_status = (int32_t)reason;
  s_snapshot.raw_audio_valid = 0U;
  s_snapshot.latest_frame_valid = 0U;
  AppPcmdCapture_StopDma();
  AppPcmdCapture_ClearPendingDmaEvents();
  s_snapshot.started = 0U;
  App_BringUpStatus_Fail(APP_BRINGUP_MODULE_PCMD_RAW, (int32_t)reason);
  App_BringUpStatus_Fail(APP_BRINGUP_MODULE_AUDIO_FRAME, (int32_t)reason);
}

/* Recover a frame stall without touching I2C: the PCMD3180 devices keep
 * their verified (frozen) register configuration, so only the SAI/DMA
 * capture path is torn down and restarted. `started` stays latched, so the
 * pipeline never falls back into the I2C config path. Returns 1 on
 * success. */
static uint8_t AppPcmdCapture_RestartDmaOnly(void)
{
  HAL_StatusTypeDef hal_status;

  s_raw_accum_enabled = 0U;
  AppPcmdCapture_StopDma();
  AppPcmdCapture_ClearPendingDmaEvents();

  hal_status = AppPcmdCapture_StartSaiDma(APP_PCMD_CAPTURE_POST_CONFIG_DISCARD_HALVES, 1U);
  if (hal_status != HAL_OK)
  {
    s_snapshot.start_status = APP_PCMD_CAPTURE_HAL_ERROR;
    AppPcmdCapture_StopDma();
    return 0U;
  }

  s_raw_accum_enabled = 1U;
  s_snapshot.started = 1U;
  s_snapshot.start_status = APP_PCMD_CAPTURE_OK;
  return 1U;
}

static void AppPcmdCapture_ArmDiscardHalves(uint16_t discard_halves)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  s_dma_discard_halves_remaining[APP_MIC_ARRAY_BUS_A] = discard_halves;
  s_dma_discard_halves_remaining[APP_MIC_ARRAY_BUS_B] = discard_halves;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

static HAL_StatusTypeDef AppPcmdCapture_StartSaiBlockB(void)
{
  HAL_StatusTypeDef hal_status;

  HAL_NVIC_DisableIRQ(SAI1_B_IRQn);
  __HAL_SAI_CLEAR_FLAG(&hsai_BlockB1, SAI_FLAG_AFSDET | SAI_FLAG_LFSDET | SAI_FLAG_OVRUDR);
  __HAL_SAI_DISABLE_IT(&hsai_BlockB1, SAI_IT_AFSDET | SAI_IT_LFSDET);
  hal_status = HAL_SAI_Receive_DMA(&hsai_BlockB1,
                                   (uint8_t *)s_bus_b_rx,
                                   APP_PCMD_CAPTURE_DMA_WORDS);
  __HAL_SAI_CLEAR_FLAG(&hsai_BlockB1, SAI_FLAG_AFSDET | SAI_FLAG_LFSDET | SAI_FLAG_OVRUDR);
  __HAL_SAI_DISABLE_IT(&hsai_BlockB1, SAI_IT_AFSDET | SAI_IT_LFSDET);
  HAL_NVIC_EnableIRQ(SAI1_B_IRQn);

  s_snapshot.dma_b_status = (int32_t)hal_status;
  if (hal_status != HAL_OK)
  {
    AppPcmdCapture_RecordSaiErrors();
    return hal_status;
  }

  __HAL_SAI_CLEAR_FLAG(&hsai_BlockB1, SAI_FLAG_AFSDET | SAI_FLAG_LFSDET | SAI_FLAG_OVRUDR);
  __HAL_SAI_DISABLE_IT(&hsai_BlockB1, SAI_IT_AFSDET | SAI_IT_LFSDET);
  return HAL_OK;
}

static HAL_StatusTypeDef AppPcmdCapture_StartSaiBlockA(void)
{
  HAL_StatusTypeDef hal_status;

  hal_status = HAL_SAI_Receive_DMA(&hsai_BlockA1,
                                   (uint8_t *)s_bus_a_rx,
                                   APP_PCMD_CAPTURE_DMA_WORDS);
  s_snapshot.dma_a_status = (int32_t)hal_status;
  if (hal_status != HAL_OK)
  {
    AppPcmdCapture_RecordSaiErrors();
  }

  return hal_status;
}

static HAL_StatusTypeDef AppPcmdCapture_StartSaiDma(uint16_t discard_halves,
                                                    uint8_t slave_first)
{
  HAL_StatusTypeDef hal_status;

  memset(s_bus_a_rx, 0, sizeof(s_bus_a_rx));
  memset(s_bus_b_rx, 0, sizeof(s_bus_b_rx));

  AppPcmdCapture_ArmDiscardHalves(discard_halves);

  if (slave_first != 0U)
  {
    hal_status = AppPcmdCapture_StartSaiBlockB();
    if (hal_status != HAL_OK)
    {
      return hal_status;
    }
    AppPcmdCapture_DelayMs(2U);
    return AppPcmdCapture_StartSaiBlockA();
  }

  hal_status = AppPcmdCapture_StartSaiBlockA();
  if (hal_status != HAL_OK)
  {
    return hal_status;
  }
  AppPcmdCapture_DelayMs(2U);
  return AppPcmdCapture_StartSaiBlockB();
}

static void AppPcmdCapture_StopDma(void)
{
  s_raw_accum_enabled = 0U;
  s_dma_discard_halves_remaining[APP_MIC_ARRAY_BUS_A] = 0U;
  s_dma_discard_halves_remaining[APP_MIC_ARRAY_BUS_B] = 0U;
  (void)HAL_SAI_DMAStop(&hsai_BlockA1);
  (void)HAL_SAI_DMAStop(&hsai_BlockB1);

  s_half_ready_mask[0] = 0U;
  s_half_ready_mask[1] = 0U;
  s_snapshot.latest_frame_valid = 0U;
  s_snapshot.started = 0U;
  AppPcmdCapture_RecordSaiErrors();
}

AppPcmdCaptureStatus_t AppPcmdCapture_Init(AppMicArrayMode_t mode)
{
  AppPcmdCaptureStatus_t event_status;
  PCMD3180_StatusTypeDef pcmd_status;

  if (AppPcmdCapture_IsSupportedMode(mode) == 0U)
  {
    return APP_PCMD_CAPTURE_UNSUPPORTED_MODE;
  }

  event_status = AppPcmdCapture_EnsureEventGroup();
  if (event_status != APP_PCMD_CAPTURE_OK)
  {
    return event_status;
  }

  AppPcmdCapture_ClearRuntime();
  s_snapshot.mode = mode;
  s_snapshot.debug_ui_enabled = ((g_app_pcmd_debug_ui_enable != 0U) ||
                                 (APP_PCMD_DIAG_UI_ENABLE != 0U)) ? 1U : 0U;

  s_hal_context.hi2c = NULL;
  /* Dedicated software-I2C pins for the mic array (rewired 2026-07-09):
   * PC10=SCL, PC11=SDA, spare expansion GPIO with secure attributes already
   * set in MX_GPIO_Init. hi2c stays NULL so the driver never de-inits,
   * locks, or recovers the camera's hardware I2C2 (PD14/PD4) - the two
   * buses are now electrically and logically independent, which removes
   * the AF<->GPIO pin flipping and bus contention that made bring-up
   * probabilistic. */
  s_hal_context.scl_port = GPIOC;
  s_hal_context.scl_pin = GPIO_PIN_10;
  s_hal_context.sda_port = GPIOC;
  s_hal_context.sda_pin = GPIO_PIN_11;
  s_hal_context.shutdown_port = MIC_SHDNZ_GPIO_Port;
  s_hal_context.shutdown_pin = MIC_SHDNZ_Pin;
  s_hal_context.timeout_ms = APP_PCMD_CAPTURE_I2C_TIMEOUT_MS;
  PCMD3180_HAL_BusInit(&s_pcmd_bus, &s_hal_context);
  s_snapshot.i2c_backend_requested = APP_PCMD_CAPTURE_I2C_BACKEND;
  s_snapshot.i2c_backend_active = APP_PCMD_CAPTURE_I2C_BACKEND_HAL;

  pcmd_status = PCMD3180_GetArrayModeConfig(AppPcmdCapture_ToPcmdMode(mode), &s_mode_config);
  if (pcmd_status != PCMD3180_OK)
  {
    s_snapshot.init_status = (int32_t)pcmd_status;
    return APP_PCMD_CAPTURE_PCMD_ERROR;
  }

  s_snapshot.sample_rate_hz = s_mode_config.sample_rate_hz;
  s_snapshot.expected_bclk_hz = s_mode_config.expected_bclk_hz;
  s_snapshot.frame_len = s_mode_config.frame_samples;
  s_snapshot.slots_per_bus = s_mode_config.tdm_slots_per_bus;

  for (uint32_t index = 0U; index < APP_PCMD_CAPTURE_DEVICE_COUNT; index++)
  {
    pcmd_status = PCMD3180_Init(&s_pcmd_handles[index],
                                &s_pcmd_bus,
                                s_mode_config.devices[index].address7);
    if (pcmd_status != PCMD3180_OK)
    {
      s_snapshot.init_status = (int32_t)pcmd_status;
      return APP_PCMD_CAPTURE_PCMD_ERROR;
    }

    pcmd_status = PCMD3180_BuildDeviceConfig(&s_mode_config, index, &s_pcmd_configs[index]);
    if (pcmd_status != PCMD3180_OK)
    {
      s_snapshot.init_status = (int32_t)pcmd_status;
      return APP_PCMD_CAPTURE_PCMD_ERROR;
    }
#if (APP_PCMD_SDOUT_BCLK_MARGIN_FIX != 0U)
    s_pcmd_configs[index].invert_bclk = 1U;
#endif
    s_pcmd_configs[index].defer_power_up = 1U;
  }

  s_snapshot.initialized = 1U;
  s_snapshot.init_status = APP_PCMD_CAPTURE_OK;
  return APP_PCMD_CAPTURE_OK;
}

AppPcmdCaptureStatus_t AppPcmdCapture_Start(void)
{
  HAL_StatusTypeDef hal_status;
  AppPcmdCaptureStatus_t config_status;

  if (s_snapshot.initialized == 0U)
  {
    AppPcmdCaptureStatus_t init_status = AppPcmdCapture_Init(APP_MIC_ARRAY_MODE_WIDE32_48K);
    if (init_status != APP_PCMD_CAPTURE_OK)
    {
      return init_status;
    }
  }

  if (s_snapshot.started != 0U)
  {
    return APP_PCMD_CAPTURE_OK;
  }

  AppPcmdCapture_StopDma();
  AppPcmdCapture_ClearPendingDmaEvents();

  hal_status = AppPcmdCapture_StartSaiDma(0U, 0U);
  if (hal_status != HAL_OK)
  {
    s_snapshot.start_status = APP_PCMD_CAPTURE_HAL_ERROR;
    AppPcmdCapture_StopDma();
    AppPcmdCapture_RecordSaiErrors();
    return APP_PCMD_CAPTURE_HAL_ERROR;
  }

  __HAL_SAI_CLEAR_FLAG(&hsai_BlockB1, SAI_FLAG_AFSDET | SAI_FLAG_LFSDET | SAI_FLAG_OVRUDR);
  __HAL_SAI_DISABLE_IT(&hsai_BlockB1, SAI_IT_AFSDET | SAI_IT_LFSDET);
  AppPcmdCapture_DelayMs(APP_PCMD_CAPTURE_CLOCK_SETTLE_MS);

  /* Hold the I2C2 lock for the whole config window even though the mic bus
   * now bit-bangs dedicated pins (PC10/PC11): the failure signature is
   * identical to the PD4/PD14 era and scales with window duration, i.e.
   * concurrent camera-exposure / power-management I2C2 traffic disturbs the
   * mic net during configuration. Serialising the window is the combination
   * that was board-verified stable (2026-07-09 08:41, all 4 devices, 5400+
   * frames). One-time boot cost only - config freezes afterwards. */
  if (AppI2C2_Lock(APP_PCMD_CAPTURE_CONFIG_LOCK_MS) == 0U)
  {
    s_snapshot.start_status = APP_PCMD_CAPTURE_BUSY;
    AppPcmdCapture_StopDma();
    return APP_PCMD_CAPTURE_BUSY;
  }

  config_status = AppPcmdCapture_ResetAndConfigurePcmd(APP_PCMD_CAPTURE_I2C_BACKEND_SW);

  AppI2C2_Unlock();

  /* Accept the pass when every device configured, or - once the failsafe is
   * armed after repeated full-array misses - when at least one device did.
   * Only a completely empty bus is a hard failure worth another
   * hardware-reset retry. */
  if (config_status != APP_PCMD_CAPTURE_OK)
  {
    const uint8_t partial_ok = ((s_pcmd_allow_partial != 0U) &&
                                (s_snapshot.device_config_ok_mask != 0U)) ? 1U : 0U;

    if (partial_ok == 0U)
    {
      s_snapshot.start_status = config_status;
      AppPcmdCapture_StopDma();
      App_BringUpStatus_Fail(APP_BRINGUP_MODULE_PCMD_RAW, (int32_t)config_status);
      return config_status;
    }
  }

  AppPcmdCapture_DelayMs(APP_PCMD_CAPTURE_POST_CONFIG_SETTLE_MS);
  AppPcmdCapture_StopDma();
  AppPcmdCapture_ClearPendingDmaEvents();
  hal_status = AppPcmdCapture_StartSaiDma(APP_PCMD_CAPTURE_POST_CONFIG_DISCARD_HALVES, 1U);
  if (hal_status != HAL_OK)
  {
    s_snapshot.start_status = APP_PCMD_CAPTURE_HAL_ERROR;
    AppPcmdCapture_StopDma();
    return APP_PCMD_CAPTURE_HAL_ERROR;
  }

  s_raw_accum_enabled = 1U;
  s_snapshot.started = 1U;
  s_snapshot.start_status = APP_PCMD_CAPTURE_OK;
  /* One-time I2C configuration succeeded: freeze the bus. The devices keep
   * their registers and audio flows over SAI, so I2C is never touched again
   * (frame stalls recover via DMA-only restart). */
  s_pcmd_config_frozen = 1U;
  App_BringUpStatus_Ready(APP_BRINGUP_MODULE_PCMD_RAW, APP_PCMD_CAPTURE_OK);
  return APP_PCMD_CAPTURE_OK;
}

AppPcmdCaptureStatus_t AppPcmdCapture_Poll(ULONG wait_ticks)
{
  static uint32_t last_error_status_ms;
  static uint32_t last_device_status_ms;
  uint8_t diag_enabled;
  uint32_t now_ms;
  ULONG events = 0U;
  UINT tx_status;

  if (s_event_flags_created == 0U)
  {
    return APP_PCMD_CAPTURE_THREADX_ERROR;
  }

  tx_status = tx_event_flags_get(&s_event_flags,
                                 APP_PCMD_CAPTURE_EVENT_MASK,
                                 TX_OR_CLEAR,
                                 &events,
                                 wait_ticks);
  if ((tx_status != TX_SUCCESS) && (tx_status != TX_NO_EVENTS))
  {
    return APP_PCMD_CAPTURE_THREADX_ERROR;
  }

  AppPcmdCapture_FlushRawLevels();

  if (((events & APP_PCMD_CAPTURE_EVENT_HALF0) != 0U) &&
      (AppPcmdCapture_TakeHalf(0U) != 0U))
  {
    AppPcmdCapture_ProcessHalf(0U);
  }

  if (((events & APP_PCMD_CAPTURE_EVENT_HALF1) != 0U) &&
      (AppPcmdCapture_TakeHalf(1U) != 0U))
  {
    AppPcmdCapture_ProcessHalf(1U);
  }

  diag_enabled = ((g_app_pcmd_debug_ui_enable != 0U) ||
                  (APP_PCMD_DIAG_UI_ENABLE != 0U)) ? 1U : 0U;
  s_snapshot.debug_ui_enabled = diag_enabled;
  now_ms = HAL_GetTick();

  if ((s_snapshot.started != 0U) && ((now_ms - last_error_status_ms) >= 1000U))
  {
    last_error_status_ms = now_ms;
    AppPcmdCapture_RecordSaiErrors();
  }

  if ((s_snapshot.started != 0U) &&
      (diag_enabled != 0U) &&
      (APP_PCMD_CAPTURE_STATUS_POLL_MS != 0U) &&
      ((now_ms - last_device_status_ms) >= APP_PCMD_CAPTURE_STATUS_POLL_MS))
  {
    last_device_status_ms = now_ms;
    AppPcmdCapture_ReadDeviceStatus();
  }

  return APP_PCMD_CAPTURE_OK;
}

void AppPcmdCapture_ThreadEntry(ULONG thread_input)
{
  /* The thread is created at a boosted priority (above TouchGFX): during the
   * boot screen the UI renders continuously and a priority-12 thread does
   * not get scheduled at all (observed run_count=0 after 8 s), so the boost
   * cannot be done from inside this thread after creation. Once the array
   * streams, drop to the steady-state priority so the render/DSP ladder in
   * app_threadx.c applies; boost again only for restart config windows. */
  TX_THREAD *self = tx_thread_identify();
  UINT discard_priority;
  uint8_t boosted = 1U;

  (void)thread_input;

  /* Camera-first gate, capped at 6 s: PCMD config holds the I2C2 lock for
   * seconds at a time (the soft-I2C pins are separate, but the lock is
   * shared); letting the camera stream first keeps its bring-up off the
   * contended window. The cap keeps a broken camera from blocking audio
   * forever (the old uncapped wait was 20 s+). */
  for (uint32_t i = 0U; i < 60U; i++)
  {
    AppBringUpSnapshot_t bringup;
    const uint32_t camera_bit = 1UL << APP_BRINGUP_MODULE_CAMERA;

    App_BringUpStatus_GetSnapshot(&bringup);
    if (((bringup.enabled_mask & camera_bit) == 0U) ||
        (((bringup.failed_mask | bringup.skipped_mask) & camera_bit) != 0U) ||
        (g_app_camera_frame_count >= 3U))
    {
      break;
    }
    tx_thread_sleep(TX_TIMER_TICKS_PER_SECOND / 10U);
  }

  uint32_t last_seq = 0U;
  uint32_t stall_ticks = 0U;
  uint8_t start_attempts = 0U;

  while (1)
  {
    if (s_snapshot.initialized == 0U)
    {
      (void)AppPcmdCapture_Init(APP_MIC_ARRAY_MODE_WIDE32_48K);
    }

    if (s_snapshot.started == 0U)
    {
      if ((boosted == 0U) && (self != TX_NULL))
      {
        (void)tx_thread_priority_change(self,
                                        APP_PCMD_CAPTURE_THREAD_BOOST_PRIORITY,
                                        &discard_priority);
        boosted = 1U;
      }
      (void)AppPcmdCapture_Start();
      if (s_snapshot.started == 0U)
      {
        /* Count consecutive full-array misses; past the threshold, arm the
         * partial-array failsafe so the next pass latches with whatever
         * devices came up instead of looping all-or-nothing. */
        if (start_attempts < 0xFFU)
        {
          start_attempts++;
        }
        if ((s_pcmd_allow_partial == 0U) &&
            (start_attempts >= APP_PCMD_CAPTURE_FULL_ATTEMPTS))
        {
          s_pcmd_allow_partial = 1U;
        }
        s_snapshot.recovering = 1U;
        AppPcmdCapture_DelayMs(APP_PCMD_CAPTURE_RETRY_DELAY_MS);
        continue;
      }
      start_attempts = 0U;
      s_snapshot.recovering = 0U;
      last_seq = s_snapshot.latest_seq;
      stall_ticks = 0U;
    }

    if ((boosted != 0U) && (self != TX_NULL))
    {
      (void)tx_thread_priority_change(self,
                                      APP_PCMD_CAPTURE_THREAD_RUN_PRIORITY,
                                      &discard_priority);
      boosted = 0U;
    }

    (void)AppPcmdCapture_Poll(TX_TIMER_TICKS_PER_SECOND / 10U);

    /* Frame-stall watchdog: SAI/DMA can wedge silently (e.g. cable brownout
     * on the mic array) - the loop keeps polling but latest_seq freezes.
     * ~2 s without a new frame triggers recovery. Once the I2C config is
     * frozen, recovery restarts only the SAI/DMA (never re-touches the
     * bus); rail faults alone never restart anything - stuck mics are not
     * fixable by reconfig and the healthy channels keep streaming. */
    if (s_snapshot.started != 0U)
    {
      if (s_snapshot.latest_seq != last_seq)
      {
        last_seq = s_snapshot.latest_seq;
        stall_ticks = 0U;
      }
      else if (++stall_ticks >= 20U)
      {
        stall_ticks = 0U;
        s_snapshot.watchdog_restart_count++;
        if (s_pcmd_config_frozen != 0U)
        {
          if (AppPcmdCapture_RestartDmaOnly() == 0U)
          {
            s_snapshot.recovering = 1U;
            AppPcmdCapture_DelayMs(APP_PCMD_CAPTURE_RETRY_DELAY_MS);
          }
          else
          {
            s_snapshot.recovering = 0U;
          }
          last_seq = s_snapshot.latest_seq;
        }
        else
        {
          s_snapshot.recovering = 1U;
          AppPcmdCapture_RequestRestart((uint32_t)APP_PCMD_CAPTURE_PCMD_ERROR);
        }
      }
    }
  }
}

void AppPcmdCapture_GetSnapshot(AppPcmdCaptureSnapshot_t *snapshot)
{
  uint32_t primask;

  if (snapshot == NULL)
  {
    return;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  *snapshot = s_snapshot;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

uint8_t AppPcmdCapture_GetLatestAudioFrame(AppAudioFrame_t *frame)
{
  uint8_t index;
  uint8_t valid;
  uint32_t primask;

  if (frame == NULL)
  {
    return 0U;
  }

  primask = __get_PRIMASK();
  __disable_irq();
  valid = s_snapshot.latest_frame_valid;
  index = s_latest_frame_index;
  if (valid != 0U)
  {
    *frame = s_frame_ring[index];
  }
  if (primask == 0U)
  {
    __enable_irq();
  }

  return valid;
}

static void AppPcmdCapture_OnSaiHalfReady(uint8_t bus,
                                          uint8_t half,
                                          const int16_t *samples)
{
  if ((bus >= APP_PCMD_CAPTURE_BUS_COUNT) ||
      (half >= APP_PCMD_CAPTURE_DMA_HALVES) ||
      (samples == NULL))
  {
    return;
  }

  if (s_dma_discard_halves_remaining[bus] != 0U)
  {
    s_dma_discard_halves_remaining[bus]--;
    return;
  }

  if (s_raw_accum_enabled != 0U)
  {
    AppPcmdCapture_AccumulateRawLevels(bus,
                                       samples,
                                       APP_PCMD_CAPTURE_DMA_WORDS_PER_HALF);
  }
  AppPcmdCapture_MarkHalfReady(bus, half);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    s_snapshot.sai_a_half_count++;
    AppPcmdCapture_OnSaiHalfReady(APP_MIC_ARRAY_BUS_A, 0U, s_bus_a_rx);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    s_snapshot.sai_b_half_count++;
    AppPcmdCapture_OnSaiHalfReady(APP_MIC_ARRAY_BUS_B, 0U, s_bus_b_rx);
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    s_snapshot.sai_a_full_count++;
    AppPcmdCapture_OnSaiHalfReady(APP_MIC_ARRAY_BUS_A,
                                  1U,
                                  &s_bus_a_rx[APP_PCMD_CAPTURE_DMA_WORDS_PER_HALF]);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    s_snapshot.sai_b_full_count++;
    AppPcmdCapture_OnSaiHalfReady(APP_MIC_ARRAY_BUS_B,
                                  1U,
                                  &s_bus_b_rx[APP_PCMD_CAPTURE_DMA_WORDS_PER_HALF]);
  }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    s_snapshot.sai_a_error_count++;
    s_snapshot.sai_a_last_error = HAL_SAI_GetError(hsai);
    s_snapshot.dma_a_error = HAL_DMA_GetError(&handle_GPDMA1_Channel0);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    s_snapshot.sai_b_error_count++;
    s_snapshot.sai_b_last_error = HAL_SAI_GetError(hsai);
    s_snapshot.dma_b_error = HAL_DMA_GetError(&handle_GPDMA1_Channel1);
  }
}
