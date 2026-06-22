/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#ifdef DEBUG
#include "./SYS/sys.h"
#include "./HyperRAM/hyperram.h"
#endif
#include "./LED/led.h"
#include "./PCMD3180/pcmd3180.h"
#include "./PCMD3180/pcmd3180_hal.h"
#include "./RGBLCD/rgblcd.h"
#include <stdio.h>
#include <string.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
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

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define APP_HYPERRAM_BASE        0x90000000UL
#define APP_HYPERRAM_TEST_BYTES  0x1000UL
#define APP_PCMD_MODE_STEP_MS    7000U
#define APP_PCMD_POLL_MS         0U
#define APP_PCMD_UI_REFRESH_MS   1000U
#define APP_PCMD_MIC_PAGE_MS     3000U
#define APP_PCMD_DMA_WORDS       4096U
#define APP_PCMD_I2C_TIMEOUT_MS  100U
#define APP_PCMD_MAX_SLOTS       16U
#define APP_PCMD_BUS_COUNT       2U
#define APP_PCMD_IRQ_PRIO        5U
#define APP_PCMD_DEFAULT_MODE    PCMD3180_ARRAY_MODE_32CH_48K
#define APP_PCMD_AUTO_MODE_SWITCH 0U
#define APP_PCMD_RESET_LOW_MS    100U
#define APP_PCMD_RESET_SETTLE_MS 10U
#define APP_PCMD_ADDR_SCAN_ROUNDS 16U
#define APP_I2C2_TIMING_25KHZ_64MHZ 0x702177C7U
#define APP_PCMD_UI_X            8U
#define APP_PCMD_UI_W            584U
#define APP_PCMD_SAI1_IC_DIVIDER_48K  20U
#define APP_PCMD_SAI1_IC_DIVIDER_192K 10U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

DMA2D_HandleTypeDef hdma2d;

I2C_HandleTypeDef hi2c2;

LTDC_HandleTypeDef hltdc;

SAI_HandleTypeDef hsai_BlockA1;
SAI_HandleTypeDef hsai_BlockB1;

XSPI_HandleTypeDef hxspi1;

DMA_HandleTypeDef handle_GPDMA1_Channel0;
DMA_HandleTypeDef handle_GPDMA1_Channel1;

/* USER CODE BEGIN PV */
#ifdef DEBUG
static HyperRAM_ObjectTypeDef HyperRAMObject = {0};
#endif
static volatile uint32_t g_app_hyperram_init_ok = 0;
static volatile uint32_t g_app_hyperram_test_ok = 0;
static volatile uint32_t g_app_hyperram_error_addr = 0;
static volatile uint32_t g_app_hyperram_expected = 0;
static volatile uint32_t g_app_hyperram_actual = 0;
volatile uint32_t g_app_tick_recover_vtor = 0;
volatile uint32_t g_app_tick_recover_ctrl = 0;
volatile uint32_t g_app_tick_recover_load = 0;
volatile uint32_t g_app_tick_recover_primask = 0;
static uint16_t g_pcmd_sai_a_rx[APP_PCMD_DMA_WORDS]
    __attribute__((section(".noncacheable"), aligned(32)));
static uint16_t g_pcmd_sai_b_rx[APP_PCMD_DMA_WORDS]
    __attribute__((section(".noncacheable"), aligned(32)));
static DMA_NodeTypeDef g_pcmd_sai_a_dma_node
    __attribute__((section(".noncacheable"), aligned(32)));
static DMA_NodeTypeDef g_pcmd_sai_b_dma_node
    __attribute__((section(".noncacheable"), aligned(32)));
static DMA_QListTypeDef g_pcmd_sai_a_dma_queue
    __attribute__((section(".noncacheable"), aligned(32)));
static DMA_QListTypeDef g_pcmd_sai_b_dma_queue
    __attribute__((section(".noncacheable"), aligned(32)));
