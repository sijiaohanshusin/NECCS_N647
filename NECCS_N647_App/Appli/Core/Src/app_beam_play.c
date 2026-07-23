/*
 * I2S2 -> MAX98357A speaker path for the directional-recording feature.
 * See app_beam_play.h for the wiring and producer model.
 *
 * Clocking: PLL2 VCO (245.76 MHz) -> IC8 /10 -> 24.576 MHz i2s_ker_ck ->
 * I2SDIV=8 -> BCLK 1.536 MHz -> exactly 48 kHz x 32-bit stereo frames.
 * Same VCO as the SAI1 mic clock (IC7), so playback is sample-locked to
 * capture: the monitor ring level cannot drift, no resampling needed.
 * IC7 itself is NOT touched here - the capture chain stays untouched.
 *
 * DMA: GPDMA1_Channel2 (0/1 belong to the mic array), single circular
 * linked-list node over a 2-half buffer in the .noncacheable section
 * (same pattern as the SAI RX buffers - no cache maintenance in the ISR).
 * Half/full-complete ISRs refill from a lock-free SPSC ring; shortfall is
 * zero-padded so an underrun clicks to silence instead of looping stale
 * audio. Mono samples are duplicated into L/R slots (MAX98357 with SD
 * floating outputs (L+R)/2, which folds back to the original signal).
 */

#include "app_beam_play.h"

#include "main.h"

#include "tx_api.h"

#include <string.h>

/* ------------------------------------------------------------------ */
/* Sizing                                                              */
/* ------------------------------------------------------------------ */

/* 256 stereo frames per half = 5.33 ms per service, comfortably slower
 * than the pcmd ISR cadence. Whole buffer = 2 halves. */
#define APP_BEAM_PLAY_HALF_FRAMES   256U
#define APP_BEAM_PLAY_BUF_SAMPLES   (APP_BEAM_PLAY_HALF_FRAMES * 2U * 2U)

/* Mono ring: 16384 samples = 341 ms. WAV playback must ride the media
 * thread's 200 ms tick; the live monitor only ever holds its ~512-sample
 * jitter cushion, so the size costs latency ONLY for the WAV path. */
#define APP_BEAM_PLAY_RING_SAMPLES  16384U
#define APP_BEAM_PLAY_RING_MASK     (APP_BEAM_PLAY_RING_SAMPLES - 1U)

/* Monitor jitter cushion (~10.7 ms): absorbs capture-frame burstiness
 * without adding audible lag. Producer and consumer share PLL2, so the
 * level parks here forever once steady. */
#define APP_BEAM_PLAY_MONITOR_PREFILL 512U

#define APP_BEAM_PLAY_IRQ_PRIORITY  11U

/* ------------------------------------------------------------------ */
/* State                                                               */
/* ------------------------------------------------------------------ */

I2S_HandleTypeDef g_hi2s2;
DMA_HandleTypeDef g_hdma_i2s2_tx;
static DMA_NodeTypeDef s_i2s2_tx_node __NON_CACHEABLE;
static DMA_QListTypeDef s_i2s2_tx_list;

/* DMA transfer buffer: interleaved L/R int16. Non-cacheable like the SAI
 * RX buffers - the ISR writes it, the GPDMA reads it, nobody flushes. */
static int16_t s_tx_buf[APP_BEAM_PLAY_BUF_SAMPLES]
    __attribute__((section(".noncacheable"), aligned(32)));

/* Mono playback ring. External RAM: CPU-only on both sides (capture or
 * media thread writes, the TX ISR reads), so cache coherency is a
 * non-issue and internal RAM (fully booked) stays untouched. */
static int16_t s_ring[APP_BEAM_PLAY_RING_SAMPLES]
    __attribute__((section(".EXTRAM"), aligned(32)));
static volatile uint32_t s_ring_wr;
static volatile uint32_t s_ring_rd;

static volatile uint8_t s_initialized;
static volatile uint8_t s_monitor_on;   /* UI switch                     */
static volatile uint8_t s_wav_active;   /* media thread owns the sink    */
static volatile uint8_t s_streaming;    /* I2S DMA running               */
static volatile uint32_t s_underruns;
static volatile uint32_t s_halves_out;
static volatile int32_t s_last_status;

/* Serializes monitor/WAV state flips + stream start/stop (UI thread vs
 * media thread). Never taken in ISRs. */
static TX_MUTEX s_lock;
static uint8_t s_lock_ready;

/* ------------------------------------------------------------------ */
/* MSP: clocks / pins / DMA / IRQs (module-local, like the USB PCD)    */
/* ------------------------------------------------------------------ */

