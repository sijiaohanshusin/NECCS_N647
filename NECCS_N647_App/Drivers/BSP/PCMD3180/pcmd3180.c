#include "pcmd3180.h"

#define PCMD3180_MAX_CHANNELS_PER_DEVICE     8U
#define PCMD3180_MAX_TDM_SLOT                63U
#define PCMD3180_ASI_CFG0_TDM_MODE           0x00U
#define PCMD3180_ASI_CFG0_TX_HIGH_Z          0x01U
#define PCMD3180_WRITE_VERIFY_RETRY_COUNT    4U
#define PCMD3180_WRITE_VERIFY_DELAY_MS       1U

static const uint8_t PCMD3180_V2_MIC_MAP[PCMD3180_ARRAY_DEVICE_COUNT][PCMD3180_ARRAY_MAX_MICS_PER_DEV] =
{
    /* U1, Bus A: slot 0..7 at 48 kHz. */
    { 1U,  2U,  9U, 10U, 17U, 18U, 19U, 20U },
    /* U2, Bus A: slot 8..15 at 48 kHz. */
    { 3U,  4U, 11U, 12U, 29U, 30U, 31U, 32U },
    /* U3, Bus B: slot 0..7 at 48 kHz. */
    { 5U,  6U, 13U, 14U, 25U, 26U, 27U, 28U },
    /* U4, Bus B: slot 8..15 at 48 kHz. */
    { 7U,  8U, 15U, 16U, 21U, 22U, 23U, 24U }
};

static PCMD3180_StatusTypeDef PCMD3180_CheckHandle(const PCMD3180_HandleTypeDef *handle)
{
    if ((handle == NULL) ||
        (handle->bus.write_reg == NULL) ||
        (handle->bus.read_reg == NULL))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    return PCMD3180_OK;
}

static void PCMD3180_Delay(const PCMD3180_HandleTypeDef *handle, uint32_t delay_ms)
{
    if ((handle != NULL) && (handle->bus.delay_ms != NULL) && (delay_ms > 0U))
    {
        handle->bus.delay_ms(handle->bus.context, delay_ms);
    }
}