static PCMD3180_BusTypeDef g_pcmd_bus;
static PCMD3180_HAL_BusContextTypeDef g_pcmd_bus_context;
static PCMD3180_HandleTypeDef g_pcmd_handles[PCMD3180_ARRAY_DEVICE_COUNT];
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
static uint32_t g_pcmd_sai_pll2_configured = 0;
static volatile uint16_t g_pcmd_slot_peak[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
static volatile uint16_t g_pcmd_slot_last_sample[APP_PCMD_BUS_COUNT][APP_PCMD_MAX_SLOTS];
static uint32_t g_pcmd_last_ui_tick = 0;
static uint32_t g_sai1_client = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static void MX_GPIO_Init(void);
static void MX_DMA2D_Init(void);
static void MX_I2C2_Init(void);
static void MX_LTDC_Init(void);
static void MX_SAI1_Init(void);
static void SystemIsolation_Config(void);
/* USER CODE BEGIN PFP */
static uint32_t App_HyperRAM_SelfTest(void);
static void App_MPU_ConfigNonCacheable(void);
static void App_RecoverHalTick(void);
static void App_PCMD_DebugInit(void);
static void App_PCMD_Task(void);
static void App_PCMD_ShowDebugPage(void);
static void App_PCMD_ConfigureMode(PCMD3180_ArrayModeTypeDef mode);
static void App_PCMD_PollStatus(void);
static void App_PCMD_StartDma(void);
static void App_PCMD_StopDma(void);
static void App_PCMD_DecayLevels(void);
static void App_PCMD_ShowMicActivity(uint16_t start_y);
static void App_PCMD_PrepareBus(uint8_t reset_device);
static void App_PCMD_EarlyI2CTest(void);
static uint8_t App_PCMD_RunAddressScan(uint8_t *ack_count,
                                       uint8_t *scan_rounds,
                                       uint8_t *scl_idle_high,
                                       uint8_t *sda_idle_high);
static uint8_t App_PCMD_CheckAddressMap(void);
static const char *App_SAI_StateName(uint32_t state);
static const char *App_SAI_ErrorName(uint32_t error_code);
static HAL_StatusTypeDef App_PCMD_ConfigSaiPll2(void);
static HAL_StatusTypeDef App_SAI_ApplyModeConfig(const PCMD3180_ArrayModeConfigTypeDef *mode_config);
static HAL_StatusTypeDef App_PCMD_InitSaiDma(DMA_HandleTypeDef *dma_handle,
                                              DMA_Channel_TypeDef *dma_instance,
                                              uint32_t request,
                                              DMA_NodeTypeDef *dma_node,
                                              DMA_QListTypeDef *dma_queue);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
extern const uint32_t g_pfnVectors[];
extern uint32_t __snoncacheable;
extern uint32_t __enoncacheable;

static void App_MPU_ConfigNonCacheable(void)
{
  const uint32_t primask = __get_PRIMASK();
  MPU_Attributes_InitTypeDef attributes = {0};
  MPU_Region_InitTypeDef region = {0};

  __disable_irq();
  HAL_MPU_Disable();

  attributes.Number = MPU_ATTRIBUTES_NUMBER7;
  attributes.Attributes = INNER_OUTER(MPU_NOT_CACHEABLE);
  HAL_MPU_ConfigMemoryAttributes(&attributes);

  region.Enable = MPU_REGION_ENABLE;
  region.Number = MPU_REGION_NUMBER7;
  region.AttributesIndex = MPU_ATTRIBUTES_NUMBER7;
  region.BaseAddress = (uint32_t)&__snoncacheable;
  region.LimitAddress = (uint32_t)&__enoncacheable - 1U;
  region.AccessPermission = MPU_REGION_ALL_RW;
  region.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  region.DisablePrivExec = MPU_PRIV_INSTRUCTION_ACCESS_DISABLE;
  region.IsShareable = MPU_ACCESS_INNER_SHAREABLE | MPU_ACCESS_OUTER_SHAREABLE;
  HAL_MPU_ConfigRegion(&region);

  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
  __set_PRIMASK(primask);
}

static void App_RecoverHalTick(void)
{
  SCB->VTOR = (uint32_t)g_pfnVectors;
  __DSB();
  __ISB();

  if (HAL_InitTick(TICK_INT_PRIORITY) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_ResumeTick();
  __enable_irq();

  g_app_tick_recover_vtor = SCB->VTOR;
  g_app_tick_recover_ctrl = SysTick->CTRL;
  g_app_tick_recover_load = SysTick->LOAD;
  g_app_tick_recover_primask = __get_PRIMASK();
}

static uint32_t App_HyperRAM_SelfTest(void)
{
  volatile uint32_t *ram = (volatile uint32_t *)APP_HYPERRAM_BASE;
  const uint32_t words = APP_HYPERRAM_TEST_BYTES / sizeof(uint32_t);
  const uint32_t patterns[] =
  {
    0x00000000U,
    0xFFFFFFFFU,
    0xA5A5A5A5U,
    0x5A5A5A5AU,
  };

  for (uint32_t p = 0; p < (sizeof(patterns) / sizeof(patterns[0])); p++)
  {
    for (uint32_t i = 0; i < words; i++)
    {
      ram[i] = patterns[p] ^ (i * 0x01010101U);
    }

    SCB_CleanInvalidateDCache_by_Addr((void *)APP_HYPERRAM_BASE, APP_HYPERRAM_TEST_BYTES);
    __DSB();
    __ISB();

    for (uint32_t i = 0; i < words; i++)
    {
      const uint32_t expected = patterns[p] ^ (i * 0x01010101U);
      const uint32_t actual = ram[i];

      if (actual != expected)
      {
        g_app_hyperram_error_addr = APP_HYPERRAM_BASE + (i * sizeof(uint32_t));
        g_app_hyperram_expected = expected;
        g_app_hyperram_actual = actual;
        return 0;
      }
    }
  }

  return 1;
}

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

static uint32_t App_SAI_AudioFrequency(uint32_t sample_rate_hz)
{
  if (sample_rate_hz == PCMD3180_SAMPLE_RATE_192K)
  {
    return SAI_AUDIO_FREQUENCY_192K;
  }

  return SAI_AUDIO_FREQUENCY_48K;
}

static uint32_t App_SAI_SlotActiveMask(uint8_t slot_count)
{
  if (slot_count >= 16U)
  {
    return SAI_SLOTACTIVE_ALL;
  }

  return (1UL << slot_count) - 1UL;
}

static HAL_StatusTypeDef App_PCMD_ConfigSaiPll2(void)
{
  RCC_OscInitTypeDef hse = {0};
  RCC_OscInitTypeDef oscillator = {0};

  if (g_pcmd_sai_pll2_configured != 0U)
  {
    return HAL_OK;
  }

  hse.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  hse.HSEState = RCC_HSE_ON;
  if (HAL_RCC_OscConfig(&hse) != HAL_OK)
  {
    return HAL_ERROR;
  }

  /* HSE48 / 5 * 128 / 5 / 1 = 245.76 MHz for exact 48 kHz/192 kHz TDM clocks. */
  oscillator.OscillatorType = RCC_OSCILLATORTYPE_NONE;
  oscillator.PLL1.PLLState = RCC_PLL_NONE;
  oscillator.PLL2.PLLState = RCC_PLL_ON;
  oscillator.PLL2.PLLSource = RCC_PLLSOURCE_HSE;
  oscillator.PLL2.PLLM = 5;
  oscillator.PLL2.PLLN = 128;
  oscillator.PLL2.PLLFractional = 0;
  oscillator.PLL2.PLLP1 = 5;
  oscillator.PLL2.PLLP2 = 1;
  oscillator.PLL3.PLLState = RCC_PLL_NONE;
  oscillator.PLL4.PLLState = RCC_PLL_NONE;

  if (HAL_RCC_OscConfig(&oscillator) != HAL_OK)
  {
    return HAL_ERROR;
  }

  g_pcmd_sai_pll2_configured = 1U;
  return HAL_OK;
}

static void App_SAI_FillBlock(SAI_HandleTypeDef *hsai,
                              SAI_Block_TypeDef *instance,
                              uint32_t audio_mode,
                              uint32_t synchro,
                              const PCMD3180_ArrayModeConfigTypeDef *mode_config)
{
  const uint32_t slot_width_bits = PCMD3180_GetSlotWidthBits(mode_config->slot_width);
  const uint32_t frame_length = mode_config->tdm_slots_per_bus * slot_width_bits;

  hsai->Instance = instance;
  hsai->Init.Protocol = SAI_FREE_PROTOCOL;
  hsai->Init.AudioMode = audio_mode;
  hsai->Init.DataSize = SAI_DATASIZE_16;
  hsai->Init.FirstBit = SAI_FIRSTBIT_MSB;
  hsai->Init.ClockStrobing = SAI_CLOCKSTROBING_FALLINGEDGE;
  hsai->Init.Synchro = synchro;
  hsai->Init.SynchroExt = SAI_SYNCEXT_DISABLE;
  hsai->Init.MckOutput = SAI_MCK_OUTPUT_DISABLE;
  hsai->Init.OutputDrive = SAI_OUTPUTDRIVE_DISABLE;
  /* NODIV lets IC7 generate the TDM bit clock directly from FS * frame length. */
  hsai->Init.NoDivider = SAI_MASTERDIVIDER_DISABLE;
  hsai->Init.FIFOThreshold = SAI_FIFOTHRESHOLD_EMPTY;
  hsai->Init.AudioFrequency = App_SAI_AudioFrequency(mode_config->sample_rate_hz);
  hsai->Init.Mckdiv = 0;
  hsai->Init.MckOverSampling = SAI_MCK_OVERSAMPLING_DISABLE;
  hsai->Init.MonoStereoMode = SAI_STEREOMODE;
  hsai->Init.CompandingMode = SAI_NOCOMPANDING;
  hsai->Init.TriState = SAI_OUTPUT_NOTRELEASED;
  hsai->Init.PdmInit.Activation = DISABLE;
  hsai->Init.PdmInit.MicPairsNbr = 1;
  hsai->Init.PdmInit.ClockEnable = 0;
  hsai->FrameInit.FrameLength = frame_length;
  hsai->FrameInit.ActiveFrameLength = 1;
  hsai->FrameInit.FSDefinition = SAI_FS_STARTFRAME;
  hsai->FrameInit.FSPolarity = SAI_FS_ACTIVE_HIGH;
  hsai->FrameInit.FSOffset = SAI_FS_BEFOREFIRSTBIT;
  hsai->SlotInit.FirstBitOffset = 0;
  hsai->SlotInit.SlotSize = SAI_SLOTSIZE_16B;
  hsai->SlotInit.SlotNumber = mode_config->tdm_slots_per_bus;
  hsai->SlotInit.SlotActive = App_SAI_SlotActiveMask(mode_config->tdm_slots_per_bus);
}

static HAL_StatusTypeDef App_PCMD_InitSaiDma(DMA_HandleTypeDef *dma_handle,
                                              DMA_Channel_TypeDef *dma_instance,
                                              uint32_t request,
                                              DMA_NodeTypeDef *dma_node,
                                              DMA_QListTypeDef *dma_queue)
{
  DMA_NodeConfTypeDef node_config = {0};

  if ((dma_handle == NULL) || (dma_instance == NULL) ||
      (dma_node == NULL) || (dma_queue == NULL))
  {
    return HAL_ERROR;
  }

  memset(dma_handle, 0, sizeof(*dma_handle));
  memset(dma_node, 0, sizeof(*dma_node));
  memset(dma_queue, 0, sizeof(*dma_queue));
  dma_handle->Instance = dma_instance;

  node_config.NodeType = DMA_GPDMA_LINEAR_NODE;
  node_config.Init.Request = request;
  node_config.Init.BlkHWRequest = DMA_BREQ_SINGLE_BURST;
  node_config.Init.Direction = DMA_PERIPH_TO_MEMORY;
  node_config.Init.SrcInc = DMA_SINC_FIXED;
  node_config.Init.DestInc = DMA_DINC_INCREMENTED;
  node_config.Init.SrcDataWidth = DMA_SRC_DATAWIDTH_HALFWORD;
  node_config.Init.DestDataWidth = DMA_DEST_DATAWIDTH_HALFWORD;
  node_config.Init.Priority = DMA_HIGH_PRIORITY;
  node_config.Init.SrcBurstLength = 1;
  node_config.Init.DestBurstLength = 1;
  node_config.Init.TransferAllocatedPort =
      DMA_SRC_ALLOCATED_PORT0 | DMA_DEST_ALLOCATED_PORT1;
  node_config.Init.TransferEventMode = DMA_TCEM_BLOCK_TRANSFER;
  node_config.DataHandlingConfig.DataExchange = DMA_EXCHANGE_NONE;
  node_config.DataHandlingConfig.DataAlignment = DMA_DATA_RIGHTALIGN_ZEROPADDED;
  node_config.TriggerConfig.TriggerPolarity = DMA_TRIG_POLARITY_MASKED;
  node_config.SrcSecure = DMA_CHANNEL_SRC_SEC;
  node_config.DestSecure = DMA_CHANNEL_DEST_SEC;

  if (HAL_DMA_ConfigChannelAttributes(dma_handle,
                                      DMA_CHANNEL_PRIV | DMA_CHANNEL_SEC |
                                      DMA_CHANNEL_SRC_SEC | DMA_CHANNEL_DEST_SEC) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DMAEx_List_BuildNode(&node_config, dma_node) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DMAEx_List_InsertNode_Tail(dma_queue, dma_node) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DMAEx_List_SetCircularMode(dma_queue) != HAL_OK)
  {
    return HAL_ERROR;
  }

  dma_handle->InitLinkedList.Priority = DMA_HIGH_PRIORITY;
  dma_handle->InitLinkedList.LinkStepMode = DMA_LSM_FULL_EXECUTION;
  dma_handle->InitLinkedList.LinkAllocatedPort = DMA_LINK_ALLOCATED_PORT1;
  dma_handle->InitLinkedList.TransferEventMode = DMA_TCEM_LAST_LL_ITEM_TRANSFER;
  dma_handle->InitLinkedList.LinkedListMode = DMA_LINKEDLIST_CIRCULAR;

  if (HAL_DMAEx_List_Init(dma_handle) != HAL_OK)
  {
    return HAL_ERROR;
  }
  if (HAL_DMAEx_List_LinkQ(dma_handle, dma_queue) != HAL_OK)
  {
    return HAL_ERROR;
  }

  return HAL_OK;
}

static HAL_StatusTypeDef App_SAI_ApplyModeConfig(const PCMD3180_ArrayModeConfigTypeDef *mode_config)
{
  HAL_StatusTypeDef status;

  if (mode_config == NULL)
  {
    return HAL_ERROR;
  }

  if (App_PCMD_ConfigSaiPll2() != HAL_OK)
  {
    return HAL_ERROR;
  }

  App_PCMD_StopDma();

  if (hsai_BlockA1.State != HAL_SAI_STATE_RESET)
  {
    (void)HAL_SAI_DeInit(&hsai_BlockA1);
  }
  if (hsai_BlockB1.State != HAL_SAI_STATE_RESET)
  {
    (void)HAL_SAI_DeInit(&hsai_BlockB1);
  }

  memset(&hsai_BlockA1, 0, sizeof(hsai_BlockA1));
  memset(&hsai_BlockB1, 0, sizeof(hsai_BlockB1));

  App_SAI_FillBlock(&hsai_BlockA1,
                    SAI1_Block_A,
                    SAI_MODEMASTER_RX,
                    SAI_ASYNCHRONOUS,
                    mode_config);
  status = HAL_SAI_Init(&hsai_BlockA1);
  if (status != HAL_OK)
  {
    return status;
  }

  App_SAI_FillBlock(&hsai_BlockB1,
                    SAI1_Block_B,
                    SAI_MODESLAVE_RX,
                    SAI_SYNCHRONOUS,
                    mode_config);
  return HAL_SAI_Init(&hsai_BlockB1);
}

static void App_PCMD_StartDma(void)
{
  if (HAL_SAI_GetState(&hsai_BlockB1) == HAL_SAI_STATE_READY)
  {
    g_pcmd_debug.dma_b_status =
        HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)g_pcmd_sai_b_rx, APP_PCMD_DMA_WORDS);
    if (g_pcmd_debug.dma_b_status == HAL_OK)
    {
      /* Block B waits for Block A's internally synchronized first frame. */
      __HAL_SAI_CLEAR_FLAG(&hsai_BlockB1, SAI_FLAG_AFSDET | SAI_FLAG_LFSDET);
      __HAL_SAI_DISABLE_IT(&hsai_BlockB1, SAI_IT_AFSDET | SAI_IT_LFSDET);
    }
  }

  if (HAL_SAI_GetState(&hsai_BlockA1) == HAL_SAI_STATE_READY)
  {
    g_pcmd_debug.dma_a_status =
        HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)g_pcmd_sai_a_rx, APP_PCMD_DMA_WORDS);
  }
}