void HAL_I2S_MspInit(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->Instance == SPI2)
  {
    GPIO_InitTypeDef gpio = {0};
    RCC_PeriphCLKInitTypeDef periph_clk = {0};
    DMA_NodeConfTypeDef node_config = {0};

    /* Kernel clock: PLL2 (already running for SAI1) via IC8 /10. */
    periph_clk.PeriphClockSelection = RCC_PERIPHCLK_SPI2;
    periph_clk.Spi2ClockSelection = RCC_SPI2CLKSOURCE_IC8;
    periph_clk.ICSelection[RCC_IC8].ClockSelection = RCC_ICCLKSOURCE_PLL2;
    periph_clk.ICSelection[RCC_IC8].ClockDivider = 10;
    if (HAL_RCCEx_PeriphCLKConfig(&periph_clk) != HAL_OK)
    {
      s_last_status = -10;
      return;
    }

    __HAL_RCC_SPI2_CLK_ENABLE();

    /* PF2=CK, PC1=WS, PG8=SDO, all AF5. MEDIUM speed: 1.5 MHz signals
     * over DuPont jumpers want slow edges, not VERY_HIGH ringing. */
    __HAL_RCC_GPIOF_CLK_ENABLE();
    gpio.Pin = GPIO_PIN_2;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_MEDIUM;
    gpio.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOF, &gpio);

    __HAL_RCC_GPIOC_CLK_ENABLE();
    gpio.Pin = GPIO_PIN_1;
    HAL_GPIO_Init(GPIOC, &gpio);

    __HAL_RCC_GPIOG_CLK_ENABLE();
    gpio.Pin = GPIO_PIN_8;
    HAL_GPIO_Init(GPIOG, &gpio);

    /* Circular single-node TX list on GPDMA1_Channel2 (0/1 = mic array). */
    node_config.NodeType = DMA_GPDMA_LINEAR_NODE;
    node_config.Init.Request = GPDMA1_REQUEST_SPI2_TX;
    node_config.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
    node_config.Init.Direction = DMA_MEMORY_TO_PERIPH;
    node_config.Init.SrcInc = DMA_SINC_INCREMENTED;
    node_config.Init.DestInc = DMA_DINC_FIXED;
    node_config.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_HALFWORD;
    node_config.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
    node_config.Init.SrcBurstLength = 1;
    node_config.Init.DestBurstLength = 1;
    node_config.Init.TransferAllocatedPort = DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT0;
    node_config.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    node_config.Init.Mode = DMA_NORMAL;
    node_config.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
    node_config.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
    node_config.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
    node_config.SrcSecure = DMA_CHANNEL_SRC_SEC;
    node_config.DestSecure = DMA_CHANNEL_DEST_SEC;

    if (HAL_DMAEx_List_BuildNode(&node_config, &s_i2s2_tx_node) != HAL_OK)
    {
      s_last_status = -11;
      return;
    }
    if (HAL_DMAEx_List_InsertNode(&s_i2s2_tx_list, NULL, &s_i2s2_tx_node) != HAL_OK)
    {
      s_last_status = -12;
      return;
    }
    if (HAL_DMAEx_List_SetCircularMode(&s_i2s2_tx_list) != HAL_OK)
    {
      s_last_status = -13;
      return;
    }

    g_hdma_i2s2_tx.Instance = GPDMA1_Channel2;
    g_hdma_i2s2_tx.InitLinkedList.Priority = DMA_HIGH_PRIORITY;
    g_hdma_i2s2_tx.InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
    g_hdma_i2s2_tx.InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT0;
    g_hdma_i2s2_tx.InitLinkedList.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
    g_hdma_i2s2_tx.InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;
    if (HAL_DMAEx_List_Init(&g_hdma_i2s2_tx) != HAL_OK)
    {
      s_last_status = -14;
      return;
    }
    if (HAL_DMAEx_List_LinkQ(&g_hdma_i2s2_tx, &s_i2s2_tx_list) != HAL_OK)
    {
      s_last_status = -15;
      return;
    }

    /* TrustZone: the app runs secure and the node carries SRC_SEC/DEST_SEC,
     * so the CHANNEL must be secure too - a non-secure channel gets its
     * first transfer killed by the RIF (board 2026-07-22: I2S err=0x8 DMA
     * error, zero halves out). Channels 0/1 get the same treatment in
     * SystemIsolation_Config; channel 2 is ours to configure here. */
    if (HAL_DMA_ConfigChannelAttributes(&g_hdma_i2s2_tx,
                                        DMA_CHANNEL_SEC |
                                        DMA_CHANNEL_PRIV |
                                        DMA_CHANNEL_SRC_SEC |
                                        DMA_CHANNEL_DEST_SEC) != HAL_OK)
    {
      s_last_status = -16;
      return;
    }

    __HAL_LINKDMA(hi2s, hdmatx, g_hdma_i2s2_tx);

    HAL_NVIC_SetPriority(GPDMA1_Channel2_IRQn, APP_BEAM_PLAY_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel2_IRQn);
    HAL_NVIC_SetPriority(SPI2_IRQn, APP_BEAM_PLAY_IRQ_PRIORITY, 0);
    HAL_NVIC_EnableIRQ(SPI2_IRQn);
  }
}