static PCMD3180_StatusTypeDef PCMD3180_WriteChecked(PCMD3180_HandleTypeDef *handle,
                                                    uint8_t reg,
                                                    uint8_t value,
                                                    uint8_t verify)
{
    uint8_t readback = 0U;
    PCMD3180_StatusTypeDef status = PCMD3180_ERROR;

    if (handle == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    handle->last_reg = reg;
    handle->last_write_value = value;
    handle->last_read_value = 0U;

    for (uint32_t attempt = 0U; attempt < PCMD3180_WRITE_VERIFY_RETRY_COUNT; attempt++)
    {
        status = PCMD3180_WriteRegister(handle, reg, value);
        if (status != PCMD3180_OK)
        {
            handle->last_status = status;
            PCMD3180_Delay(handle, PCMD3180_WRITE_VERIFY_DELAY_MS);
            continue;
        }

        if (verify == 0U)
        {
            handle->last_status = PCMD3180_OK;
            return PCMD3180_OK;
        }

        PCMD3180_Delay(handle, PCMD3180_WRITE_VERIFY_DELAY_MS);
        status = PCMD3180_ReadRegister(handle, reg, &readback);
        handle->last_read_value = readback;
        if ((status == PCMD3180_OK) && (readback == value))
        {
            handle->last_status = PCMD3180_OK;
            return PCMD3180_OK;
        }

        handle->last_status = (status == PCMD3180_OK) ? PCMD3180_VERIFY_ERROR : status;
        PCMD3180_Delay(handle, PCMD3180_WRITE_VERIFY_DELAY_MS);
    }

    return handle->last_status;
}

static PCMD3180_StatusTypeDef PCMD3180_WriteChannelSlots(PCMD3180_HandleTypeDef *handle,
                                                         uint8_t start_slot,
                                                         uint8_t verify)
{
    uint8_t channel;

    if ((uint32_t)start_slot + (PCMD3180_MAX_CHANNELS_PER_DEVICE - 1U) > PCMD3180_MAX_TDM_SLOT)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    for (channel = 0U; channel < PCMD3180_MAX_CHANNELS_PER_DEVICE; channel++)
    {
        PCMD3180_StatusTypeDef status;

        status = PCMD3180_WriteChecked(handle,
                                       (uint8_t)(PCMD3180_REG_ASI_CH1 + channel),
                                       (uint8_t)(start_slot + channel),
                                       verify);
        if (status != PCMD3180_OK)
        {
            return status;
        }
    }

    return PCMD3180_OK;
}

static PCMD3180_StatusTypeDef PCMD3180_WritePdmInputConfig(PCMD3180_HandleTypeDef *handle,
                                                           uint8_t verify)
{
    uint8_t channel;

    for (channel = 0U; channel < PCMD3180_MAX_CHANNELS_PER_DEVICE; channel++)
    {
        PCMD3180_StatusTypeDef status;

        status = PCMD3180_WriteChecked(handle,
                                       (uint8_t)(PCMD3180_REG_CH1_CFG0 + (channel * 5U)),
                                       PCMD3180_CH_CFG0_PDM_INPUT,
                                       verify);
        if (status != PCMD3180_OK)
        {
            return status;
        }
    }

    return PCMD3180_OK;
}

static void PCMD3180_ClearModeConfig(PCMD3180_ArrayModeConfigTypeDef *mode_config)
{
    uint32_t device;
    uint32_t mic;

    if (mode_config == NULL)
    {
        return;
    }

    mode_config->mode = PCMD3180_ARRAY_MODE_32CH_48K;
    mode_config->sample_rate_hz = 0U;
    mode_config->frame_samples = 0U;
    mode_config->slot_width = PCMD3180_SLOT_WIDTH_16_BITS;
    mode_config->tdm_slots_per_bus = 0U;
    mode_config->physical_mic_count = 0U;
    mode_config->capture_channel_count = 0U;
    mode_config->recommended_pair_count = 0U;
    mode_config->band_low_hz = 0U;
    mode_config->band_high_hz = 0U;
    mode_config->expected_bclk_hz = 0U;

    for (device = 0U; device < PCMD3180_ARRAY_DEVICE_COUNT; device++)
    {
        mode_config->devices[device].device = (PCMD3180_ArrayDeviceTypeDef)device;
        mode_config->devices[device].tdm_bus =
            (device < 2U) ? PCMD3180_TDM_BUS_A : PCMD3180_TDM_BUS_B;
        mode_config->devices[device].address7 = (uint8_t)(PCMD3180_I2C_ADDR_0 + device);
        mode_config->devices[device].start_slot = 0U;
        mode_config->devices[device].input_channel_mask = 0U;
        mode_config->devices[device].output_channel_mask = 0U;
        mode_config->devices[device].mic_count = 0U;

        for (mic = 0U; mic < PCMD3180_ARRAY_MAX_MICS_PER_DEV; mic++)
        {
            mode_config->devices[device].mic_id[mic] = 0U;
        }
    }
}

static void PCMD3180_SetDevicePlan(PCMD3180_ArrayModeConfigTypeDef *mode_config,
                                   uint32_t device,
                                   uint8_t start_slot,
                                   uint8_t channel_mask,
                                   uint8_t mic_count)
{
    uint32_t mic;

    mode_config->devices[device].start_slot = start_slot;
    mode_config->devices[device].input_channel_mask = channel_mask;
    mode_config->devices[device].output_channel_mask = channel_mask;
    mode_config->devices[device].mic_count = mic_count;

    for (mic = 0U; mic < PCMD3180_ARRAY_MAX_MICS_PER_DEV; mic++)
    {
        mode_config->devices[device].mic_id[mic] =
            (mic < mic_count) ? PCMD3180_V2_MIC_MAP[device][mic] : 0U;
    }
}

static void PCMD3180_Set32ChannelPlan(PCMD3180_ArrayModeConfigTypeDef *mode_config)
{
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U1, 0U, PCMD3180_CHANNEL_ALL, 8U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U2, 8U, PCMD3180_CHANNEL_ALL, 8U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U3, 0U, PCMD3180_CHANNEL_ALL, 8U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U4, 8U, PCMD3180_CHANNEL_ALL, 8U);
}

