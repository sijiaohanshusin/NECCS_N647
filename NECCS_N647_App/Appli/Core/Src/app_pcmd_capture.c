#include "app_pcmd_capture.h"

#include <stddef.h>
#include <string.h>

#include "main.h"
#include "app_bringup_thread.h"
#include "PCMD3180/pcmd3180_hal.h"

#define APP_PCMD_CAPTURE_SLOTS_PER_BUS       APP_MIC_ARRAY_WIDE32_SLOTS_PER_BUS
#define APP_PCMD_CAPTURE_FRAME_LEN           APP_PCMD_CAPTURE_WIDE32_FRAME_LEN
#define APP_PCMD_CAPTURE_DMA_HALVES          2U
#define APP_PCMD_CAPTURE_DMA_WORDS_PER_HALF  (APP_PCMD_CAPTURE_SLOTS_PER_BUS * APP_PCMD_CAPTURE_FRAME_LEN)
#define APP_PCMD_CAPTURE_DMA_WORDS           (APP_PCMD_CAPTURE_DMA_WORDS_PER_HALF * APP_PCMD_CAPTURE_DMA_HALVES)
#define APP_PCMD_CAPTURE_AUDIO_SCALE         APP_MIC_ARRAY_Q15_TO_FLOAT_SCALE
#define APP_PCMD_CAPTURE_I2C_TIMEOUT_MS      20U
#define APP_PCMD_CAPTURE_RESET_LOW_MS        100U
#define APP_PCMD_CAPTURE_RESET_SETTLE_MS     10U
#define APP_PCMD_CAPTURE_CLOCK_SETTLE_MS     1000U
#define APP_PCMD_CAPTURE_RETRY_DELAY_MS      500U
#define APP_PCMD_CAPTURE_EVENT_HALF0         0x00000001UL
#define APP_PCMD_CAPTURE_EVENT_HALF1         0x00000002UL
#define APP_PCMD_CAPTURE_EVENT_MASK          (APP_PCMD_CAPTURE_EVENT_HALF0 | APP_PCMD_CAPTURE_EVENT_HALF1)

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

