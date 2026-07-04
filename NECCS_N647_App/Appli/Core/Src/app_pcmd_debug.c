#include "app_pcmd_debug.h"

#include "main.h"
#include "app_pcmd_bus.h"
#include "./PCMD3180/pcmd3180.h"
#include "./RGBLCD/rgblcd.h"

#include <stdio.h>
#include <string.h>

typedef struct
{
  uint8_t present;
  PCMD3180_StatusTypeDef probe_status;
  PCMD3180_StatusTypeDef config_status;
  PCMD3180_StatusTypeDef status_status;
  PCMD3180_StatusSnapshotTypeDef snapshot;
} AppPcmdDeviceState_t;

typedef struct
{
  PCMD3180_ArrayModeTypeDef mode;
  PCMD3180_ArrayModeConfigTypeDef mode_config;
  PCMD3180_StatusTypeDef mode_config_status;
  HAL_StatusTypeDef sai_status;
  HAL_StatusTypeDef dma_a_status;
  HAL_StatusTypeDef dma_b_status;
  uint32_t mode_start_tick;
  uint32_t last_poll_tick;
  uint32_t reconfigure_count;
  uint8_t early_address_ack_count[PCMD3180_ARRAY_DEVICE_COUNT];
  uint8_t early_address_scan_rounds;
  uint8_t early_address_scan_ok;
  uint8_t early_scl_idle_high;
  uint8_t early_sda_idle_high;
  uint8_t address_ack_count[PCMD3180_ARRAY_DEVICE_COUNT];
  uint8_t address_scan_rounds;
  uint8_t address_scan_ok;
  uint8_t scl_idle_high;
  uint8_t sda_idle_high;
  uint8_t initialized;
} AppPcmdDebugState_t;

#define APP_PCMD_MODE_STEP_MS      7000U
#define APP_PCMD_POLL_MS           0U
#define APP_PCMD_UI_REFRESH_MS     250U
#define APP_PCMD_DMA_WORDS         8192U
#define APP_PCMD_I2C_TIMEOUT_MS    100U
#define APP_PCMD_MAX_SLOTS         16U
#define APP_PCMD_BUS_COUNT         2U
#define APP_PCMD_DEFAULT_MODE      PCMD3180_ARRAY_MODE_32CH_48K
#define APP_PCMD_AUTO_MODE_SWITCH  0U
#ifndef APP_PCMD_SDOUT_BCLK_MARGIN_FIX
#define APP_PCMD_SDOUT_BCLK_MARGIN_FIX  1U
#endif
#define APP_PCMD_RESET_LOW_MS      100U
#define APP_PCMD_RESET_SETTLE_MS   10U
#define APP_PCMD_CLOCK_SETTLE_MS   1000U
#define APP_PCMD_ADDR_SCAN_ROUNDS  16U
#define APP_PCMD_UI_X              8U
#define APP_PCMD_UI_W              584U
#define APP_PCMD_BAR_X_OFFSET      72U
#define APP_PCMD_BAR_W             48U
#define APP_PCMD_BAR_H             10U
#define APP_PCMD_BAR_FULL_SCALE    0x0800U

extern SAI_HandleTypeDef hsai_BlockA1;
extern SAI_HandleTypeDef hsai_BlockB1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;

static uint16_t g_pcmd_sai_a_rx[APP_PCMD_DMA_WORDS]
    __attribute__((section(".noncacheable"), aligned(32)));
static uint16_t g_pcmd_sai_b_rx[APP_PCMD_DMA_WORDS]
    __attribute__((section(".noncacheable"), aligned(32)));