static void PCMD3180_SetCore16Plan(PCMD3180_ArrayModeConfigTypeDef *mode_config)
{
    /*
     * Core16 keeps four active mics per PCMD3180. This is the important V2
     * bandwidth trick: both TDM buses remain TDM8 at 192 kHz instead of forcing
     * all 16 core channels onto one high-speed bus.
     */
    const uint8_t core4_mask = PCMD3180_CHANNEL_1 |
                               PCMD3180_CHANNEL_2 |
                               PCMD3180_CHANNEL_3 |
                               PCMD3180_CHANNEL_4;

    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U1, 0U, core4_mask, 4U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U2, 4U, core4_mask, 4U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U3, 0U, core4_mask, 4U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U4, 4U, core4_mask, 4U);
}

static void PCMD3180_SetHf12Plan(PCMD3180_ArrayModeConfigTypeDef *mode_config)
{
    const uint8_t core3_mask = PCMD3180_CHANNEL_1 |
                               PCMD3180_CHANNEL_2 |
                               PCMD3180_CHANNEL_3;

    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U1, 0U, core3_mask, 3U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U2, 3U, core3_mask, 3U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U3, 0U, core3_mask, 3U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U4, 3U, core3_mask, 3U);
}

static void PCMD3180_SetHotspot8Plan(PCMD3180_ArrayModeConfigTypeDef *mode_config)
{
    const uint8_t core2_mask = PCMD3180_CHANNEL_1 | PCMD3180_CHANNEL_2;

    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U1, 0U, core2_mask, 2U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U2, 2U, core2_mask, 2U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U3, 0U, core2_mask, 2U);
    PCMD3180_SetDevicePlan(mode_config, PCMD3180_ARRAY_DEVICE_U4, 2U, core2_mask, 2U);
}

PCMD3180_StatusTypeDef PCMD3180_Init(PCMD3180_HandleTypeDef *handle,
                                     const PCMD3180_BusTypeDef *bus,
                                     uint8_t address7)
{
    if ((handle == NULL) ||
        (bus == NULL) ||
        (bus->write_reg == NULL) ||
        (bus->read_reg == NULL) ||
        (address7 > 0x7FU))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    handle->address7 = address7;
    handle->current_page = 0U;
    handle->configured = 0U;
    handle->last_status = PCMD3180_OK;
    handle->last_reg = 0U;
    handle->last_write_value = 0U;
    handle->last_read_value = 0U;
    handle->bus = *bus;

    return PCMD3180_OK;
}

void PCMD3180_GetDefaultConfig(PCMD3180_ConfigTypeDef *config)
{
    if (config == NULL)
    {
        return;
    }

    config->start_slot = 0U;
    config->input_channel_mask = PCMD3180_CHANNEL_ALL;
    config->output_channel_mask = PCMD3180_CHANNEL_ALL;
    config->logical_channel_count = 8U;
    config->slot_width = PCMD3180_SLOT_WIDTH_32_BITS;
    config->sample_rate_hz = PCMD3180_SAMPLE_RATE_48K;
    config->tdm_slots_per_bus = 8U;
    config->expected_bclk_hz = PCMD3180_CalculateBitClockHz(config->sample_rate_hz,
                                                            config->tdm_slots_per_bus,
                                                            config->slot_width);
    config->tdm_tx_offset = 1U;
    config->invert_bclk = 0U;
    config->invert_fsync = 0U;
    config->pdmclk_divider = PCMD3180_PDMCLK_DIV_64FS;
    config->pdmin_edge_mask = 0U;
    config->hpf_select = PCMD3180_HPF_96HZ_AT_48K;
    config->enable_micbias = 0U;
    /*
     * Match the H7-proven bring-up path: configure by write sequence first,
     * then use status snapshots for diagnosis. Immediate readback verification
     * makes the shared I2C bus much noisier and can fail while the device clock
     * domain is still settling.
     */
    config->verify_writes = 0U;
}