static void AppPcmdCapture_UpdateSlotLevels(uint8_t bus,
                                            const int16_t *interleaved,
                                            uint32_t frame_len)
{
  int32_t dc_sum[APP_PCMD_CAPTURE_SLOTS_PER_BUS] = { 0 };
  int16_t dc_level[APP_PCMD_CAPTURE_SLOTS_PER_BUS] = { 0 };
  uint32_t ac_sum[APP_PCMD_CAPTURE_SLOTS_PER_BUS] = { 0U };

  if ((bus >= APP_PCMD_CAPTURE_BUS_COUNT) || (interleaved == NULL) || (frame_len == 0U))
  {
    return;
  }

  for (uint32_t frame = 0U; frame < frame_len; frame++)
  {
    for (uint32_t slot = 0U; slot < APP_PCMD_CAPTURE_SLOTS_PER_BUS; slot++)
    {
      const int16_t sample = interleaved[(frame * APP_PCMD_CAPTURE_SLOTS_PER_BUS) + slot];
      dc_sum[slot] += sample;
      s_snapshot.slot_last_sample[bus][slot] = sample;
    }
  }

  for (uint32_t slot = 0U; slot < APP_PCMD_CAPTURE_SLOTS_PER_BUS; slot++)
  {
    dc_level[slot] = (int16_t)(dc_sum[slot] / (int32_t)frame_len);
  }

  for (uint32_t frame = 0U; frame < frame_len; frame++)
  {
    for (uint32_t slot = 0U; slot < APP_PCMD_CAPTURE_SLOTS_PER_BUS; slot++)
    {
      const int32_t sample = (int32_t)interleaved[(frame * APP_PCMD_CAPTURE_SLOTS_PER_BUS) + slot];
      const int32_t ac_sample = sample - (int32_t)dc_level[slot];
      ac_sum[slot] += (uint32_t)((ac_sample < 0) ? -ac_sample : ac_sample);
    }
  }

  for (uint32_t slot = 0U; slot < APP_PCMD_CAPTURE_SLOTS_PER_BUS; slot++)
  {
    uint32_t raw_level = ac_sum[slot] / frame_len;
    int8_t dbfs;

    if (raw_level > 65535U)
    {
      raw_level = 65535U;
    }

    dbfs = AppPcmdCapture_LevelToDbfs((uint16_t)raw_level);
    s_snapshot.slot_level_raw[bus][slot] = (uint16_t)raw_level;
    s_snapshot.slot_dc_level[bus][slot] = dc_level[slot];
    s_snapshot.slot_dbfs[bus][slot] = dbfs;
    s_snapshot.slot_level[bus][slot] = AppPcmdCapture_DbfsToPercent(dbfs);
  }
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
  s_snapshot.raw_quality_flags = 0U;
  if (s_snapshot.device_config_ok_mask == 0x0FU)
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
  s_snapshot.raw_audio_valid =
      ((s_snapshot.raw_quality_flags & (APP_PCMD_CAPTURE_RAW_FLAG_CONFIG_OK |
                                        APP_PCMD_CAPTURE_RAW_FLAG_DMA_SYNC |
                                        APP_PCMD_CAPTURE_RAW_FLAG_NONZERO)) ==
       (APP_PCMD_CAPTURE_RAW_FLAG_CONFIG_OK |
        APP_PCMD_CAPTURE_RAW_FLAG_DMA_SYNC |
        APP_PCMD_CAPTURE_RAW_FLAG_NONZERO)) ? 1U : 0U;
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

  AppPcmdCapture_UpdateSlotLevels(APP_MIC_ARRAY_BUS_A, bus_a, APP_PCMD_CAPTURE_FRAME_LEN);
  AppPcmdCapture_UpdateSlotLevels(APP_MIC_ARRAY_BUS_B, bus_b, APP_PCMD_CAPTURE_FRAME_LEN);

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
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_ReadStatus(&s_pcmd_handles[index], &s_snapshot.device_status[index]);
    s_snapshot.device_status_status[index] = (int32_t)status;
    if (status == PCMD3180_OK)
    {
      s_snapshot.device_status_ok_mask = (uint8_t)(s_snapshot.device_status_ok_mask | (uint8_t)(1U << index));
    }
    else
    {
      s_snapshot.device_status_ok_mask = (uint8_t)(s_snapshot.device_status_ok_mask & (uint8_t)~(1U << index));
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
    PCMD3180_StatusTypeDef status = PCMD3180_HAL_ProbeAddress(&s_hal_context,
                                                               s_pcmd_handles[index].address7);
    if (status == PCMD3180_OK)
    {
      s_snapshot.device_present_mask = (uint8_t)(s_snapshot.device_present_mask | (uint8_t)(1U << index));
    }
    else
    {
      s_snapshot.device_config_status[index] = (int32_t)status;
      result = APP_PCMD_CAPTURE_PCMD_ERROR;
      continue;
    }

    status = PCMD3180_Probe(&s_pcmd_handles[index]);
    if (status != PCMD3180_OK)
    {
      s_snapshot.device_config_status[index] = (int32_t)status;
      result = APP_PCMD_CAPTURE_PCMD_ERROR;
      continue;
    }

    status = PCMD3180_Configure(&s_pcmd_handles[index], &s_pcmd_configs[index]);
    s_snapshot.device_config_status[index] = (int32_t)status;
    if (status == PCMD3180_OK)
    {
      s_snapshot.device_config_ok_mask = (uint8_t)(s_snapshot.device_config_ok_mask | (uint8_t)(1U << index));
    }
    else
    {
      result = APP_PCMD_CAPTURE_PCMD_ERROR;
    }
  }

  AppPcmdCapture_ReadDeviceStatus();
  return result;
}

static void AppPcmdCapture_RecordSaiErrors(void)
{
  s_snapshot.sai_a_last_error = HAL_SAI_GetError(&hsai_BlockA1);
  s_snapshot.sai_b_last_error = HAL_SAI_GetError(&hsai_BlockB1);
  s_snapshot.dma_a_error = HAL_DMA_GetError(&handle_GPDMA1_Channel0);
  s_snapshot.dma_b_error = HAL_DMA_GetError(&handle_GPDMA1_Channel1);
}

static void AppPcmdCapture_StopDma(void)
{
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

  s_hal_context.hi2c = &hi2c2;
  s_hal_context.shutdown_port = MIC_SHDNZ_GPIO_Port;
  s_hal_context.shutdown_pin = MIC_SHDNZ_Pin;
  s_hal_context.timeout_ms = APP_PCMD_CAPTURE_I2C_TIMEOUT_MS;
  PCMD3180_HAL_BusInit(&s_pcmd_bus, &s_hal_context);

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
  }

  s_snapshot.initialized = 1U;
  s_snapshot.init_status = APP_PCMD_CAPTURE_OK;
  return APP_PCMD_CAPTURE_OK;
}

