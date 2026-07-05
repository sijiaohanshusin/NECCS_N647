#include "pcmd3180.h"

#define PCMD3180_MAX_CHANNELS_PER_DEVICE     8U
#define PCMD3180_MAX_TDM_SLOT                63U
#define PCMD3180_ASI_CFG0_TDM_MODE           0x00U
#define PCMD3180_ASI_CFG0_TX_HIGH_Z          0x01U

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
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_WriteRegister(handle, reg, value);
    if ((status != PCMD3180_OK) || (verify == 0U))
    {
        return status;
    }

    status = PCMD3180_ReadRegister(handle, reg, &readback);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    return (readback == value) ? PCMD3180_OK : PCMD3180_VERIFY_ERROR;
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

static uint8_t PCMD3180_BuildAsiCfg0(const PCMD3180_ConfigTypeDef *config)
{
    return (uint8_t)(PCMD3180_ASI_CFG0_TDM_MODE |
                    (uint8_t)(((uint8_t)config->slot_width & 0x03U) << 4) |
                    (uint8_t)((config->invert_fsync == 0U) ? 0U : 0x08U) |
                    (uint8_t)((config->invert_bclk == 0U) ? 0U : 0x04U) |
                    PCMD3180_ASI_CFG0_TX_HIGH_Z);
}

static PCMD3180_StatusTypeDef PCMD3180_WriteAsiRouting(PCMD3180_HandleTypeDef *handle,
                                                       const PCMD3180_ConfigTypeDef *config,
                                                       uint8_t verify)
{
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_ASI_CFG0, PCMD3180_BuildAsiCfg0(config), verify);
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

    return PCMD3180_WriteChannelSlots(handle, config->start_slot, verify);
}

static PCMD3180_StatusTypeDef PCMD3180_ApplyAsiRouting(PCMD3180_HandleTypeDef *handle,
                                                       const PCMD3180_ConfigTypeDef *config,
                                                       uint8_t verify)
{
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_SelectPage(handle, 0U);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    return PCMD3180_WriteAsiRouting(handle, config, verify);
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
    /* Optional readback verification is left off for the normal ordered-write path. */
    config->verify_writes = 0U;
    config->defer_power_up = 0U;
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
    uint8_t pwr_cfg;
    uint8_t verify;
    uint8_t defer_power_up;
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
    defer_power_up = (config->defer_power_up == 0U) ? 0U : 1U;

    status = PCMD3180_SelectPage(handle, 0U);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    /* Reset before programming so stale partial transactions cannot leak across configurations. */
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

    /* Program the register table after the device exits sleep mode. */
    status = PCMD3180_WriteChecked(handle,
                                   PCMD3180_REG_SLEEP_CFG,
                                   PCMD3180_SLEEP_CFG_WAKE,
                                   verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }
    PCMD3180_Delay(handle, 10U);

    status = PCMD3180_ApplyAsiRouting(handle, config, verify);
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

    pwr_cfg = (defer_power_up == 0U) ?
              (PCMD3180_PWR_PDM_AND_PLL |
               (uint8_t)((config->enable_micbias == 0U) ? 0U : PCMD3180_PWR_MICBIAS)) :
              0U;

    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_PWR_CFG, pwr_cfg, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    if ((defer_power_up == 0U) && (pwr_cfg != 0U))
    {
        /* Re-apply ASI routing after power-up so slot registers stay aligned with the mode. */
        PCMD3180_Delay(handle, 10U);
        status = PCMD3180_SelectPage(handle, 0U);
        if (status != PCMD3180_OK)
        {
            return status;
        }

        status = PCMD3180_ApplyAsiRouting(handle, config, verify);
        if (status != PCMD3180_OK)
        {
            return status;
        }
    }

    handle->configured = 1U;

    return PCMD3180_OK;
}

PCMD3180_StatusTypeDef PCMD3180_Activate(PCMD3180_HandleTypeDef *handle,
                                         const PCMD3180_ConfigTypeDef *config)
{
    uint8_t pwr_cfg;
    uint8_t verify;
    PCMD3180_StatusTypeDef status;

    status = PCMD3180_CheckHandle(handle);
    if (status != PCMD3180_OK)
    {
        return status;
    }
    if (config == NULL)
    {
        return PCMD3180_INVALID_ARGUMENT;
    }

    verify = (config->verify_writes == 0U) ? 0U : 1U;

    status = PCMD3180_SelectPage(handle, 0U);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    /* Power up a previously configured device once its audio clocks are available. */
    pwr_cfg = PCMD3180_PWR_PDM_AND_PLL |
              (uint8_t)((config->enable_micbias == 0U) ? 0U : PCMD3180_PWR_MICBIAS);
    status = PCMD3180_WriteChecked(handle, PCMD3180_REG_PWR_CFG, pwr_cfg, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }
    PCMD3180_Delay(handle, 10U);

    status = PCMD3180_SelectPage(handle, 0U);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    status = PCMD3180_ApplyAsiRouting(handle, config, verify);
    if (status != PCMD3180_OK)
    {
        return status;
    }

    handle->configured = 1U;
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