PCMD3180_StatusTypeDef PCMD3180_GetArrayModeConfig(PCMD3180_ArrayModeTypeDef mode,
                                                   PCMD3180_ArrayModeConfigTypeDef *mode_config)
{
    if (mode_config == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    PCMD3180_ClearModeConfig(mode_config);
    mode_config->mode = mode;
    mode_config->slot_width = PCMD3180_SLOT_WIDTH_16_BITS;

    switch (mode)
    {
    case PCMD3180_ARRAY_MODE_32CH_48K:
        mode_config->sample_rate_hz = PCMD3180_SAMPLE_RATE_48K;
        mode_config->frame_samples = 256U;
        mode_config->tdm_slots_per_bus = 16U;
        mode_config->physical_mic_count = 32U;
        mode_config->capture_channel_count = 32U;
        mode_config->recommended_pair_count = 96U;
        mode_config->band_low_hz = 1500U;
        mode_config->band_high_hz = 14000U;
        PCMD3180_Set32ChannelPlan(mode_config);
        break;

    case PCMD3180_ARRAY_MODE_CORE16_192K:
        mode_config->sample_rate_hz = PCMD3180_SAMPLE_RATE_192K;
        mode_config->frame_samples = 512U;
        mode_config->tdm_slots_per_bus = 8U;
        mode_config->physical_mic_count = 16U;
        mode_config->capture_channel_count = 16U;
        mode_config->recommended_pair_count = 48U;
        mode_config->band_low_hz = 6000U;
        mode_config->band_high_hz = 40000U;
        PCMD3180_SetCore16Plan(mode_config);
        break;

    case PCMD3180_ARRAY_MODE_HF12_192K:
        mode_config->sample_rate_hz = PCMD3180_SAMPLE_RATE_192K;
        mode_config->frame_samples = 512U;
        mode_config->tdm_slots_per_bus = 8U;
        mode_config->physical_mic_count = 12U;
        mode_config->capture_channel_count = 16U;
        mode_config->recommended_pair_count = 28U;
        mode_config->band_low_hz = 12000U;
        mode_config->band_high_hz = 30000U;
        PCMD3180_SetHf12Plan(mode_config);
        break;

    case PCMD3180_ARRAY_MODE_HOTSPOT8_192K:
        mode_config->sample_rate_hz = PCMD3180_SAMPLE_RATE_192K;
        mode_config->frame_samples = 512U;
        mode_config->tdm_slots_per_bus = 8U;
        mode_config->physical_mic_count = 8U;
        mode_config->capture_channel_count = 16U;
        mode_config->recommended_pair_count = 12U;
        mode_config->band_low_hz = 25000U;
        mode_config->band_high_hz = 80000U;
        PCMD3180_SetHotspot8Plan(mode_config);
        break;

    case PCMD3180_ARRAY_MODE_MID32_48K:
        mode_config->sample_rate_hz = PCMD3180_SAMPLE_RATE_48K;
        mode_config->frame_samples = 256U;
        mode_config->tdm_slots_per_bus = 16U;
        mode_config->physical_mic_count = 32U;
        mode_config->capture_channel_count = 32U;
        mode_config->recommended_pair_count = 112U;
        mode_config->band_low_hz = 2000U;
        mode_config->band_high_hz = 8000U;
        PCMD3180_Set32ChannelPlan(mode_config);
        break;

    case PCMD3180_ARRAY_MODE_LOW32_48K:
        mode_config->sample_rate_hz = PCMD3180_SAMPLE_RATE_48K;
        mode_config->frame_samples = 256U;
        mode_config->tdm_slots_per_bus = 16U;
        mode_config->physical_mic_count = 32U;
        mode_config->capture_channel_count = 32U;
        mode_config->recommended_pair_count = 120U;
        mode_config->band_low_hz = 800U;
        mode_config->band_high_hz = 4000U;
        PCMD3180_Set32ChannelPlan(mode_config);
        break;

    case PCMD3180_ARRAY_MODE_FAR32_48K:
        mode_config->sample_rate_hz = PCMD3180_SAMPLE_RATE_48K;
        mode_config->frame_samples = 256U;
        mode_config->tdm_slots_per_bus = 16U;
        mode_config->physical_mic_count = 32U;
        mode_config->capture_channel_count = 32U;
        mode_config->recommended_pair_count = 56U;
        mode_config->band_low_hz = 2000U;
        mode_config->band_high_hz = 10000U;
        PCMD3180_Set32ChannelPlan(mode_config);
        break;

    default:
        return PCMD3180_INVALID_ARGUMENT;
    }

    mode_config->expected_bclk_hz = PCMD3180_CalculateBitClockHz(mode_config->sample_rate_hz,
                                                                 mode_config->tdm_slots_per_bus,
                                                                 mode_config->slot_width);

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_BuildDeviceConfig(const PCMD3180_ArrayModeConfigTypeDef *mode_config,
                                                  uint32_t device_index,
                                                  PCMD3180_ConfigTypeDef *device_config)
{
    const PCMD3180_ArrayDevicePlanTypeDef *plan;

    if ((mode_config == NULL) ||
        (device_config == NULL) ||
        (device_index >= PCMD3180_ARRAY_DEVICE_COUNT))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    plan = &mode_config->devices[device_index];

    PCMD3180_GetDefaultConfig(device_config);
    device_config->start_slot = plan->start_slot;
    device_config->input_channel_mask = plan->input_channel_mask;
    device_config->output_channel_mask = plan->output_channel_mask;
    device_config->logical_channel_count = plan->mic_count;
    device_config->slot_width = mode_config->slot_width;
    device_config->sample_rate_hz = mode_config->sample_rate_hz;
    device_config->tdm_slots_per_bus = mode_config->tdm_slots_per_bus;
    device_config->expected_bclk_hz = mode_config->expected_bclk_hz;

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_HardwareReset(PCMD3180_HandleTypeDef *handle,
                                              uint32_t reset_low_ms,
                                              uint32_t settle_ms)
{
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_CheckHandle(handle);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    if (handle->bus.set_shutdown == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    handle->bus.set_shutdown(handle->bus.context, 1U);
    PCMD3180_Delay(handle, reset_low_ms);
    handle->bus.set_shutdown(handle->bus.context, 0U);
    PCMD3180_Delay(handle, settle_ms);

    handle->current_page = 0U;
    handle->configured = 0U;

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_SoftwareReset(PCMD3180_HandleTypeDef *handle)
{
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_CheckHandle(handle);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteRegister(handle, PCMD3180_REG_SW_RESET, PCMD3180_SW_RESET_ASSERT);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    PCMD3180_Delay(handle, 10U);
    handle->current_page = 0U;
    handle->configured = 0U;

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_Probe(PCMD3180_HandleTypeDef *handle)
{
    uint8_t page = 0U;
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_SelectPage(handle, 0U);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    return PCMD3180_ReadRegister(handle, PCMD3180_REG_PAGE_CFG, &page);
}

PCMD3180_StatusTypeDef PCMD3180_Configure(PCMD3180_HandleTypeDef *handle,
                                          const PCMD3180_ConfigTypeDef *config)
{
    uint8_t asi_cfg0;
    uint8_t pwr_cfg;
    uint8_t verify;
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_CheckHandle(handle);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    if ((config == NULL) ||
        (config->slot_width > PCMD3180_SLOT_WIDTH_32_BITS) ||
        (config->pdmclk_divider > PCMD3180_PDMCLK_DIV_32FS) ||
        (config->hpf_select > PCMD3180_HPF_192HZ_AT_48K) ||
        (config->tdm_tx_offset > 31U) ||
        ((uint32_t)config->start_slot + (PCMD3180_MAX_CHANNELS_PER_DEVICE - 1U) > PCMD3180_MAX_TDM_SLOT))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    verify = (config->verify_writes == 0U) ? 0U : 1U;

    status = PCMD3180_SelectPage(handle, 0U);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    /*
     * Match the proven H7 bring-up flow: reset each PCMD3180 before applying
     * the register table so a previous partial I2C transaction cannot leave
     * the device in a half-configured state.
     */
    status = PCMD3180_SoftwareReset(handle);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_SelectPage(handle, 0U);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_SLEEP_CFG, PCMD3180_SLEEP_CFG_WAKE, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    PCMD3180_Delay(handle, 10U);

    asi_cfg0 = PCMD3180_ASI_CFG0_TDM_MODE |
               (uint8_t)(((uint8_t)config->slot_width & 0x03U) << 4) |
               (uint8_t)((config->invert_fsync == 0U) ? 0U : 0x08U) |
               (uint8_t)((config->invert_bclk == 0U) ? 0U : 0x04U) |
               PCMD3180_ASI_CFG0_TX_HIGH_Z;

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_ASI_CFG0, asi_cfg0, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_ASI_CFG1, config->tdm_tx_offset, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_ASI_CFG2, 0U, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChannelSlots(handle, config->start_slot, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    /* Default slave-clock setup: BCLK/FSYNC must be provided by the host or another master. */
    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_MST_CFG0, 0U, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_MST_CFG1, 0U, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_CLK_SRC, 0U, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle,
                                   PCMD3180_REG_PDMCLK_CFG,
                                   (uint8_t)(PCMD3180_PDMCLK_CFG_RESET_MASK |
                                             ((uint8_t)config->pdmclk_divider & 0x03U)),
                                   verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_PDMIN_CFG, config->pdmin_edge_mask, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_GPIO_CFG0, 0U, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_GPO_CFG0, PCMD3180_GPO_CFG_PDMCLK_OUTPUT, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_GPO_CFG1, PCMD3180_GPO_CFG_PDMCLK_OUTPUT, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_GPO_CFG2, PCMD3180_GPO_CFG_PDMCLK_OUTPUT, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_GPO_CFG3, PCMD3180_GPO_CFG_PDMCLK_OUTPUT, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_GPI_CFG0, PCMD3180_GPI_CFG0_DEFAULT, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_GPI_CFG1, PCMD3180_GPI_CFG1_DEFAULT, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WritePdmInputConfig(handle, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_DSP_CFG0, (uint8_t)config->hpf_select, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_DSP_CFG1, 0U, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_IN_CH_EN, config->input_channel_mask, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_ASI_OUT_CH_EN, config->output_channel_mask, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    pwr_cfg = PCMD3180_PWR_PDM_AND_PLL |
              (uint8_t)((config->enable_micbias == 0U) ? 0U : PCMD3180_PWR_MICBIAS);

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_PWR_CFG, pwr_cfg, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    handle->configured = 1U;

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_ConfigureTdmChain(PCMD3180_HandleTypeDef *handles,
                                                  const uint8_t *addresses,
                                                  uint32_t device_count,
                                                  const PCMD3180_BusTypeDef *bus,
                                                  PCMD3180_ConfigTypeDef config)
{
    uint32_t index;

    if ((handles == NULL) ||
        (addresses == NULL) ||
        (bus == NULL) ||
        (device_count == 0U) ||
        ((uint32_t)config.start_slot + (device_count * PCMD3180_MAX_CHANNELS_PER_DEVICE) >
         (PCMD3180_MAX_TDM_SLOT + 1U)))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    for (index = 0U; index < device_count; index++)
    {
        PCMD3180_StatusTypeDef status;
        PCMD3180_ConfigTypeDef device_config = config;

        status = PCMD3180_Init(&handles[index], bus, addresses[index]);
        if (status != PCMD3180_OK)
        {
            return status;
        }

        device_config.start_slot = (uint8_t)(config.start_slot +
                                             (index * PCMD3180_MAX_CHANNELS_PER_DEVICE));

        status = PCMD3180_Configure(&handles[index], &device_config);
        if (status != PCMD3180_OK)
        {
            return status;
        }
    }

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_ConfigureArrayMode(PCMD3180_HandleTypeDef *handles,
                                                   const uint8_t *addresses,
                                                   const PCMD3180_BusTypeDef *bus,
                                                   PCMD3180_ArrayModeTypeDef mode)
{
    PCMD3180_ArrayModeConfigTypeDef mode_config;
    uint32_t index;
    PCMD3180_StatusTypeDef status;

    if ((handles == NULL) || (bus == NULL))
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    status = PCMD3180_GetArrayModeConfig(mode, &mode_config);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    for (index = 0U; index < PCMD3180_ARRAY_DEVICE_COUNT; index++)
    {
        PCMD3180_ConfigTypeDef device_config;
        uint8_t address7 = mode_config.devices[index].address7;

        if (addresses != NULL)
        {
            address7 = addresses[index];
        }

        status = PCMD3180_Init(&handles[index], bus, address7);
        if (status != PCMD3180_OK)
        {
            return status;
        }

        status = PCMD3180_BuildDeviceConfig(&mode_config, index, &device_config);
        if (status != PCMD3180_OK)
        {
            return status;
        }

        status = PCMD3180_Configure(&handles[index], &device_config);
        if (status != PCMD3180_OK)
        {
            return status;
        }
    }

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_SelectPage(PCMD3180_HandleTypeDef *handle,
                                           uint8_t page)
{
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_CheckHandle(handle);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = handle->bus.write_reg(handle->bus.context,
                                   handle->address7,
                                   PCMD3180_REG_PAGE_CFG,
                                   page);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    handle->current_page = page;

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_WriteRegister(PCMD3180_HandleTypeDef *handle,
                                              uint8_t reg,
                                              uint8_t value)
{
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_CheckHandle(handle);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    return handle->bus.write_reg(handle->bus.context, handle->address7, reg, value);
}

PCMD3180_StatusTypeDef PCMD3180_ReadRegister(PCMD3180_HandleTypeDef *handle,
                                             uint8_t reg,
                                             uint8_t *value)
{
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_CheckHandle(handle);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    if (value == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    return handle->bus.read_reg(handle->bus.context, handle->address7, reg, value);
}

PCMD3180_StatusTypeDef PCMD3180_ReadStatus(PCMD3180_HandleTypeDef *handle,
                                           PCMD3180_StatusSnapshotTypeDef *status_snapshot)
{
    PCMD3180_StatusTypeDef status;

    if (status_snapshot == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    status = PCMD3180_SelectPage(handle, 0U);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_DEV_STS0, &status_snapshot->dev_sts0);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_DEV_STS1, &status_snapshot->dev_sts1);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_ASI_STS, &status_snapshot->asi_sts);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_PDMCLK_CFG, &status_snapshot->pdmclk_cfg);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_PDMIN_CFG, &status_snapshot->pdmin_cfg);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_GPO_CFG0, &status_snapshot->gpo_cfg0);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_GPO_CFG1, &status_snapshot->gpo_cfg1);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_GPO_CFG2, &status_snapshot->gpo_cfg2);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_GPO_CFG3, &status_snapshot->gpo_cfg3);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_GPI_CFG0, &status_snapshot->gpi_cfg0);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_GPI_CFG1, &status_snapshot->gpi_cfg1);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_IN_CH_EN, &status_snapshot->in_ch_en);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_ASI_OUT_CH_EN, &status_snapshot->asi_out_ch_en);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_INT_LTCH0, &status_snapshot->int_latch0);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status_snapshot->int_latch1 = 0U;
    (void)PCMD3180_ReadRegister(handle, PCMD3180_REG_INT_LTCH1, &status_snapshot->int_latch1);

    status = PCMD3180_ReadRegister(handle, PCMD3180_REG_PWR_CFG, &status_snapshot->pwr_cfg);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    return PCMD3180_ReadRegister(handle, PCMD3180_REG_GPI_MON, &status_snapshot->gpi_mon);
}