void HAL_I2S_MspDeInit(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->Instance == SPI2)
  {
    HAL_NVIC_DisableIRQ(SPI2_IRQn);
    HAL_NVIC_DisableIRQ(GPDMA1_Channel2_IRQn);
    __HAL_RCC_SPI2_CLK_DISABLE();
  }
}

/* ------------------------------------------------------------------ */
/* ISR refill                                                          */
/* ------------------------------------------------------------------ */

static void AppBeamPlay_FillHalf(uint32_t half_index)
{
  int16_t *dst = &s_tx_buf[half_index * APP_BEAM_PLAY_HALF_FRAMES * 2U];
  const uint8_t source_active = (s_wav_active != 0U) || (s_monitor_on != 0U);
  uint32_t filled = 0U;

  while ((filled < APP_BEAM_PLAY_HALF_FRAMES) && (s_ring_rd != s_ring_wr))
  {
    const int16_t v = s_ring[s_ring_rd & APP_BEAM_PLAY_RING_MASK];

    s_ring_rd++;
    dst[filled * 2U] = v;
    dst[filled * 2U + 1U] = v;
    filled++;
  }

  if (filled < APP_BEAM_PLAY_HALF_FRAMES)
  {
    memset(&dst[filled * 2U], 0,
           (APP_BEAM_PLAY_HALF_FRAMES - filled) * 2U * sizeof(int16_t));
    if ((source_active != 0U) && (filled == 0U))
    {
      s_underruns++;
    }
  }

  s_halves_out++;
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->Instance == SPI2)
  {
    AppBeamPlay_FillHalf(0U);
  }
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->Instance == SPI2)
  {
    AppBeamPlay_FillHalf(1U);
  }
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s)
{
  if (hi2s->Instance == SPI2)
  {
    /* Observability only (UDR/DMA errors land here); the stream keeps
     * running - a TX underrun just repeats/zeroes one frame on the wire. */
    s_last_status = (int32_t)(0x1000U | hi2s->ErrorCode);
  }
}

/* ------------------------------------------------------------------ */
/* Stream control (thread context only, under s_lock)                  */
/* ------------------------------------------------------------------ */

static void AppBeamPlay_RingReset(void)
{
  uint32_t primask = __get_PRIMASK();

  __disable_irq();
  s_ring_rd = 0U;
  s_ring_wr = 0U;
  if (primask == 0U)
  {
    __enable_irq();
  }
}

static uint32_t AppBeamPlay_RingWrite(const int16_t *samples, uint32_t count)
{
  uint32_t written = 0U;

  while ((written < count) &&
         ((s_ring_wr - s_ring_rd) < APP_BEAM_PLAY_RING_SAMPLES))
  {
    s_ring[s_ring_wr & APP_BEAM_PLAY_RING_MASK] = samples[written];
    s_ring_wr++;
    written++;
  }

  return written;
}

static int32_t AppBeamPlay_InitLocked(void)
{
  if (s_initialized != 0U)
  {
    return 0;
  }

  s_last_status = 0;
  memset(&g_hi2s2, 0, sizeof(g_hi2s2));
  g_hi2s2.Instance = SPI2;
  g_hi2s2.Init.Mode = I2S_MODE_MASTER_TX;
  g_hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  g_hi2s2.Init.DataFormat = I2S_DATAFORMAT_16B;
  g_hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  g_hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_48K;
  g_hi2s2.Init.CPOL = I2S_CPOL_LOW;
  g_hi2s2.Init.FirstBit = I2S_FIRSTBIT_MSB;
  g_hi2s2.Init.WSInversion = I2S_WS_INVERSION_DISABLE;
  g_hi2s2.Init.Data24BitAlignment = I2S_DATA_24BIT_ALIGNMENT_RIGHT;
  g_hi2s2.Init.MasterKeepIOState = I2S_MASTER_KEEP_IO_STATE_ENABLE;

  if (HAL_I2S_Init(&g_hi2s2) != HAL_OK)
  {
    if (s_last_status == 0)
    {
      s_last_status = -1;
    }
    return s_last_status;
  }
  if (s_last_status != 0)
  {
    return s_last_status; /* MspInit failed mid-way */
  }

  s_initialized = 1U;
  return 0;
}