static void App_PCMD_StopDma(void)
{
  if (HAL_SAI_GetState(&hsai_BlockA1) == HAL_SAI_STATE_BUSY_RX)
  {
    (void)HAL_SAI_DMAStop(&hsai_BlockA1);
  }
  if (HAL_SAI_GetState(&hsai_BlockB1) == HAL_SAI_STATE_BUSY_RX)
  {
    (void)HAL_SAI_DMAStop(&hsai_BlockB1);
  }
}

static void App_PCMD_ProcessBuffer(const uint16_t *buffer,
                                   uint32_t word_count,
                                   uint8_t slot_count,
                                   volatile uint16_t *peak_hold,
                                   volatile uint16_t *last_sample)
{
  uint16_t instant_peak[APP_PCMD_MAX_SLOTS] = {0U};
  uint16_t instant_last[APP_PCMD_MAX_SLOTS] = {0U};

  if ((buffer == NULL) || (peak_hold == NULL) || (last_sample == NULL) ||
      (slot_count == 0U) || (slot_count > APP_PCMD_MAX_SLOTS))
  {
    return;
  }

  for (uint32_t word = 0U; word < word_count; word++)
  {
    const int32_t sample = (int16_t)buffer[word];
    const uint32_t magnitude = (sample < 0) ? (uint32_t)(-sample) : (uint32_t)sample;
    const uint32_t slot = word % slot_count;

    if (magnitude > instant_peak[slot])
    {
      instant_peak[slot] = (uint16_t)magnitude;
    }
    instant_last[slot] = buffer[word];
  }

  for (uint32_t slot = 0U; slot < APP_PCMD_MAX_SLOTS; slot++)
  {
    if (slot >= slot_count)
    {
      peak_hold[slot] = 0U;
      last_sample[slot] = 0U;
    }
    else if (instant_peak[slot] >= peak_hold[slot])
    {
      peak_hold[slot] = instant_peak[slot];
    }

    if (slot < slot_count)
    {
      last_sample[slot] = instant_last[slot];
    }
  }
}