static PCMD3180_HandleTypeDef g_pcmd_handles[PCMD3180_ARRAY_DEVICE_COUNT];
static PCMD3180_ConfigTypeDef g_pcmd_device_configs[PCMD3180_ARRAY_DEVICE_COUNT];
static AppPcmdDeviceState_t g_pcmd_devices[PCMD3180_ARRAY_DEVICE_COUNT];
static AppPcmdDebugState_t g_pcmd_debug;
static volatile uint32_t g_pcmd_sai_a_half_count = 0;
static volatile uint32_t g_pcmd_sai_a_full_count = 0;
static volatile uint32_t g_pcmd_sai_a_error_count = 0;
static volatile uint32_t g_pcmd_sai_b_half_count = 0;
static volatile uint32_t g_pcmd_sai_b_full_count = 0;
static volatile uint32_t g_pcmd_sai_b_error_count = 0;
static volatile uint32_t g_pcmd_sai_a_last_error = 0;
static volatile uint32_t g_pcmd_sai_b_last_error = 0;
static uint32_t g_pcmd_sai_rate_tick = 0;
static uint32_t g_pcmd_sai_a_rate_last_count = 0;
static uint32_t g_pcmd_sai_b_rate_last_count = 0;
static uint32_t g_pcmd_sai_a_full_rate = 0;
static uint32_t g_pcmd_sai_b_full_rate = 0;
static volatile uint32_t g_pcmd_slot_abs_sum[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
static volatile int32_t g_pcmd_slot_dc_sum[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
static volatile uint32_t g_pcmd_slot_abs_count[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
static volatile uint16_t g_pcmd_slot_level[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
static volatile int16_t g_pcmd_slot_dc_level[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
static volatile uint16_t g_pcmd_slot_last_sample[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
static uint32_t g_pcmd_last_ui_tick = 0;
static volatile uint32_t g_app_pcmd_ram_test_ok = 0;

static const char *App_PCMD_ModeName(PCMD3180_ArrayModeTypeDef mode);
static const char *App_PCMD_StatusName(PCMD3180_StatusTypeDef status);
static uint8_t App_PCMD_IsRoutingSnapshotOk(uint32_t device_index,
                                            const PCMD3180_StatusSnapshotTypeDef *snapshot);
static const char *App_SAI_StateName(uint32_t state);
static const char *App_SAI_ErrorName(uint32_t error_code);
static void App_PCMD_StartDma(void);
static void App_PCMD_ProcessBuffer(const uint16_t *buffer,
                                   uint32_t sample_words,
                                   uint8_t slots_per_bus,
                                   volatile uint32_t *slot_abs_sum,
                                   volatile int32_t *slot_dc_sum,
                                   volatile uint32_t *slot_abs_count,
                                   volatile uint16_t *slot_samples);
static void App_PCMD_UpdateLevelsFromAccumulator(void);
static void App_PCMD_PollStatus(void);
static uint8_t App_PCMD_CheckAddressMap(void);
static void App_PCMD_SetExpectedSnapshot(uint32_t device_index,
                                         const PCMD3180_ConfigTypeDef *device_config);
static void App_PCMD_ConfigureMode(PCMD3180_ArrayModeTypeDef mode);
static void App_PCMD_EarlyI2CTest(void) __attribute__((unused));
static void App_PCMD_DebugInit(void);
static void App_PCMD_Task(void);
static void App_PCMD_ShowMicActivity(uint16_t start_y);
static void App_PCMD_ShowDebugPage(void);
static uint16_t App_PCMD_LevelToBarWidth(uint16_t level);
static uint16_t App_PCMD_LevelToBarColor(uint16_t level);
static int16_t App_PCMD_LevelToDbfs(uint16_t level);
static void App_PCMD_DrawLevelBar(uint16_t x,
                                  uint16_t y,
                                  uint16_t level,
                                  uint8_t enabled,
                                  uint8_t healthy);
static const char *App_PCMD_ModeName(PCMD3180_ArrayModeTypeDef mode)
{
  switch (mode)
  {
    case PCMD3180_ARRAY_MODE_32CH_48K:
      return "32CH 48K";
    case PCMD3180_ARRAY_MODE_CORE16_192K:
      return "CORE16 192K";
    default:
      return "UNKNOWN";
  }
}

static const char *App_PCMD_StatusName(PCMD3180_StatusTypeDef status)
{
  switch (status)
  {
    case PCMD3180_OK:
      return "OK";
    case PCMD3180_ERROR:
      return "ERR";
    case PCMD3180_INVALID_ARGUMENT:
      return "ARG";
    case PCMD3180_IO_ERROR:
      return "IO";
    case PCMD3180_VERIFY_ERROR:
      return "VERIFY";
    case PCMD3180_TIMEOUT:
      return "TIMEOUT";
    default:
      return "ERR";
  }
}

static uint8_t App_PCMD_IsRoutingSnapshotOk(uint32_t device_index,
                                            const PCMD3180_StatusSnapshotTypeDef *snapshot)
{
  uint8_t expected_mask = PCMD3180_CHANNEL_ALL;

  if (snapshot == NULL)
  {
    return 0U;
  }

  if (device_index < PCMD3180_ARRAY_DEVICE_COUNT)
  {
    expected_mask = g_pcmd_debug.mode_config.devices[device_index].input_channel_mask;
    const uint8_t expected_start = g_pcmd_debug.mode_config.devices[device_index].start_slot;
    for (uint32_t channel = 0U; channel < PCMD3180_ARRAY_MAX_MICS_PER_DEV; channel++)
    {
      if (snapshot->asi_ch_slot[channel] != (uint8_t)(expected_start + channel))
      {
        return 0U;
      }
    }
  }

  if (snapshot->pdmclk_cfg != (PCMD3180_PDMCLK_CFG_RESET_MASK |
                               (uint8_t)PCMD3180_PDMCLK_DIV_64FS))
  {
    return 0U;
  }
  if (snapshot->pdmin_cfg != 0U)
  {
    return 0U;
  }
  if ((snapshot->gpo_cfg0 != PCMD3180_GPO_CFG_PDMCLK_OUTPUT) ||
      (snapshot->gpo_cfg1 != PCMD3180_GPO_CFG_PDMCLK_OUTPUT) ||
      (snapshot->gpo_cfg2 != PCMD3180_GPO_CFG_PDMCLK_OUTPUT) ||
      (snapshot->gpo_cfg3 != PCMD3180_GPO_CFG_PDMCLK_OUTPUT))
  {
    return 0U;
  }
  if ((snapshot->gpi_cfg0 != PCMD3180_GPI_CFG0_DEFAULT) ||
      (snapshot->gpi_cfg1 != PCMD3180_GPI_CFG1_DEFAULT))
  {
    return 0U;
  }
  if ((snapshot->in_ch_en != expected_mask) ||
      (snapshot->asi_out_ch_en != expected_mask))
  {
    return 0U;
  }
  if ((snapshot->pwr_cfg & PCMD3180_PWR_PDM_AND_PLL) != PCMD3180_PWR_PDM_AND_PLL)
  {
    return 0U;
  }

  return 1U;
}

static const char *App_SAI_StateName(uint32_t state)
{
  switch (state)
  {
    case HAL_SAI_STATE_RESET:
      return "RESET";
    case HAL_SAI_STATE_READY:
      return "READY";
    case HAL_SAI_STATE_BUSY:
      return "BUSY";
    case HAL_SAI_STATE_BUSY_TX:
      return "BUSY_TX";
    case HAL_SAI_STATE_BUSY_RX:
      return "BUSY_RX";
    default:
      return "UNK";
  }
}

static const char *App_SAI_ErrorName(uint32_t error_code)
{
  if (error_code == HAL_SAI_ERROR_NONE)
  {
    return "NONE";
  }
  if ((error_code & (HAL_SAI_ERROR_WCKCFG | HAL_SAI_ERROR_TIMEOUT)) ==
      (HAL_SAI_ERROR_WCKCFG | HAL_SAI_ERROR_TIMEOUT))
  {
    return "WCK+TO";
  }
  if ((error_code & (HAL_SAI_ERROR_AFSDET | HAL_SAI_ERROR_TIMEOUT)) ==
      (HAL_SAI_ERROR_AFSDET | HAL_SAI_ERROR_TIMEOUT))
  {
    return "AFS+TO";
  }
  if ((error_code & (HAL_SAI_ERROR_LFSDET | HAL_SAI_ERROR_TIMEOUT)) ==
      (HAL_SAI_ERROR_LFSDET | HAL_SAI_ERROR_TIMEOUT))
  {
    return "LFS+TO";
  }
  if ((error_code & HAL_SAI_ERROR_DMA) != 0U)
  {
    return "DMA";
  }
  if ((error_code & HAL_SAI_ERROR_WCKCFG) != 0U)
  {
    return "WCK";
  }
  if ((error_code & HAL_SAI_ERROR_LFSDET) != 0U)
  {
    return "LFS";
  }
  if ((error_code & HAL_SAI_ERROR_AFSDET) != 0U)
  {
    return "AFS";
  }
  if ((error_code & HAL_SAI_ERROR_OVR) != 0U)
  {
    return "OVR";
  }
  if ((error_code & HAL_SAI_ERROR_UDR) != 0U)
  {
    return "UDR";
  }
  if ((error_code & HAL_SAI_ERROR_CNREADY) != 0U)
  {
    return "CNRDY";
  }
  if ((error_code & HAL_SAI_ERROR_TIMEOUT) != 0U)
  {
    return "TO";
  }

  return "ERR";
}

static void App_PCMD_StartDma(void)
{
  if (HAL_SAI_GetState(&hsai_BlockA1) == HAL_SAI_STATE_READY)
  {
    g_pcmd_debug.dma_a_status =
        HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)g_pcmd_sai_a_rx, APP_PCMD_DMA_WORDS);
    if (g_pcmd_debug.dma_a_status != HAL_OK)
    {
      return;
    }

    /* Block A is the clock master; let BCLK/FSYNC start before enabling B. */
    HAL_Delay(2U);
  }

  if (HAL_SAI_GetState(&hsai_BlockB1) == HAL_SAI_STATE_READY)
  {
    g_pcmd_debug.dma_b_status =
        HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)g_pcmd_sai_b_rx, APP_PCMD_DMA_WORDS);
    if (g_pcmd_debug.dma_b_status == HAL_OK)
    {
      __HAL_SAI_CLEAR_FLAG(&hsai_BlockB1, SAI_FLAG_AFSDET | SAI_FLAG_LFSDET);
      __HAL_SAI_DISABLE_IT(&hsai_BlockB1, SAI_IT_AFSDET | SAI_IT_LFSDET);
    }
  }
}

static void App_PCMD_ProcessBuffer(const uint16_t *buffer,
                                   uint32_t word_count,
                                   uint8_t slot_count,
                                   volatile uint32_t *slot_abs_sum,
                                   volatile int32_t *slot_dc_sum,
                                   volatile uint32_t *slot_abs_count,
                                   volatile uint16_t *last_sample)
{
  int32_t instant_dc_sum[APP_PCMD_MAX_SLOTS] = {0};
  uint32_t instant_sum[APP_PCMD_MAX_SLOTS] = {0U};
  uint16_t instant_count[APP_PCMD_MAX_SLOTS] = {0U};
  uint16_t instant_last[APP_PCMD_MAX_SLOTS] = {0U};

  if ((buffer == NULL) || (slot_abs_sum == NULL) || (slot_dc_sum == NULL) ||
      (slot_abs_count == NULL) ||
      (last_sample == NULL) ||
      (slot_count == 0U) || (slot_count > APP_PCMD_MAX_SLOTS))
  {
    return;
  }

  for (uint32_t word = 0U; word < word_count; word++)
  {
    const int32_t sample = (int16_t)buffer[word];
    const uint32_t slot = word % slot_count;

    instant_dc_sum[slot] += sample;
    instant_count[slot]++;
    instant_last[slot] = buffer[word];
  }

  for (uint32_t word = 0U; word < word_count; word++)
  {
    const int32_t sample = (int16_t)buffer[word];
    const uint32_t slot = word % slot_count;
    const int32_t mean = (instant_count[slot] == 0U) ? 0 :
                         (instant_dc_sum[slot] / (int32_t)instant_count[slot]);
    const int32_t ac_sample = sample - mean;
    const uint32_t magnitude =
        (ac_sample < 0) ? (uint32_t)(-ac_sample) : (uint32_t)ac_sample;

    instant_sum[slot] += magnitude;
  }

  for (uint32_t slot = 0U; slot < APP_PCMD_MAX_SLOTS; slot++)
  {
    if (slot >= slot_count)
    {
      slot_abs_sum[slot] = 0U;
      slot_dc_sum[slot] = 0;
      slot_abs_count[slot] = 0U;
      last_sample[slot] = 0U;
    }
    else
    {
      if ((UINT32_MAX - slot_abs_sum[slot]) > instant_sum[slot])
      {
        slot_abs_sum[slot] += instant_sum[slot];
      }
      else
      {
        slot_abs_sum[slot] = UINT32_MAX;
      }

      slot_dc_sum[slot] += instant_dc_sum[slot];

      if ((UINT32_MAX - slot_abs_count[slot]) > instant_count[slot])
      {
        slot_abs_count[slot] += instant_count[slot];
      }
      else
      {
        slot_abs_count[slot] = UINT32_MAX;
      }
      last_sample[slot] = instant_last[slot];
    }
  }
}

static void App_PCMD_UpdateLevelsFromAccumulator(void)
{
  uint32_t sums[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
  int32_t dc_sums[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
  uint32_t counts[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
  const uint32_t primask = __get_PRIMASK();

  __disable_irq();
  for (uint32_t bus = 0U; bus < APP_PCMD_BUS_COUNT; bus++)
  {
    for (uint32_t slot = 0U; slot < APP_PCMD_MAX_SLOTS; slot++)
    {
      sums[bus][slot] = g_pcmd_slot_abs_sum[bus][slot];
      dc_sums[bus][slot] = g_pcmd_slot_dc_sum[bus][slot];
      counts[bus][slot] = g_pcmd_slot_abs_count[bus][slot];
      g_pcmd_slot_abs_sum[bus][slot] = 0U;
      g_pcmd_slot_dc_sum[bus][slot] = 0;
      g_pcmd_slot_abs_count[bus][slot] = 0U;
    }
  }
  if ((primask & 1U) == 0U)
  {
    __enable_irq();
  }

  for (uint32_t bus = 0U; bus < APP_PCMD_BUS_COUNT; bus++)
  {
    for (uint32_t slot = 0U; slot < APP_PCMD_MAX_SLOTS; slot++)
    {
      if (counts[bus][slot] == 0U)
      {
        g_pcmd_slot_level[bus][slot] = 0U;
        g_pcmd_slot_dc_level[bus][slot] = 0;
      }
      else
      {
        uint32_t avg = sums[bus][slot] / counts[bus][slot];
        int32_t dc_avg = dc_sums[bus][slot] / (int32_t)counts[bus][slot];
        if (avg > UINT16_MAX)
        {
          avg = UINT16_MAX;
        }
        if (dc_avg > INT16_MAX)
        {
          dc_avg = INT16_MAX;
        }
        else if (dc_avg < INT16_MIN)
        {
          dc_avg = INT16_MIN;
        }
        g_pcmd_slot_level[bus][slot] = (uint16_t)avg;
        g_pcmd_slot_dc_level[bus][slot] = (int16_t)dc_avg;
      }
    }
  }
}

static uint16_t App_PCMD_LevelToBarWidth(uint16_t level)
{
  uint32_t width = ((uint32_t)level * APP_PCMD_BAR_W) / APP_PCMD_BAR_FULL_SCALE;

  if (level == 0U)
  {
    return 0U;
  }

  if (width == 0U)
  {
    width = 1U;
  }
  if (width > APP_PCMD_BAR_W)
  {
    width = APP_PCMD_BAR_W;
  }

  return (uint16_t)width;
}

static uint16_t App_PCMD_LevelToBarColor(uint16_t level)
{
  if (level >= 0x0400U)
  {
    return GREEN;
  }
  if (level >= 0x0100U)
  {
    return YELLOW;
  }
  if (level != 0U)
  {
    return CYAN;
  }

  return LGRAY;
}

static int16_t App_PCMD_LevelToDbfs(uint16_t level)
{
  static const struct
  {
    uint16_t threshold;
    int16_t dbfs;
  } table[] =
  {
    {32700U,   0}, {29155U,  -1}, {25986U,  -2}, {23162U,  -3},
    {20645U,  -4}, {18401U,  -5}, {16402U,  -6}, {13029U,  -8},
    {10351U, -10}, { 8224U, -12}, { 6534U, -14}, { 5192U, -16},
    { 4125U, -18}, { 3277U, -20}, { 2068U, -24}, { 1305U, -28},
    { 1036U, -30}, {  519U, -36}, {  260U, -42}, {  130U, -48},
    {   65U, -54}, {   33U, -60}, {   16U, -66}, {    8U, -72},
    {    4U, -78}, {    2U, -84}, {    1U, -90},
  };

  for (uint32_t i = 0U; i < (sizeof(table) / sizeof(table[0])); i++)
  {
    if (level >= table[i].threshold)
    {
      return table[i].dbfs;
    }
  }

  return -90;
}

static void App_PCMD_DrawLevelBar(uint16_t x,
                                  uint16_t y,
                                  uint16_t level,
                                  uint8_t enabled,
                                  uint8_t healthy)
{
  const uint16_t x2 = (uint16_t)(x + APP_PCMD_BAR_W);
  const uint16_t y2 = (uint16_t)(y + APP_PCMD_BAR_H);
  const uint16_t inner_x = (uint16_t)(x + 1U);
  const uint16_t inner_y = (uint16_t)(y + 1U);
  const uint16_t inner_x2 = (uint16_t)(x2 - 1U);
  const uint16_t inner_y2 = (uint16_t)(y2 - 1U);
  const uint16_t border_color = (healthy != 0U) ? LGRAY : RED;

  rgblcd_draw_rectangle(x, y, x2, y2, border_color);
  rgblcd_fill(inner_x, inner_y, inner_x2, inner_y2, BLACK);

  if ((enabled == 0U) || (healthy == 0U))
  {
    return;
  }

  {
    const uint16_t fill_width = App_PCMD_LevelToBarWidth(level);

    if (fill_width != 0U)
    {
      rgblcd_fill(inner_x,
                  inner_y,
                  (uint16_t)(inner_x + fill_width - 1U),
                  inner_y2,
                  App_PCMD_LevelToBarColor(level));
    }
  }
}

static void App_PCMD_PollStatus(void)
{
  for (uint32_t i = 0; i < PCMD3180_ARRAY_DEVICE_COUNT; i++)
  {
    if (g_pcmd_devices[i].present != 0U)
    {
      g_pcmd_devices[i].status_status =
          PCMD3180_ReadStatus(&g_pcmd_handles[i], &g_pcmd_devices[i].snapshot);
    }
  }
}

static uint8_t App_PCMD_CheckAddressMap(void)
{
  return App_PCMD_BusRunAddressScan(g_pcmd_debug.address_ack_count,
                                    &g_pcmd_debug.address_scan_rounds,
                                    &g_pcmd_debug.scl_idle_high,
                                    &g_pcmd_debug.sda_idle_high,
                                    APP_PCMD_ADDR_SCAN_ROUNDS);
}

static void App_PCMD_SetExpectedSnapshot(uint32_t device_index,
                                         const PCMD3180_ConfigTypeDef *device_config)
{
  PCMD3180_StatusSnapshotTypeDef *snapshot;

  if ((device_index >= PCMD3180_ARRAY_DEVICE_COUNT) || (device_config == NULL))
  {
    return;
  }

  snapshot = &g_pcmd_devices[device_index].snapshot;
  memset(snapshot, 0, sizeof(*snapshot));
  for (uint32_t channel = 0U; channel < PCMD3180_ARRAY_MAX_MICS_PER_DEV; channel++)
  {
    snapshot->asi_ch_slot[channel] = (uint8_t)(device_config->start_slot + channel);
  }
  snapshot->pwr_cfg = PCMD3180_PWR_PDM_AND_PLL |
                      (uint8_t)((device_config->enable_micbias == 0U) ? 0U : PCMD3180_PWR_MICBIAS);
  snapshot->pdmclk_cfg = PCMD3180_PDMCLK_CFG_RESET_MASK |
                         ((uint8_t)device_config->pdmclk_divider & 0x03U);
  snapshot->pdmin_cfg = device_config->pdmin_edge_mask;
  snapshot->gpo_cfg0 = PCMD3180_GPO_CFG_PDMCLK_OUTPUT;
  snapshot->gpo_cfg1 = PCMD3180_GPO_CFG_PDMCLK_OUTPUT;
  snapshot->gpo_cfg2 = PCMD3180_GPO_CFG_PDMCLK_OUTPUT;
  snapshot->gpo_cfg3 = PCMD3180_GPO_CFG_PDMCLK_OUTPUT;
  snapshot->gpi_cfg0 = PCMD3180_GPI_CFG0_DEFAULT;
  snapshot->gpi_cfg1 = PCMD3180_GPI_CFG1_DEFAULT;
  snapshot->in_ch_en = device_config->input_channel_mask;
  snapshot->asi_out_ch_en = device_config->output_channel_mask;
}

static void App_PCMD_ConfigureMode(PCMD3180_ArrayModeTypeDef mode)
{
  PCMD3180_ArrayModeConfigTypeDef mode_config;
  PCMD3180_StatusTypeDef pcmd_status;

  memset(&mode_config, 0, sizeof(mode_config));
  memset(g_pcmd_device_configs, 0, sizeof(g_pcmd_device_configs));
  memset(g_pcmd_devices, 0, sizeof(g_pcmd_devices));
  for (uint32_t i = 0U; i < PCMD3180_ARRAY_DEVICE_COUNT; i++)
  {
    g_pcmd_devices[i].probe_status = PCMD3180_ERROR;
    g_pcmd_devices[i].config_status = PCMD3180_ERROR;
    g_pcmd_devices[i].status_status = PCMD3180_ERROR;
  }
  for (uint32_t bus = 0U; bus < APP_PCMD_BUS_COUNT; bus++)
  {
    for (uint32_t slot = 0U; slot < APP_PCMD_MAX_SLOTS; slot++)
    {
      g_pcmd_slot_abs_sum[bus][slot] = 0U;
      g_pcmd_slot_dc_sum[bus][slot] = 0;
      g_pcmd_slot_abs_count[bus][slot] = 0U;
      g_pcmd_slot_level[bus][slot] = 0U;
      g_pcmd_slot_dc_level[bus][slot] = 0;
      g_pcmd_slot_last_sample[bus][slot] = 0U;
    }
  }

  g_pcmd_debug.mode = mode;
  g_pcmd_debug.mode_config_status = PCMD3180_GetArrayModeConfig(mode, &mode_config);
  g_pcmd_debug.sai_status = HAL_OK;
  if (g_pcmd_debug.mode_config_status != PCMD3180_OK)
  {
    g_pcmd_debug.mode_start_tick = HAL_GetTick();
    return;
  }

  g_pcmd_debug.mode_config = mode_config;

  /*
   * Keep the H7-proven bring-up order: SAI DMA is already running here, so
   * BCLK/FSYNC are stable before the PCMD3180 register table is written.
   */
  if (g_pcmd_debug.early_address_scan_rounds != 0U)
  {
    memcpy(g_pcmd_debug.address_ack_count,
           g_pcmd_debug.early_address_ack_count,
           sizeof(g_pcmd_debug.address_ack_count));
    g_pcmd_debug.address_scan_rounds = g_pcmd_debug.early_address_scan_rounds;
    g_pcmd_debug.address_scan_ok = g_pcmd_debug.early_address_scan_ok;
    g_pcmd_debug.scl_idle_high = g_pcmd_debug.early_scl_idle_high;
    g_pcmd_debug.sda_idle_high = g_pcmd_debug.early_sda_idle_high;
  }
  else
  {
    g_pcmd_debug.address_scan_ok = App_PCMD_CheckAddressMap();
  }

  /*
   * Keep the address scan as a diagnostic only. A noisy scan while SAI/LTDC are
   * already active must not prevent the H7/TI register sequence from running,
   * otherwise every device row only reports the scan gate instead of the real
   * probe/configure failure point.
   */

  for (uint32_t i = 0; i < PCMD3180_ARRAY_DEVICE_COUNT; i++)
  {
    const uint8_t address7 = mode_config.devices[i].address7;

    (void)PCMD3180_Init(&g_pcmd_handles[i], App_PCMD_BusGet(), address7);
    g_pcmd_devices[i].present = 1U;

    /*
     * The TI bring-up sequence starts with writes (wake + register table).
     * Keep this read probe as a diagnostic only; a transient read failure must
     * not prevent the actual configuration writes from running.
     */
    pcmd_status = PCMD3180_Probe(&g_pcmd_handles[i]);
    g_pcmd_devices[i].probe_status = pcmd_status;

    pcmd_status = PCMD3180_BuildDeviceConfig(&mode_config, i, &g_pcmd_device_configs[i]);
    if (pcmd_status == PCMD3180_OK)
    {
      g_pcmd_device_configs[i].defer_power_up = 0U;
#if (APP_PCMD_SDOUT_BCLK_MARGIN_FIX != 0U)
      g_pcmd_device_configs[i].invert_bclk = 1U;
#endif
      pcmd_status = PCMD3180_Configure(&g_pcmd_handles[i], &g_pcmd_device_configs[i]);
    }
    g_pcmd_devices[i].config_status = pcmd_status;
    if (pcmd_status == PCMD3180_OK)
    {
      App_PCMD_SetExpectedSnapshot(i, &g_pcmd_device_configs[i]);
      g_pcmd_devices[i].status_status = PCMD3180_OK;
    }
  }

  g_pcmd_debug.mode_start_tick = HAL_GetTick();
  g_pcmd_debug.last_poll_tick = 0U;
  g_pcmd_debug.reconfigure_count++;
  g_pcmd_sai_rate_tick = HAL_GetTick();
  g_pcmd_sai_a_rate_last_count = g_pcmd_sai_a_full_count;
  g_pcmd_sai_b_rate_last_count = g_pcmd_sai_b_full_count;
  g_pcmd_sai_a_full_rate = 0U;
  g_pcmd_sai_b_full_rate = 0U;
}

static void App_PCMD_EarlyI2CTest(void)
{
  App_PCMD_BusPrepare(1U, APP_PCMD_RESET_LOW_MS, APP_PCMD_RESET_SETTLE_MS);
  g_pcmd_debug.early_address_scan_ok =
      App_PCMD_BusRunAddressScan(g_pcmd_debug.early_address_ack_count,
                                 &g_pcmd_debug.early_address_scan_rounds,
                                 &g_pcmd_debug.early_scl_idle_high,
                                 &g_pcmd_debug.early_sda_idle_high,
                                 APP_PCMD_ADDR_SCAN_ROUNDS);
}

static void App_PCMD_DebugInit(void)
{
  memset(g_pcmd_devices, 0, sizeof(g_pcmd_devices));

  if (g_pcmd_debug.early_address_scan_rounds == 0U)
  {
    App_PCMD_BusPrepare(1U, APP_PCMD_RESET_LOW_MS, APP_PCMD_RESET_SETTLE_MS);
  }
  else
  {
    App_PCMD_BusPrepare(0U, APP_PCMD_RESET_LOW_MS, APP_PCMD_RESET_SETTLE_MS);
  }

  g_pcmd_debug.initialized = 1U;
  App_PCMD_ConfigureMode(APP_PCMD_DEFAULT_MODE);
}

static void App_PCMD_Task(void)
{
  const uint32_t now = HAL_GetTick();

  if (g_pcmd_debug.initialized == 0U)
  {
    return;
  }

  if ((APP_PCMD_AUTO_MODE_SWITCH != 0U) &&
      ((now - g_pcmd_debug.mode_start_tick) >= APP_PCMD_MODE_STEP_MS))
  {
    const PCMD3180_ArrayModeTypeDef next_mode =
        (g_pcmd_debug.mode == PCMD3180_ARRAY_MODE_32CH_48K) ?
        PCMD3180_ARRAY_MODE_CORE16_192K :
        PCMD3180_ARRAY_MODE_32CH_48K;

    App_PCMD_ConfigureMode(next_mode);
  }

  if ((APP_PCMD_POLL_MS != 0U) &&
      ((now - g_pcmd_debug.last_poll_tick) >= APP_PCMD_POLL_MS))
  {
    g_pcmd_debug.last_poll_tick = now;
    App_PCMD_PollStatus();
  }

  if ((now - g_pcmd_sai_rate_tick) >= 1000U)
  {
    const uint32_t elapsed = now - g_pcmd_sai_rate_tick;
    const uint32_t sai_a_count = g_pcmd_sai_a_full_count;
    const uint32_t sai_b_count = g_pcmd_sai_b_full_count;

    g_pcmd_sai_a_full_rate =
        ((sai_a_count - g_pcmd_sai_a_rate_last_count) * 1000U) / elapsed;
    g_pcmd_sai_b_full_rate =
        ((sai_b_count - g_pcmd_sai_b_rate_last_count) * 1000U) / elapsed;
    g_pcmd_sai_a_rate_last_count = sai_a_count;
    g_pcmd_sai_b_rate_last_count = sai_b_count;
    g_pcmd_sai_rate_tick = now;
  }
}

static void App_PCMD_ShowMicActivity(uint16_t start_y)
{
  char label[32];
  PCMD3180_ArrayModeConfigTypeDef full_array_map;
  const uint16_t row_h = 22U;
  const uint16_t col_x0 = 56U;
  const uint16_t col_w = 132U;
  const uint16_t text_w = (uint16_t)(APP_PCMD_BAR_X_OFFSET - 4U);
  uint16_t y = start_y;

  memset(&full_array_map, 0, sizeof(full_array_map));
  (void)PCMD3180_GetArrayModeConfig(PCMD3180_ARRAY_MODE_32CH_48K, &full_array_map);

  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 16, 16,
                     "MIC AC dBFS + bar: U1-U4 all channels",
                     CYAN);
  y += 24U;

  rgblcd_show_string(APP_PCMD_UI_X, y, 40, 12, 12, "CH", YELLOW);
  for (uint32_t device_index = 0U;
       device_index < PCMD3180_ARRAY_DEVICE_COUNT;
       device_index++)
  {
    snprintf(label, sizeof(label), "U%lu", (unsigned long)(device_index + 1U));
    rgblcd_show_string((uint16_t)(col_x0 + device_index * col_w),
                       y,
                       (uint16_t)(col_w - 4U),
                       12,
                       12,
                       label,
                       YELLOW);
  }
  y += 18U;

  for (uint32_t channel = 0U;
       channel < PCMD3180_ARRAY_MAX_MICS_PER_DEV;
       channel++)
  {
    const uint16_t row_y = (uint16_t)(y + channel * row_h);

    snprintf(label, sizeof(label), "C%lu", (unsigned long)(channel + 1U));
    rgblcd_show_string(APP_PCMD_UI_X, row_y, 40, 12, 12, label, LGRAY);

    for (uint32_t device_index = 0U;
         device_index < PCMD3180_ARRAY_DEVICE_COUNT;
         device_index++)
    {
      const AppPcmdDeviceState_t *device = &g_pcmd_devices[device_index];
      const PCMD3180_ArrayDevicePlanTypeDef *plan =
          &g_pcmd_debug.mode_config.devices[device_index];
      const uint16_t x = (uint16_t)(col_x0 + device_index * col_w);
      const uint16_t bar_x = (uint16_t)(x + APP_PCMD_BAR_X_OFFSET);
      const uint16_t bar_y = (uint16_t)(row_y + 1U);
      const uint8_t mic_id = full_array_map.devices[device_index].mic_id[channel];
      const uint8_t channel_bit = (uint8_t)(PCMD3180_CHANNEL_1 >> channel);
      const uint8_t active = ((channel < plan->mic_count) &&
                              ((plan->output_channel_mask & channel_bit) != 0U)) ? 1U : 0U;
      uint16_t color = GRAY;
      uint8_t healthy = 0U;
      uint16_t level = 0U;

      if (device->present == 0U)
      {
        snprintf(label, sizeof(label), "M%02u IO", mic_id);
        color = RED;
      }
      else if (device->config_status != PCMD3180_OK)
      {
        snprintf(label, sizeof(label), "M%02u CFG", mic_id);
        color = RED;
      }
      else if (device->status_status != PCMD3180_OK)
      {
        snprintf(label, sizeof(label), "M%02u ST", mic_id);
        color = RED;
      }
      else if (active == 0U)
      {
        snprintf(label, sizeof(label), "M%02u ----", mic_id);
      }
      else
      {
        const uint32_t slot = (uint32_t)plan->start_slot + channel;
        level = ((plan->tdm_bus < APP_PCMD_BUS_COUNT) &&
                 (slot < APP_PCMD_MAX_SLOTS)) ?
                g_pcmd_slot_level[plan->tdm_bus][slot] : 0U;
        healthy = 1U;

        if (level >= 0x0400U)
        {
          color = GREEN;
        }
        else if (level != 0U)
        {
          color = YELLOW;
        }

        if (level == 0U)
        {
          snprintf(label, sizeof(label), "M%02u --dB", mic_id);
        }
        else
        {
          snprintf(label, sizeof(label), "M%02u %ddB",
                   mic_id,
                   (int)App_PCMD_LevelToDbfs(level));
        }
      }

      rgblcd_show_string(x, row_y, text_w, 12, 12, label, color);
      App_PCMD_DrawLevelBar(bar_x, bar_y, level, active, healthy);
    }
  }
}

static void App_PCMD_ShowDebugPage(void)
{
  const PCMD3180_HAL_BusContextTypeDef *bus_context = App_PCMD_BusGetContext();
  char line[128];
  uint16_t y = 18;

  rgblcd_clear(BLACK);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 24, 24,
                     "NECCS N647 PCMD3180 DEBUG", CYAN);
  y += 34U;

#if (APP_PCMD_SDOUT_BCLK_MARGIN_FIX != 0U)
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12,
                     "SDOUT edge fixed: BCLK_POL=1 SAI_RX=RISING Fs unchanged",
                     YELLOW);
  y += 16U;
#endif

  snprintf(line,
           sizeof(line),
           "Mode:%s Auto:%s Fs:%lu Slot:%u BCLK:%lu",
           App_PCMD_ModeName(g_pcmd_debug.mode),
           (APP_PCMD_AUTO_MODE_SWITCH != 0U) ? "ON" : "OFF",
           (unsigned long)g_pcmd_debug.mode_config.sample_rate_hz,
           g_pcmd_debug.mode_config.tdm_slots_per_bus,
           (unsigned long)g_pcmd_debug.mode_config.expected_bclk_hz);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 16, 16, line, WHITE);
  y += 22U;

  snprintf(line,
           sizeof(line),
           "LCD:%04X RAM:%lu Cfg:%s SAI:%ld ReCfg:%lu",
           rgblcddev.id,
           (unsigned long)g_app_pcmd_ram_test_ok,
           App_PCMD_StatusName(g_pcmd_debug.mode_config_status),
           (long)g_pcmd_debug.sai_status,
           (unsigned long)g_pcmd_debug.reconfigure_count);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 16, 16, line, WHITE);
  y += 20U;

  snprintf(line,
           sizeof(line),
           "DMA A:%ld B:%ld",
           (long)g_pcmd_debug.dma_a_status,
           (long)g_pcmd_debug.dma_b_status);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 16, 16, line, WHITE);
  y += 20U;

  snprintf(line,
           sizeof(line),
           "Rate full/s A:%lu B:%lu expected:%lu",
           (unsigned long)g_pcmd_sai_a_full_rate,
           (unsigned long)g_pcmd_sai_b_full_rate,
           (unsigned long)((g_pcmd_debug.mode_config.sample_rate_hz *
                            g_pcmd_debug.mode_config.tdm_slots_per_bus +
                            (APP_PCMD_DMA_WORDS / 2U)) /
                           APP_PCMD_DMA_WORDS));
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, CYAN);
  y += 20U;

  snprintf(line,
           sizeof(line),
           "PLL2:%lu SAIclk:%lu Mdiv:%lu NODIV:%lu",
           (unsigned long)HAL_RCCEx_GetPLL2CLKFreq(),
           (unsigned long)HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI1),
           (unsigned long)hsai_BlockA1.Init.Mckdiv,
           (unsigned long)((hsai_BlockA1.Instance->CR1 & SAI_xCR1_NODIV) != 0U));
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, CYAN);
  y += 20U;

  uint32_t sai_cr1 = hsai_BlockA1.Instance->CR1;
  uint32_t sai_frcr = hsai_BlockA1.Instance->FRCR;
  uint32_t sai_slotr = hsai_BlockA1.Instance->SLOTR;
  snprintf(line,
           sizeof(line),
           "SAIA cr1:%08lX fr:%08lX sl:%08lX",
           (unsigned long)sai_cr1,
           (unsigned long)sai_frcr,
           (unsigned long)sai_slotr);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, CYAN);
  y += 16U;

  snprintf(line,
           sizeof(line),
           "DEC nodiv:%lu mck:%lu frl:%lu slots:%lu en:%04lX",
           (unsigned long)((sai_cr1 & SAI_xCR1_NODIV_Msk) >> SAI_xCR1_NODIV_Pos),
           (unsigned long)((sai_cr1 & SAI_xCR1_MCKDIV_Msk) >> SAI_xCR1_MCKDIV_Pos),
           (unsigned long)(((sai_frcr & SAI_xFRCR_FRL_Msk) >> SAI_xFRCR_FRL_Pos) + 1U),
           (unsigned long)(((sai_slotr & SAI_xSLOTR_NBSLOT_Msk) >> SAI_xSLOTR_NBSLOT_Pos) + 1U),
           (unsigned long)((sai_slotr & SAI_xSLOTR_SLOTEN_Msk) >> SAI_xSLOTR_SLOTEN_Pos));
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, CYAN);
  y += 16U;

  snprintf(line,
           sizeof(line),
           "PDMCLK exp:%luHz  active cfg, pwr after clocks",
           (unsigned long)(g_pcmd_debug.mode_config.sample_rate_hz * 64U));
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, CYAN);
  y += 16U;

  snprintf(line,
           sizeof(line),
           "EADDR/%u 4C:%u 4D:%u 4E:%u 4F:%u BUS:%u%u %s",
           g_pcmd_debug.early_address_scan_rounds,
           g_pcmd_debug.early_address_ack_count[0],
           g_pcmd_debug.early_address_ack_count[1],
           g_pcmd_debug.early_address_ack_count[2],
           g_pcmd_debug.early_address_ack_count[3],
           g_pcmd_debug.early_scl_idle_high,
           g_pcmd_debug.early_sda_idle_high,
           (g_pcmd_debug.early_address_scan_ok != 0U) ? "OK" : "BAD");
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line,
                     (g_pcmd_debug.early_address_scan_ok != 0U) ? GREEN : RED);
  y += 16U;

  snprintf(line,
           sizeof(line),
           "ADDR/%u 4C:%u 4D:%u 4E:%u 4F:%u BUS:%u%u %s",
           g_pcmd_debug.address_scan_rounds,
           g_pcmd_debug.address_ack_count[0],
           g_pcmd_debug.address_ack_count[1],
           g_pcmd_debug.address_ack_count[2],
           g_pcmd_debug.address_ack_count[3],
           g_pcmd_debug.scl_idle_high,
           g_pcmd_debug.sda_idle_high,
           (g_pcmd_debug.address_scan_ok != 0U) ? "OK" : "BAD");
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line,
                     (g_pcmd_debug.address_scan_ok != 0U) ? GREEN : RED);
  y += 16U;

  uint32_t failed_device = PCMD3180_ARRAY_DEVICE_COUNT;
  for (uint32_t i = 0; i < PCMD3180_ARRAY_DEVICE_COUNT; i++)
  {
    if ((g_pcmd_devices[i].present != 0U) &&
        (g_pcmd_devices[i].config_status != PCMD3180_OK))
    {
      failed_device = i;
      break;
    }
  }

  if (failed_device < PCMD3180_ARRAY_DEVICE_COUNT)
  {
    const PCMD3180_HandleTypeDef *handle = &g_pcmd_handles[failed_device];
    snprintf(line,
             sizeof(line),
             "Fail U%lu %s reg:%02X wr:%02X rd:%02X",
             (unsigned long)(failed_device + 1U),
             App_PCMD_StatusName(g_pcmd_devices[failed_device].config_status),
             handle->last_reg,
             handle->last_write_value,
             handle->last_read_value);
  }
  else
  {
    snprintf(line, sizeof(line), "Fail none");
  }
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line,
                     (failed_device < PCMD3180_ARRAY_DEVICE_COUNT) ? YELLOW : GREEN);
  y += 16U;

  snprintf(line,
           sizeof(line),
           "I2C:%s rec:%lu %c %02X:%02X val:%02X h:%lu e:%lX",
           (bus_context->use_software_i2c != 0U) ? "SW" : "HW",
           (unsigned long)bus_context->recover_count,
           (bus_context->last_is_read != 0U) ? 'R' : 'W',
           bus_context->last_address7,
           bus_context->last_reg,
           bus_context->last_value,
           (unsigned long)bus_context->last_hal_status,
           (unsigned long)bus_context->last_hal_error);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, CYAN);
  y += 20U;

  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12,
                     "Dev Adr P C S SL PWR ASI D0/D1 PD PI GPOs GI EN/AO",
                     YELLOW);
  y += 16U;

  for (uint32_t i = 0; i < PCMD3180_ARRAY_DEVICE_COUNT; i++)
  {
    const AppPcmdDeviceState_t *device = &g_pcmd_devices[i];
    const uint8_t routing_ok = App_PCMD_IsRoutingSnapshotOk(i, &device->snapshot);
    const uint16_t color = (device->present != 0U) ?
                           (((device->config_status == PCMD3180_OK) &&
                             (device->status_status == PCMD3180_OK)) ?
                            ((routing_ok != 0U) ? GREEN : RED) : RED) :
                           RED;

    snprintf(line,
             sizeof(line),
             "U%lu %02X %-2s %-2s %-2s %02X-%02X P%02X A%02X D%02X/%02X PD%02X PI%02X G%02X%02X%02X%02X GI%02X/%02X E%02X/O%02X",
             (unsigned long)(i + 1U),
             (unsigned int)(PCMD3180_I2C_ADDR_0 + i),
             App_PCMD_StatusName(device->probe_status),
             App_PCMD_StatusName(device->config_status),
             App_PCMD_StatusName(device->status_status),
             device->snapshot.asi_ch_slot[0],
             device->snapshot.asi_ch_slot[PCMD3180_ARRAY_MAX_MICS_PER_DEV - 1U],
             device->snapshot.pwr_cfg,
             device->snapshot.asi_sts,
             device->snapshot.dev_sts0,
             device->snapshot.dev_sts1,
             device->snapshot.pdmclk_cfg,
             device->snapshot.pdmin_cfg,
             device->snapshot.gpo_cfg0,
             device->snapshot.gpo_cfg1,
             device->snapshot.gpo_cfg2,
             device->snapshot.gpo_cfg3,
             device->snapshot.gpi_cfg0,
             device->snapshot.gpi_cfg1,
             device->snapshot.in_ch_en,
             device->snapshot.asi_out_ch_en);
    rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, color);
    y += 16U;
  }

  y += 8U;
  snprintf(line,
           sizeof(line),
            "SAI A h=%lu f=%lu e=%lu last=%s(0x%lX) d=%lX %s",
            (unsigned long)g_pcmd_sai_a_half_count,
            (unsigned long)g_pcmd_sai_a_full_count,
            (unsigned long)g_pcmd_sai_a_error_count,
            App_SAI_ErrorName(g_pcmd_sai_a_last_error),
            (unsigned long)g_pcmd_sai_a_last_error,
            (unsigned long)HAL_DMA_GetError(&handle_GPDMA1_Channel0),
            App_SAI_StateName(HAL_SAI_GetState(&hsai_BlockA1)));
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, WHITE);
  y += 16U;

  snprintf(line,
           sizeof(line),
            "SAI B h=%lu f=%lu e=%lu last=%s(0x%lX) d=%lX %s",
            (unsigned long)g_pcmd_sai_b_half_count,
            (unsigned long)g_pcmd_sai_b_full_count,
            (unsigned long)g_pcmd_sai_b_error_count,
            App_SAI_ErrorName(g_pcmd_sai_b_last_error),
            (unsigned long)g_pcmd_sai_b_last_error,
            (unsigned long)HAL_DMA_GetError(&handle_GPDMA1_Channel1),
            App_SAI_StateName(HAL_SAI_GetState(&hsai_BlockB1)));
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, WHITE);
  y += 20U;

  snprintf(line,
           sizeof(line),
           "AC removes DC. DC A0:%d A8:%d B0:%d B8:%d",
           (int)g_pcmd_slot_dc_level[PCMD3180_TDM_BUS_A][0],
           (int)g_pcmd_slot_dc_level[PCMD3180_TDM_BUS_A][8],
           (int)g_pcmd_slot_dc_level[PCMD3180_TDM_BUS_B][0],
           (int)g_pcmd_slot_dc_level[PCMD3180_TDM_BUS_B][8]);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, CYAN);
  y += 16U;

  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12,
                      "Expected: U1-U4 OK; raw hex changes with input.",
                      LGRAY);
  y += 20U;
  App_PCMD_ShowMicActivity(y);
}
void App_PCMD_DebugSetRamStatus(uint32_t init_ok, uint32_t test_ok)
{
  (void)init_ok;
  g_app_pcmd_ram_test_ok = test_ok;
}