static int32_t AppBeamPlay_StreamStart(void)
{
  if (s_streaming != 0U)
  {
    return 0;
  }

  memset(s_tx_buf, 0, sizeof(s_tx_buf));
  s_halves_out = 0U;

  if (HAL_I2S_Transmit_DMA(&g_hi2s2, (const uint16_t *)s_tx_buf,
                           APP_BEAM_PLAY_BUF_SAMPLES) != HAL_OK)
  {
    s_last_status = -2;
    return -2;
  }

  s_streaming = 1U;
  return 0;
}

static void AppBeamPlay_StreamStopIfIdle(void)
{
  if ((s_streaming == 0U) ||
      (s_wav_active != 0U) ||
      (s_monitor_on != 0U))
  {
    return;
  }

  (void)HAL_I2S_DMAStop(&g_hi2s2);
  s_streaming = 0U;
  AppBeamPlay_RingReset();
}

static void AppBeamPlay_Lock(void)
{
  if (s_lock_ready == 0U)
  {
    /* First call happens before any concurrency (UI or media thread). */
    (void)tx_mutex_create(&s_lock, (CHAR *)"beam_play", TX_INHERIT);
    s_lock_ready = 1U;
  }
  (void)tx_mutex_get(&s_lock, TX_WAIT_FOREVER);
}

static void AppBeamPlay_Unlock(void)
{
  (void)tx_mutex_put(&s_lock);
}

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

int32_t AppBeamPlay_SetMonitor(uint8_t on)
{
  int32_t status = 0;

  AppBeamPlay_Lock();

  if (on != 0U)
  {
    status = AppBeamPlay_InitLocked();
    if (status == 0)
    {
      if ((s_monitor_on == 0U) && (s_wav_active == 0U))
      {
        /* Fresh monitor session: silence cushion sets the latency. */
        AppBeamPlay_RingReset();
        {
          static const int16_t zeros[64] = {0};
          uint32_t seeded = 0U;

          while (seeded < APP_BEAM_PLAY_MONITOR_PREFILL)
          {
            seeded += AppBeamPlay_RingWrite(zeros, 64U);
          }
        }
      }
      s_monitor_on = 1U;
      status = AppBeamPlay_StreamStart();
    }
  }
  else
  {
    s_monitor_on = 0U;
    AppBeamPlay_StreamStopIfIdle();
  }

  AppBeamPlay_Unlock();
  return status;
}

uint8_t AppBeamPlay_GetMonitor(void)
{
  return s_monitor_on;
}

void AppBeamPlay_MonitorFeed(const int16_t *samples, uint32_t count)
{
  /* Lock-free on purpose: capture thread must never block on UI/media
   * transitions. Worst case during a WAV takeover is one stale frame. */
  if ((s_monitor_on == 0U) ||
      (s_wav_active != 0U) ||
      (s_streaming == 0U) ||
      (samples == NULL))
  {
    return;
  }

  (void)AppBeamPlay_RingWrite(samples, count);
}

int32_t AppBeamPlay_WavStart(void)
{
  int32_t status;

  AppBeamPlay_Lock();

  status = AppBeamPlay_InitLocked();
  if (status == 0)
  {
    s_wav_active = 1U;      /* mutes the monitor tap immediately */
    AppBeamPlay_RingReset();
    s_underruns = 0U;
    status = AppBeamPlay_StreamStart();
    if (status != 0)
    {
      s_wav_active = 0U;
    }
  }

  AppBeamPlay_Unlock();
  return status;
}

void AppBeamPlay_WavStop(void)
{
  AppBeamPlay_Lock();

  s_wav_active = 0U;
  if (s_monitor_on != 0U)
  {
    /* Hand the sink back to the live monitor: drop leftover WAV audio. */
    AppBeamPlay_RingReset();
  }
  else
  {
    AppBeamPlay_StreamStopIfIdle();
  }

  AppBeamPlay_Unlock();
}

uint32_t AppBeamPlay_WavWrite(const int16_t *samples, uint32_t count)
{
  if ((s_wav_active == 0U) || (samples == NULL))
  {
    return 0U;
  }

  return AppBeamPlay_RingWrite(samples, count);
}

uint32_t AppBeamPlay_WavFreeSpace(void)
{
  return APP_BEAM_PLAY_RING_SAMPLES - (s_ring_wr - s_ring_rd);
}

uint8_t AppBeamPlay_WavDrained(void)
{
  return (s_ring_rd == s_ring_wr) ? 1U : 0U;
}

void AppBeamPlay_GetSnapshot(AppBeamPlaySnapshot_t *snapshot)
{
  if (snapshot == NULL)
  {
    return;
  }

  snapshot->initialized = s_initialized;
  snapshot->monitor_on = s_monitor_on;
  snapshot->wav_active = s_wav_active;
  snapshot->streaming = s_streaming;
  snapshot->ring_level = s_ring_wr - s_ring_rd;
  snapshot->underruns = s_underruns;
  snapshot->halves_out = s_halves_out;
  snapshot->last_status = s_last_status;
}