AppPcmdCaptureStatus_t AppPcmdCapture_Start(void)
{
  HAL_StatusTypeDef hal_status;
  PCMD3180_StatusTypeDef pcmd_status;
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

  hal_status = HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)s_bus_a_rx, APP_PCMD_CAPTURE_DMA_WORDS);
  s_snapshot.dma_a_status = (int32_t)hal_status;
  if (hal_status != HAL_OK)
  {
    s_snapshot.start_status = APP_PCMD_CAPTURE_HAL_ERROR;
    AppPcmdCapture_StopDma();
    AppPcmdCapture_RecordSaiErrors();
    return APP_PCMD_CAPTURE_HAL_ERROR;
  }

  AppPcmdCapture_DelayMs(2U);

  hal_status = HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)s_bus_b_rx, APP_PCMD_CAPTURE_DMA_WORDS);
  s_snapshot.dma_b_status = (int32_t)hal_status;
  if (hal_status != HAL_OK)
  {
    s_snapshot.start_status = APP_PCMD_CAPTURE_HAL_ERROR;
    AppPcmdCapture_StopDma();
    AppPcmdCapture_RecordSaiErrors();
    return APP_PCMD_CAPTURE_HAL_ERROR;
  }

  __HAL_SAI_CLEAR_FLAG(&hsai_BlockB1, SAI_FLAG_AFSDET | SAI_FLAG_LFSDET);
  __HAL_SAI_DISABLE_IT(&hsai_BlockB1, SAI_IT_AFSDET | SAI_IT_LFSDET);
  AppPcmdCapture_DelayMs(APP_PCMD_CAPTURE_CLOCK_SETTLE_MS);

  pcmd_status = PCMD3180_HardwareReset(&s_pcmd_handles[0],
                                       APP_PCMD_CAPTURE_RESET_LOW_MS,
                                       APP_PCMD_CAPTURE_RESET_SETTLE_MS);
  if (pcmd_status != PCMD3180_OK)
  {
    s_snapshot.start_status = (int32_t)pcmd_status;
    AppPcmdCapture_StopDma();
    return APP_PCMD_CAPTURE_PCMD_ERROR;
  }

  config_status = AppPcmdCapture_ConfigPcmdDevices();
  if (config_status != APP_PCMD_CAPTURE_OK)
  {
    s_snapshot.start_status = config_status;
    AppPcmdCapture_StopDma();
    App_BringUpStatus_Fail(APP_BRINGUP_MODULE_PCMD_RAW, (int32_t)config_status);
    return config_status;
  }

  AppPcmdCapture_ClearPendingDmaEvents();

  s_snapshot.started = 1U;
  s_snapshot.start_status = APP_PCMD_CAPTURE_OK;
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
      ((now_ms - last_device_status_ms) >= 5000U))
  {
    last_device_status_ms = now_ms;
    AppPcmdCapture_ReadDeviceStatus();
  }

  return APP_PCMD_CAPTURE_OK;
}

void AppPcmdCapture_ThreadEntry(ULONG thread_input)
{
  (void)thread_input;

  while (1)
  {
    if (s_snapshot.initialized == 0U)
    {
      (void)AppPcmdCapture_Init(APP_MIC_ARRAY_MODE_WIDE32_48K);
    }

    if (s_snapshot.started == 0U)
    {
      (void)AppPcmdCapture_Start();
      if (s_snapshot.started == 0U)
      {
        AppPcmdCapture_DelayMs(APP_PCMD_CAPTURE_RETRY_DELAY_MS);
        continue;
      }
    }

    (void)AppPcmdCapture_Poll(TX_TIMER_TICKS_PER_SECOND / 10U);
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

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    s_snapshot.sai_a_half_count++;
    AppPcmdCapture_MarkHalfReady(APP_MIC_ARRAY_BUS_A, 0U);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    s_snapshot.sai_b_half_count++;
    AppPcmdCapture_MarkHalfReady(APP_MIC_ARRAY_BUS_B, 0U);
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    s_snapshot.sai_a_full_count++;
    AppPcmdCapture_MarkHalfReady(APP_MIC_ARRAY_BUS_A, 1U);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    s_snapshot.sai_b_full_count++;
    AppPcmdCapture_MarkHalfReady(APP_MIC_ARRAY_BUS_B, 1U);
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