void App_PCMD_DebugBusInit(I2C_HandleTypeDef *hi2c)
{
  App_PCMD_BusInit(hi2c,
                   GPIOD,
                   GPIO_PIN_14,
                   GPIOD,
                   GPIO_PIN_4,
                   MIC_SHDNZ_GPIO_Port,
                   MIC_SHDNZ_Pin,
                   APP_PCMD_I2C_TIMEOUT_MS);
}

void App_PCMD_DebugStartDma(void)
{
  App_PCMD_StartDma();
}

void App_PCMD_DebugInitAfterSaiClock(void)
{
  HAL_Delay(APP_PCMD_CLOCK_SETTLE_MS);
  App_PCMD_DebugInit();
  g_pcmd_last_ui_tick = HAL_GetTick() - APP_PCMD_UI_REFRESH_MS;
}

void App_PCMD_DebugTask(void)
{
  App_PCMD_Task();
}

uint8_t App_PCMD_DebugRenderTask(void)
{
  const uint32_t now = HAL_GetTick();

  if ((now - g_pcmd_last_ui_tick) < APP_PCMD_UI_REFRESH_MS)
  {
    return 0U;
  }

  g_pcmd_last_ui_tick = now;
  App_PCMD_UpdateLevelsFromAccumulator();
  App_PCMD_ShowDebugPage();

  return 1U;
}
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    g_pcmd_sai_a_half_count++;
    App_PCMD_ProcessBuffer(g_pcmd_sai_a_rx,
                           APP_PCMD_DMA_WORDS / 2U,
                           g_pcmd_debug.mode_config.tdm_slots_per_bus,
                           g_pcmd_slot_abs_sum[PCMD3180_TDM_BUS_A],
                           g_pcmd_slot_dc_sum[PCMD3180_TDM_BUS_A],
                           g_pcmd_slot_abs_count[PCMD3180_TDM_BUS_A],
                           g_pcmd_slot_last_sample[PCMD3180_TDM_BUS_A]);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    g_pcmd_sai_b_half_count++;
    App_PCMD_ProcessBuffer(g_pcmd_sai_b_rx,
                           APP_PCMD_DMA_WORDS / 2U,
                           g_pcmd_debug.mode_config.tdm_slots_per_bus,
                           g_pcmd_slot_abs_sum[PCMD3180_TDM_BUS_B],
                           g_pcmd_slot_dc_sum[PCMD3180_TDM_BUS_B],
                           g_pcmd_slot_abs_count[PCMD3180_TDM_BUS_B],
                           g_pcmd_slot_last_sample[PCMD3180_TDM_BUS_B]);
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    g_pcmd_sai_a_full_count++;
    App_PCMD_ProcessBuffer(&g_pcmd_sai_a_rx[APP_PCMD_DMA_WORDS / 2U],
                           APP_PCMD_DMA_WORDS / 2U,
                           g_pcmd_debug.mode_config.tdm_slots_per_bus,
                           g_pcmd_slot_abs_sum[PCMD3180_TDM_BUS_A],
                           g_pcmd_slot_dc_sum[PCMD3180_TDM_BUS_A],
                           g_pcmd_slot_abs_count[PCMD3180_TDM_BUS_A],
                           g_pcmd_slot_last_sample[PCMD3180_TDM_BUS_A]);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    g_pcmd_sai_b_full_count++;
    App_PCMD_ProcessBuffer(&g_pcmd_sai_b_rx[APP_PCMD_DMA_WORDS / 2U],
                           APP_PCMD_DMA_WORDS / 2U,
                           g_pcmd_debug.mode_config.tdm_slots_per_bus,
                           g_pcmd_slot_abs_sum[PCMD3180_TDM_BUS_B],
                           g_pcmd_slot_dc_sum[PCMD3180_TDM_BUS_B],
                           g_pcmd_slot_abs_count[PCMD3180_TDM_BUS_B],
                           g_pcmd_slot_last_sample[PCMD3180_TDM_BUS_B]);
  }
}

void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    g_pcmd_sai_a_error_count++;
    g_pcmd_sai_a_last_error = HAL_SAI_GetError(hsai);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    g_pcmd_sai_b_error_count++;
    g_pcmd_sai_b_last_error = HAL_SAI_GetError(hsai);
  }
}