uint8_t PCMD3180_CountEnabledChannels(uint8_t channel_mask)
{
    uint8_t count = 0U;

    while (channel_mask != 0U)
    {
        count = (uint8_t)(count + (channel_mask & 0x01U));
        channel_mask = (uint8_t)(channel_mask >> 1U);
    }

    return count;
}

uint8_t PCMD3180_GetSlotWidthBits(PCMD3180_SlotWidthTypeDef slot_width)
{
    switch (slot_width)
    {
    case PCMD3180_SLOT_WIDTH_16_BITS:
        return 16U;

    case PCMD3180_SLOT_WIDTH_20_BITS:
        return 20U;

    case PCMD3180_SLOT_WIDTH_24_BITS:
        return 24U;

    case PCMD3180_SLOT_WIDTH_32_BITS:
        return 32U;

    default:
        return 0U;
    }
}

uint32_t PCMD3180_CalculateBitClockHz(uint32_t sample_rate_hz,
                                      uint8_t tdm_slots_per_bus,
                                      PCMD3180_SlotWidthTypeDef slot_width)
{
    uint8_t slot_width_bits = PCMD3180_GetSlotWidthBits(slot_width);

    if ((sample_rate_hz == 0U) || (tdm_slots_per_bus == 0U) || (slot_width_bits == 0U))
    {
        return 0U;
    }

    return sample_rate_hz * (uint32_t)tdm_slots_per_bus * (uint32_t)slot_width_bits;
}