static void App_PCMD_DecayLevels(void)
{
  for (uint32_t bus = 0U; bus < APP_PCMD_BUS_COUNT; bus++)
  {
    for (uint32_t slot = 0U; slot < APP_PCMD_MAX_SLOTS; slot++)
    {
      /* Decay once per UI refresh, not once per sub-millisecond DMA block. */
      g_pcmd_slot_peak[bus][slot] =
          (uint16_t)(((uint32_t)g_pcmd_slot_peak[bus][slot] * 3U) / 4U);
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

static uint8_t App_PCMD_RunAddressScan(uint8_t *ack_count,
                                       uint8_t *scan_rounds,
                                       uint8_t *scl_idle_high,
                                       uint8_t *sda_idle_high)
{
  uint8_t all_stable = 1U;
  uint8_t local_scl_idle_high = 0U;
  uint8_t local_sda_idle_high = 0U;

  if ((ack_count == NULL) || (scan_rounds == NULL) ||
      (scl_idle_high == NULL) || (sda_idle_high == NULL))
  {
    return 0U;
  }

  memset(ack_count, 0, PCMD3180_ARRAY_DEVICE_COUNT * sizeof(ack_count[0]));
  *scan_rounds = APP_PCMD_ADDR_SCAN_ROUNDS;

  for (uint32_t round = 0U; round < APP_PCMD_ADDR_SCAN_ROUNDS; round++)
  {
    for (uint32_t device = 0U;
         device < PCMD3180_ARRAY_DEVICE_COUNT;
         device++)
    {
      if (PCMD3180_HAL_ProbeAddress(&g_pcmd_bus_context,
                                    (uint8_t)(PCMD3180_I2C_ADDR_0 + device)) == PCMD3180_OK)
      {
        ack_count[device]++;
      }
    }
    HAL_Delay(1U);
  }

  PCMD3180_HAL_GetLineLevels(&g_pcmd_bus_context,
                             &local_scl_idle_high,
                             &local_sda_idle_high);
  *scl_idle_high = local_scl_idle_high;
  *sda_idle_high = local_sda_idle_high;
  for (uint32_t device = 0U;
       device < PCMD3180_ARRAY_DEVICE_COUNT;
       device++)
  {
    if (ack_count[device] != APP_PCMD_ADDR_SCAN_ROUNDS)
    {
      all_stable = 0U;
    }
  }

  if ((local_scl_idle_high == 0U) ||
      (local_sda_idle_high == 0U))
  {
    all_stable = 0U;
  }

  return all_stable;
}

static uint8_t App_PCMD_CheckAddressMap(void)
{
  return App_PCMD_RunAddressScan(g_pcmd_debug.address_ack_count,
                                 &g_pcmd_debug.address_scan_rounds,
                                 &g_pcmd_debug.scl_idle_high,
                                 &g_pcmd_debug.sda_idle_high);
}

static void App_PCMD_ConfigureMode(PCMD3180_ArrayModeTypeDef mode)
{
  PCMD3180_ArrayModeConfigTypeDef mode_config;
  PCMD3180_StatusTypeDef pcmd_status;

  memset(&mode_config, 0, sizeof(mode_config));
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
      g_pcmd_slot_peak[bus][slot] = 0U;
      g_pcmd_slot_last_sample[bus][slot] = 0U;
    }
  }

  g_pcmd_debug.mode = mode;
  g_pcmd_debug.mode_config_status = PCMD3180_GetArrayModeConfig(mode, &mode_config);
  if (g_pcmd_debug.mode_config_status != PCMD3180_OK)
  {
    g_pcmd_debug.mode_start_tick = HAL_GetTick();
    return;
  }

  g_pcmd_debug.mode_config = mode_config;
  g_pcmd_debug.sai_status = App_SAI_ApplyModeConfig(&mode_config);
  if (g_pcmd_debug.sai_status != HAL_OK)
  {
    g_pcmd_debug.mode_start_tick = HAL_GetTick();
    g_pcmd_debug.last_poll_tick = 0U;
    g_pcmd_debug.reconfigure_count++;
    return;
  }

  g_pcmd_debug.address_scan_ok = App_PCMD_CheckAddressMap();
  if (g_pcmd_debug.address_scan_ok == 0U)
  {
    g_pcmd_debug.mode_start_tick = HAL_GetTick();
    g_pcmd_debug.last_poll_tick = 0U;
    g_pcmd_debug.reconfigure_count++;
    return;
  }

  /*
   * Keep all PCMD3180 I2C traffic before SAI starts. This board has already
   * shown unreliable I2C once SAI is running, so the status below is a cached
   * pre-SAI readback used to prove the codec configuration was written.
   */
  for (uint32_t i = 0; i < PCMD3180_ARRAY_DEVICE_COUNT; i++)
  {
    PCMD3180_ConfigTypeDef device_config;
    const uint8_t address7 = mode_config.devices[i].address7;

    (void)PCMD3180_Init(&g_pcmd_handles[i], &g_pcmd_bus, address7);

    pcmd_status = PCMD3180_Probe(&g_pcmd_handles[i]);
    g_pcmd_devices[i].probe_status = pcmd_status;
    g_pcmd_devices[i].present = (pcmd_status == PCMD3180_OK) ? 1U : 0U;
    if (pcmd_status != PCMD3180_OK)
    {
      g_pcmd_devices[i].config_status = pcmd_status;
      g_pcmd_devices[i].status_status = pcmd_status;
      continue;
    }

    pcmd_status = PCMD3180_BuildDeviceConfig(&mode_config, i, &device_config);
    if (pcmd_status == PCMD3180_OK)
    {
      pcmd_status = PCMD3180_Configure(&g_pcmd_handles[i], &device_config);
    }
    g_pcmd_devices[i].config_status = pcmd_status;
    if (pcmd_status == PCMD3180_OK)
    {
      g_pcmd_devices[i].status_status =
          PCMD3180_ReadStatus(&g_pcmd_handles[i], &g_pcmd_devices[i].snapshot);
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
  App_PCMD_StartDma();
}

static void App_PCMD_PrepareBus(uint8_t reset_device)
{
  memset(&g_pcmd_bus_context, 0, sizeof(g_pcmd_bus_context));

  g_pcmd_bus_context.hi2c = &hi2c2;
  g_pcmd_bus_context.scl_port = GPIOD;
  g_pcmd_bus_context.scl_pin = GPIO_PIN_14;
  g_pcmd_bus_context.sda_port = GPIOD;
  g_pcmd_bus_context.sda_pin = GPIO_PIN_4;
  g_pcmd_bus_context.shutdown_port = MIC_SHDNZ_GPIO_Port;
  g_pcmd_bus_context.shutdown_pin = MIC_SHDNZ_Pin;
  g_pcmd_bus_context.timeout_ms = APP_PCMD_I2C_TIMEOUT_MS;
  PCMD3180_HAL_BusInitSoftwareI2C(&g_pcmd_bus, &g_pcmd_bus_context);

  if ((reset_device != 0U) && (g_pcmd_bus.set_shutdown != NULL))
  {
    g_pcmd_bus.set_shutdown(g_pcmd_bus.context, 1U);
    /* TI requires a complete shutdown before release; 100 ms also covers warm reset. */
    HAL_Delay(APP_PCMD_RESET_LOW_MS);
    g_pcmd_bus.set_shutdown(g_pcmd_bus.context, 0U);
    HAL_Delay(APP_PCMD_RESET_SETTLE_MS);
  }
}

static void App_PCMD_EarlyI2CTest(void)
{
  App_PCMD_PrepareBus(1U);
  g_pcmd_debug.early_address_scan_ok =
      App_PCMD_RunAddressScan(g_pcmd_debug.early_address_ack_count,
                              &g_pcmd_debug.early_address_scan_rounds,
                              &g_pcmd_debug.early_scl_idle_high,
                              &g_pcmd_debug.early_sda_idle_high);
}

static void App_PCMD_DebugInit(void)
{
  memset(g_pcmd_devices, 0, sizeof(g_pcmd_devices));

  if (g_pcmd_debug.early_address_scan_rounds == 0U)
  {
    App_PCMD_PrepareBus(1U);
  }
  else
  {
    App_PCMD_PrepareBus(0U);
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

  if (g_pcmd_debug.address_scan_ok != 0U)
  {
    App_PCMD_StartDma();
  }
}

static void App_PCMD_ShowMicActivity(uint16_t start_y)
{
  char label[32];
  PCMD3180_ArrayModeConfigTypeDef full_array_map;
  const uint16_t row_h = 18U;
  const uint16_t col_x0 = 56U;
  const uint16_t col_w = 132U;
  uint16_t y = start_y;

  memset(&full_array_map, 0, sizeof(full_array_map));
  (void)PCMD3180_GetArrayModeConfig(PCMD3180_ARRAY_MODE_32CH_48K, &full_array_map);

  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 16, 16,
                     "MIC RAW16: U1-U4 all channels, latest sample",
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
      const uint8_t mic_id = full_array_map.devices[device_index].mic_id[channel];
      const uint8_t channel_bit = (uint8_t)(PCMD3180_CHANNEL_1 >> channel);
      const uint8_t active = ((channel < plan->mic_count) &&
                              ((plan->output_channel_mask & channel_bit) != 0U)) ? 1U : 0U;
      uint16_t color = GRAY;

      if ((device->present == 0U) || (device->probe_status != PCMD3180_OK))
      {
        snprintf(label, sizeof(label), "M%02u IO", mic_id);
        color = RED;
      }
      else if (device->config_status != PCMD3180_OK)
      {
        snprintf(label, sizeof(label), "M%02u CFG", mic_id);
        color = RED;
      }
      else if (active == 0U)
      {
        snprintf(label, sizeof(label), "M%02u ----", mic_id);
      }
      else
      {
        const uint32_t slot = (uint32_t)plan->start_slot + channel;
        const uint16_t raw = ((plan->tdm_bus < APP_PCMD_BUS_COUNT) &&
                              (slot < APP_PCMD_MAX_SLOTS)) ?
                             g_pcmd_slot_last_sample[plan->tdm_bus][slot] : 0U;
        const uint16_t peak = ((plan->tdm_bus < APP_PCMD_BUS_COUNT) &&
                               (slot < APP_PCMD_MAX_SLOTS)) ?
                              g_pcmd_slot_peak[plan->tdm_bus][slot] : 0U;

        if (peak >= 0x0400U)
        {
          color = GREEN;
        }
        else if (peak != 0U)
        {
          color = YELLOW;
        }

        snprintf(label, sizeof(label), "M%02u %04X", mic_id, (unsigned int)raw);
      }

      rgblcd_show_string(x, row_y, (uint16_t)(col_w - 6U), 12, 12, label, color);
    }
  }
}

static void App_PCMD_ShowDebugPage(void)
{
  char line[128];
  uint16_t y = 18;

  rgblcd_clear(BLACK);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 24, 24,
                     "NECCS N647 PCMD3180 DEBUG", CYAN);
  y += 34U;

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
           (unsigned long)g_app_hyperram_test_ok,
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

  snprintf(line,
           sizeof(line),
           "PDMCLK exp:%luHz  pre-SAI cfg: PD40 G41414141 GI4567",
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
           "I2C rec:%lu %c %02X:%02X val:%02X h:%lu e:%lX",
           (unsigned long)g_pcmd_bus_context.recover_count,
           (g_pcmd_bus_context.last_is_read != 0U) ? 'R' : 'W',
           g_pcmd_bus_context.last_address7,
           g_pcmd_bus_context.last_reg,
           g_pcmd_bus_context.last_value,
           (unsigned long)g_pcmd_bus_context.last_hal_status,
           (unsigned long)g_pcmd_bus_context.last_hal_error);
  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12, line, CYAN);
  y += 20U;

  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12,
                     "Dev Adr P C S PWR ASI D0/D1 PD PI GPOs GI EN/AO",
                     YELLOW);
  y += 16U;

  for (uint32_t i = 0; i < PCMD3180_ARRAY_DEVICE_COUNT; i++)
  {
    const AppPcmdDeviceState_t *device = &g_pcmd_devices[i];
    const uint8_t routing_ok = App_PCMD_IsRoutingSnapshotOk(i, &device->snapshot);
    const uint16_t color = (device->present != 0U) ?
                           (((device->config_status == PCMD3180_OK) &&
                             (device->status_status == PCMD3180_OK)) ?
                            ((routing_ok != 0U) ? GREEN : RED) : YELLOW) :
                           RED;

    snprintf(line,
             sizeof(line),
             "U%lu %02X %-2s %-2s %-2s P%02X A%02X D%02X/%02X PD%02X PI%02X G%02X%02X%02X%02X GI%02X/%02X E%02X/O%02X",
             (unsigned long)(i + 1U),
             (unsigned int)(PCMD3180_I2C_ADDR_0 + i),
             App_PCMD_StatusName(device->probe_status),
             App_PCMD_StatusName(device->config_status),
             App_PCMD_StatusName(device->status_status),
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

  rgblcd_show_string(APP_PCMD_UI_X, y, APP_PCMD_UI_W, 12, 12,
                      "Expected: U1-U4 OK; raw hex changes with input.",
                      LGRAY);
  y += 20U;
  App_PCMD_ShowMicActivity(y);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable the CPU Cache */

  App_MPU_ConfigNonCacheable();

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/
  HAL_Init();

  /* USER CODE BEGIN Init */
#ifdef DEBUG
  sys_clock_config_debug();
#endif
  SystemCoreClockUpdate();
  App_RecoverHalTick();

  /* USER CODE END Init */

  /* USER CODE BEGIN SysInit */
#ifdef DEBUG
  MX_XSPI1_Init();
  if (HyperRAM_Init(&HyperRAMObject, &hxspi1) != HyperRAM_OK)
  {
    Error_Handler();
  }
  if (HyperRAM_EnableMemoryMappedMode(&HyperRAMObject) != HyperRAM_OK)
  {
    Error_Handler();
  }
  g_app_hyperram_init_ok = 1;
#else
  /* In cold boot, FSBL already configures XSPI1 HyperRAM before jumping to Appli. */
  g_app_hyperram_init_ok = 2;
#endif

  g_app_hyperram_test_ok = App_HyperRAM_SelfTest();
  if (g_app_hyperram_test_ok == 0)
  {
    Error_Handler();
  }

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  SystemIsolation_Config();
  MX_I2C2_Init();
  App_PCMD_EarlyI2CTest();
  MX_DMA2D_Init();
  MX_LTDC_Init();
  MX_SAI1_Init();
  /* USER CODE BEGIN 2 */
  led_init();
  rgblcd_init();
  App_PCMD_DebugInit();
  g_pcmd_last_ui_tick = HAL_GetTick() - APP_PCMD_UI_REFRESH_MS;

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    const uint32_t now = HAL_GetTick();

    App_PCMD_Task();
    if ((now - g_pcmd_last_ui_tick) >= APP_PCMD_UI_REFRESH_MS)
    {
      g_pcmd_last_ui_tick = now;
      App_PCMD_ShowDebugPage();
      App_PCMD_DecayLevels();
      LED0_TOGGLE();
    }
    HAL_Delay(20U);
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief DMA2D Initialization Function
  * @param None
  * @retval None
  */
static void MX_DMA2D_Init(void)
{

  /* USER CODE BEGIN DMA2D_Init 0 */

  /* USER CODE END DMA2D_Init 0 */

  /* USER CODE BEGIN DMA2D_Init 1 */

  /* USER CODE END DMA2D_Init 1 */
  hdma2d.Instance = DMA2D;
  hdma2d.Init.Mode = DMA2D_M2M_PFC;
  hdma2d.Init.ColorMode = DMA2D_OUTPUT_RGB565;
  hdma2d.Init.OutputOffset = 0;
  hdma2d.LayerCfg[1].InputOffset = 0;
  hdma2d.LayerCfg[1].InputColorMode = DMA2D_INPUT_RGB565;
  hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hdma2d.LayerCfg[1].InputAlpha = 255;
  if (HAL_DMA2D_Init(&hdma2d) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_DMA2D_ConfigLayer(&hdma2d, 1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DMA2D_Init 2 */

  /* USER CODE END DMA2D_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.Timing = APP_I2C2_TIMING_25KHZ_64MHZ;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.OwnAddress2Masks = I2C_OA2_NOMASK;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_I2CEx_ConfigAnalogFilter(&hi2c2, I2C_ANALOGFILTER_ENABLE) != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_I2CEx_ConfigDigitalFilter(&hi2c2, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief LTDC Initialization Function
  * @param None
  * @retval None
  */
static void MX_LTDC_Init(void)
{

  /* USER CODE BEGIN LTDC_Init 0 */

  /* USER CODE END LTDC_Init 0 */

  LTDC_LayerCfgTypeDef pLayerCfg = {0};

  /* USER CODE BEGIN LTDC_Init 1 */

  /* USER CODE END LTDC_Init 1 */
  hltdc.Instance = LTDC;
  hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
  hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
  hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
  hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
  hltdc.Init.HorizontalSync = 19;
  hltdc.Init.VerticalSync = 2;
  hltdc.Init.AccumulatedHBP = 159;
  hltdc.Init.AccumulatedVBP = 22;
  hltdc.Init.AccumulatedActiveW = 1183;
  hltdc.Init.AccumulatedActiveH = 622;
  hltdc.Init.TotalWidth = 1343;
  hltdc.Init.TotalHeigh = 634;
  hltdc.Init.Backcolor.Blue = 0;
  hltdc.Init.Backcolor.Green = 0;
  hltdc.Init.Backcolor.Red = 0;
  if (HAL_LTDC_Init(&hltdc) != HAL_OK)
  {
    Error_Handler();
  }
  pLayerCfg.WindowX0 = 0;
  pLayerCfg.WindowX1 = 1024;
  pLayerCfg.WindowY0 = 0;
  pLayerCfg.WindowY1 = 600;
  pLayerCfg.PixelFormat = LTDC_PIXEL_FORMAT_RGB565;
  pLayerCfg.Alpha = 255;
  pLayerCfg.Alpha0 = 0;
  pLayerCfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_CA;
  pLayerCfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_CA;
  pLayerCfg.FBStartAdress = 0;
  pLayerCfg.ImageWidth = 1024;
  pLayerCfg.ImageHeight = 600;
  pLayerCfg.Backcolor.Blue = 0;
  pLayerCfg.Backcolor.Green = 0;
  pLayerCfg.Backcolor.Red = 0;
  if (HAL_LTDC_ConfigLayer(&hltdc, &pLayerCfg, 0) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN LTDC_Init 2 */

  /* USER CODE END LTDC_Init 2 */

}

/**
  * @brief SAI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SAI1_Init(void)
{
  /* USER CODE BEGIN SAI1_Init 0 */

  /* USER CODE END SAI1_Init 0 */

  /* USER CODE BEGIN SAI1_Init 1 */

  /* USER CODE END SAI1_Init 1 */
  /* USER CODE BEGIN SAI1_Init 2 */
  PCMD3180_ArrayModeConfigTypeDef default_mode_config;

  /*
   * CubeMX cannot fully describe the PCMD3180 2-bus TDM setup in the IOC.
   * Keep this override in USER CODE so regeneration cannot drop the runtime
   * frame/slot/clock configuration.
   */
  if (PCMD3180_GetArrayModeConfig(PCMD3180_ARRAY_MODE_32CH_48K,
                                  &default_mode_config) != PCMD3180_OK)
  {
    Error_Handler();
  }

  if (App_SAI_ApplyModeConfig(&default_mode_config) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE END SAI1_Init 2 */

}

/**
  * @brief RIF Initialization Function
  * @param None
  * @retval None
  */
  static void SystemIsolation_Config(void)
{

  /* USER CODE BEGIN RIF_Init 0 */

  /* USER CODE END RIF_Init 0 */

  /* set all required IPs as secure privileged */
  __HAL_RCC_RIFSC_CLK_ENABLE();

  /*RIMC configuration*/
  RIMC_MasterConfig_t RIMC_master = {0};
  RIMC_master.MasterCID = RIF_CID_1;
  RIMC_master.SecPriv = RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV;
  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DCMIPP, &RIMC_master);

  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_DMA2D, &RIMC_master);

  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_LTDC1, &RIMC_master);

  HAL_RIF_RIMC_ConfigMasterAttributes(RIF_MASTER_INDEX_SDMMC2, &RIMC_master);

  /*RISUP configuration*/
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_SDMMC2 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_SAI1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_I2C2 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DCMIPP , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_DMA2D , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RISC_PERIPH_INDEX_LTDCL1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);
  HAL_RIF_RISC_SetSlaveSecureAttributes(RIF_RCC_PERIPH_INDEX_GPDMA1 , RIF_ATTRIBUTE_SEC | RIF_ATTRIBUTE_PRIV);

  /* RIF-Aware IPs Config */

  /* set up PWR configuration */
  HAL_PWR_ConfigAttributes(PWR_ITEM_0,PWR_SEC_NPRIV);

  /* set up GPIO configuration */
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOA,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOB,GPIO_PIN_15,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOC,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOD,GPIO_PIN_14,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_14,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOE,GPIO_PIN_15,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOF,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_9,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_10,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_11,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_13,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_14,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOG,GPIO_PIN_15,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOH,GPIO_PIN_8,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPION,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPION,GPIO_PIN_12,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOO,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOP,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_0,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_1,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_2,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_3,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_4,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_5,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_6,GPIO_PIN_SEC|GPIO_PIN_NPRIV);
  HAL_GPIO_ConfigPinAttributes(GPIOQ,GPIO_PIN_7,GPIO_PIN_SEC|GPIO_PIN_NPRIV);

  /* USER CODE BEGIN RIF_Init 1 */

  /* USER CODE END RIF_Init 1 */
  /* USER CODE BEGIN RIF_Init 2 */

  /* USER CODE END RIF_Init 2 */

}

/**
  * @brief XSPI1 Initialization Function
  * @param None
  * @retval None
  */
void MX_XSPI1_Init(void)
{

  /* USER CODE BEGIN XSPI1_Init 0 */

  /* USER CODE END XSPI1_Init 0 */

  XSPIM_CfgTypeDef sXspiManagerCfg = {0};
  XSPI_HyperbusCfgTypeDef sHyperBusCfg = {0};

  /* USER CODE BEGIN XSPI1_Init 1 */

  /* USER CODE END XSPI1_Init 1 */
  /* XSPI1 parameter configuration*/
  hxspi1.Instance = XSPI1;
  hxspi1.Init.FifoThresholdByte = 4;
  hxspi1.Init.MemoryMode = HAL_XSPI_SINGLE_MEM;
  hxspi1.Init.MemoryType = HAL_XSPI_MEMTYPE_HYPERBUS;
  hxspi1.Init.MemorySize = HAL_XSPI_SIZE_256MB;
  hxspi1.Init.ChipSelectHighTimeCycle = 2;
  hxspi1.Init.FreeRunningClock = HAL_XSPI_FREERUNCLK_DISABLE;
  hxspi1.Init.ClockMode = HAL_XSPI_CLOCK_MODE_0;
  hxspi1.Init.WrapSize = HAL_XSPI_WRAP_32_BYTES;
  hxspi1.Init.ClockPrescaler = 1 - 1;
  hxspi1.Init.SampleShifting = HAL_XSPI_SAMPLE_SHIFT_NONE;
  hxspi1.Init.DelayHoldQuarterCycle = HAL_XSPI_DHQC_DISABLE;
  hxspi1.Init.ChipSelectBoundary = HAL_XSPI_BONDARYOF_NONE;
  hxspi1.Init.MaxTran = 0;
  hxspi1.Init.Refresh = 0;
  hxspi1.Init.MemorySelect = HAL_XSPI_CSSEL_NCS1;
  if (HAL_XSPI_Init(&hxspi1) != HAL_OK)
  {
    Error_Handler();
  }
  sXspiManagerCfg.nCSOverride = HAL_XSPI_CSSEL_OVR_NCS1;
  sXspiManagerCfg.IOPort = HAL_XSPIM_IOPORT_1;
  sXspiManagerCfg.Req2AckTime = 1;
  if (HAL_XSPIM_Config(&hxspi1, &sXspiManagerCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  sHyperBusCfg.RWRecoveryTimeCycle = 7;
  sHyperBusCfg.AccessTimeCycle = 7;
  sHyperBusCfg.WriteZeroLatency = HAL_XSPI_LATENCY_ON_WRITE;
  sHyperBusCfg.LatencyMode = HAL_XSPI_FIXED_LATENCY;
  if (HAL_XSPI_HyperbusCfg(&hxspi1, &sHyperBusCfg, HAL_XSPI_TIMEOUT_DEFAULT_VALUE) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN XSPI1_Init 2 */

  /* USER CODE END XSPI1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */
  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOQ_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOP_CLK_ENABLE();
  __HAL_RCC_GPIOO_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPION_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, CTP_RST_Pin|LCD_BL_PWM_Pin|CAM_LED_EN_Pin|LCD_NRST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(MIC_SHDNZ_GPIO_Port, MIC_SHDNZ_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_10, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(EXT_SMPS_MODE_GPIO_Port, EXT_SMPS_MODE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(CAM_EN_MODULE_GPIO_Port, CAM_EN_MODULE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(PWR_SD_EN_GPIO_Port, PWR_SD_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED1_GPIO_Port, LED1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SPI2_CS0_GPIO_Port, SPI2_CS0_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|USB1_EN_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED2_GPIO_Port, LED2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pins : EXP_GPIO_PQ6_Pin EXP_GPIO_PQ3_Pin CTP_INT_Pin EXP_GPIO_PQ5_Pin
                           EXP_GPIO_PQ2_Pin EXP_GPIO_PQ1_Pin EXP_GPIO_PQ0_Pin */
  GPIO_InitStruct.Pin = EXP_GPIO_PQ6_Pin|EXP_GPIO_PQ3_Pin|CTP_INT_Pin|EXP_GPIO_PQ5_Pin
                          |EXP_GPIO_PQ2_Pin|EXP_GPIO_PQ1_Pin|EXP_GPIO_PQ0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOQ, &GPIO_InitStruct);

  /*Configure GPIO pins : EXP_GPIO_PC10_Pin EXP_GPIO_PC11_Pin EXP_GPIO_PC12_Pin EXP_GPIO_PC7_Pin
                           TAMP_Pin */
  GPIO_InitStruct.Pin = EXP_GPIO_PC10_Pin|EXP_GPIO_PC11_Pin|EXP_GPIO_PC12_Pin|EXP_GPIO_PC7_Pin
                          |TAMP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : EXP_GPIO_PE13_Pin EXP_GPIO_PE15_Pin EXP_GPIO_PE12_Pin EXP_GPIO_PE7_Pin
                           USER2_Pin USER1_Pin */
  GPIO_InitStruct.Pin = EXP_GPIO_PE13_Pin|EXP_GPIO_PE15_Pin|EXP_GPIO_PE12_Pin|EXP_GPIO_PE7_Pin
                          |USER2_Pin|USER1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : UCPD1_INT_Pin EXP_ALT_ETH_PD1_Pin EXP_ALT_ETH_PD12_Pin EXP_GPIO_PD13_Pin
                           EXP_GPIO_PD3_Pin EXP_GPIO_PD11_Pin */
  GPIO_InitStruct.Pin = UCPD1_INT_Pin|EXP_ALT_ETH_PD1_Pin|EXP_ALT_ETH_PD12_Pin|EXP_GPIO_PD13_Pin
                          |EXP_GPIO_PD3_Pin|EXP_GPIO_PD11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : CTP_RST_Pin LCD_BL_PWM_Pin CAM_LED_EN_Pin LCD_NRST_Pin */
  GPIO_InitStruct.Pin = CTP_RST_Pin|LCD_BL_PWM_Pin|CAM_LED_EN_Pin|LCD_NRST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : MIC_SHDNZ_Pin */
  GPIO_InitStruct.Pin = MIC_SHDNZ_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(MIC_SHDNZ_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PE10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : EXP_GPIO_PH8_Pin EXP_GPIO_PH2_Pin EXP_GPIO_PH5_Pin */
  GPIO_InitStruct.Pin = EXP_GPIO_PH8_Pin|EXP_GPIO_PH2_Pin|EXP_GPIO_PH5_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : EXT_SMPS_MODE_Pin */
  GPIO_InitStruct.Pin = EXT_SMPS_MODE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(EXT_SMPS_MODE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : CAM_EN_MODULE_Pin */
  GPIO_InitStruct.Pin = CAM_EN_MODULE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(CAM_EN_MODULE_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : EXP_GPIO_PB3_Pin EXP_GPIO_PB1_Pin */
  GPIO_InitStruct.Pin = EXP_GPIO_PB3_Pin|EXP_GPIO_PB1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PWR_SD_EN_Pin */
  GPIO_InitStruct.Pin = PWR_SD_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(PWR_SD_EN_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LED1_Pin */
  GPIO_InitStruct.Pin = LED1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : EXP_GPIO_PG7_Pin EXP_GPIO_PG6_Pin EXP_GPIO_PG4_Pin EXP_GPIO_PG14_Pin
                           EXP_GPIO_PG3_Pin EXP_GPIO_PG15_Pin EXP_GPIO_PG1_Pin EXP_ALT_ETH_PG12_Pin
                           EXP_GPIO_PG2_Pin EXP_ALT_ETH_PG11_Pin */
  GPIO_InitStruct.Pin = EXP_GPIO_PG7_Pin|EXP_GPIO_PG6_Pin|EXP_GPIO_PG4_Pin|EXP_GPIO_PG14_Pin
                          |EXP_GPIO_PG3_Pin|EXP_GPIO_PG15_Pin|EXP_GPIO_PG1_Pin|EXP_ALT_ETH_PG12_Pin
                          |EXP_GPIO_PG2_Pin|EXP_ALT_ETH_PG11_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : EXP_ALT_ETH_PF10_Pin EXP_ALT_ETH_PF7_Pin EXP_GPIO_PF3_Pin EXP_GPIO_PF5_Pin
                           EXP_ALT_ETH_PF11_Pin EXP_GPIO_PF1_Pin EXP_ALT_ETH_PF13_Pin EXP_ALT_ETH_PF12_Pin */
  GPIO_InitStruct.Pin = EXP_ALT_ETH_PF10_Pin|EXP_ALT_ETH_PF7_Pin|EXP_GPIO_PF3_Pin|EXP_GPIO_PF5_Pin
                          |EXP_ALT_ETH_PF11_Pin|EXP_GPIO_PF1_Pin|EXP_ALT_ETH_PF13_Pin|EXP_ALT_ETH_PF12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : BOOT1_Pin EXP_GPIO_PA12_Pin */
  GPIO_InitStruct.Pin = BOOT1_Pin|EXP_GPIO_PA12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SD_DET_Pin UCPD1_VSENSE_Pin */
  GPIO_InitStruct.Pin = SD_DET_Pin|UCPD1_VSENSE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPION, &GPIO_InitStruct);

  /*Configure GPIO pin : SPI2_CS0_Pin */
  GPIO_InitStruct.Pin = SPI2_CS0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SPI2_CS0_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PA3 */
  GPIO_InitStruct.Pin = GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : LED2_Pin */
  GPIO_InitStruct.Pin = LED2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB1_EN_Pin */
  GPIO_InitStruct.Pin = USB1_EN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB1_EN_GPIO_Port, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if (hi2c->Instance == I2C2)
  {
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2C2;
    PeriphClkInitStruct.I2c2ClockSelection = RCC_I2C2CLKSOURCE_CLKP;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_RCC_GPIOD_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF4_I2C2;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    __HAL_RCC_I2C2_CLK_ENABLE();
  }
}

void HAL_I2C_MspDeInit(I2C_HandleTypeDef *hi2c)
{
  if (hi2c->Instance == I2C2)
  {
    __HAL_RCC_I2C2_CLK_DISABLE();
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_4 | GPIO_PIN_14);
  }
}

void HAL_SAI_MspInit(SAI_HandleTypeDef *hsai)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  const uint32_t ic7_divider =
      (hsai->Init.AudioFrequency == SAI_AUDIO_FREQUENCY_192K) ?
      APP_PCMD_SAI1_IC_DIVIDER_192K : APP_PCMD_SAI1_IC_DIVIDER_48K;

  if ((hsai->Instance != SAI1_Block_A) && (hsai->Instance != SAI1_Block_B))
  {
    return;
  }

  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_IC7;
  PeriphClkInitStruct.ICSelection[RCC_IC7].ClockSelection = RCC_ICCLKSOURCE_PLL2;
  PeriphClkInitStruct.ICSelection[RCC_IC7].ClockDivider = ic7_divider;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  if (g_sai1_client == 0U)
  {
    __HAL_RCC_SAI1_CLK_ENABLE();
  }
  g_sai1_client++;

  __HAL_RCC_GPDMA1_CLK_ENABLE();

  if (hsai->Instance == SAI1_Block_A)
  {
    __HAL_RCC_GPIOB_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    if (App_PCMD_InitSaiDma(&handle_GPDMA1_Channel0,
                            GPDMA1_Channel0,
                            GPDMA1_REQUEST_SAI1_A,
                            &g_pcmd_sai_a_dma_node,
                            &g_pcmd_sai_a_dma_queue) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_LINKDMA(hsai, hdmarx, handle_GPDMA1_Channel0);

    HAL_NVIC_SetPriority(SAI1_A_IRQn, APP_PCMD_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(SAI1_A_IRQn);
    HAL_NVIC_SetPriority(GPDMA1_Channel0_IRQn, APP_PCMD_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel0_IRQn);
  }
  else
  {
    __HAL_RCC_GPIOE_CLK_ENABLE();
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    if (App_PCMD_InitSaiDma(&handle_GPDMA1_Channel1,
                            GPDMA1_Channel1,
                            GPDMA1_REQUEST_SAI1_B,
                            &g_pcmd_sai_b_dma_node,
                            &g_pcmd_sai_b_dma_queue) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_LINKDMA(hsai, hdmarx, handle_GPDMA1_Channel1);

    HAL_NVIC_SetPriority(SAI1_B_IRQn, APP_PCMD_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(SAI1_B_IRQn);
    HAL_NVIC_SetPriority(GPDMA1_Channel1_IRQn, APP_PCMD_IRQ_PRIO, 0);
    HAL_NVIC_EnableIRQ(GPDMA1_Channel1_IRQn);
  }
}

void HAL_SAI_MspDeInit(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    HAL_NVIC_DisableIRQ(SAI1_A_IRQn);
    HAL_NVIC_DisableIRQ(GPDMA1_Channel0_IRQn);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_0 | GPIO_PIN_6 | GPIO_PIN_7);
    if (hsai->hdmarx != NULL)
    {
      (void)HAL_DMAEx_List_DeInit(hsai->hdmarx);
    }
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    HAL_NVIC_DisableIRQ(SAI1_B_IRQn);
    HAL_NVIC_DisableIRQ(GPDMA1_Channel1_IRQn);
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_3);
    if (hsai->hdmarx != NULL)
    {
      (void)HAL_DMAEx_List_DeInit(hsai->hdmarx);
    }
  }
  else
  {
    return;
  }

  if (g_sai1_client > 0U)
  {
    g_sai1_client--;
    if (g_sai1_client == 0U)
    {
      __HAL_RCC_SAI1_CLK_DISABLE();
    }
  }
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  if (hsai->Instance == SAI1_Block_A)
  {
    g_pcmd_sai_a_half_count++;
    App_PCMD_ProcessBuffer(g_pcmd_sai_a_rx,
                           APP_PCMD_DMA_WORDS / 2U,
                           g_pcmd_debug.mode_config.tdm_slots_per_bus,
                           g_pcmd_slot_peak[PCMD3180_TDM_BUS_A],
                           g_pcmd_slot_last_sample[PCMD3180_TDM_BUS_A]);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    g_pcmd_sai_b_half_count++;
    App_PCMD_ProcessBuffer(g_pcmd_sai_b_rx,
                           APP_PCMD_DMA_WORDS / 2U,
                           g_pcmd_debug.mode_config.tdm_slots_per_bus,
                           g_pcmd_slot_peak[PCMD3180_TDM_BUS_B],
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
                           g_pcmd_slot_peak[PCMD3180_TDM_BUS_A],
                           g_pcmd_slot_last_sample[PCMD3180_TDM_BUS_A]);
  }
  else if (hsai->Instance == SAI1_Block_B)
  {
    g_pcmd_sai_b_full_count++;
    App_PCMD_ProcessBuffer(&g_pcmd_sai_b_rx[APP_PCMD_DMA_WORDS / 2U],
                           APP_PCMD_DMA_WORDS / 2U,
                           g_pcmd_debug.mode_config.tdm_slots_per_bus,
                           g_pcmd_slot_peak[PCMD3180_TDM_BUS_B],
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

void SAI1_A_IRQHandler(void)
{
  HAL_SAI_IRQHandler(&hsai_BlockA1);
}

void SAI1_B_IRQHandler(void)
{
  HAL_SAI_IRQHandler(&hsai_BlockB1);
}

void GPDMA1_Channel0_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel0);
}

void GPDMA1_Channel1_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&handle_GPDMA1_Channel1);
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
