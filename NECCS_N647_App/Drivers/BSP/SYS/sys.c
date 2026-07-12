/**
  ******************************************************************************
  * @file    sys.c
  * @brief   Debug system clock and SAI audio clock configuration.
  ******************************************************************************
  */

#include "sys.h"

#define SYS_AUDIO_PLL2_FREQ_HZ  245760000UL

/**
 * @brief   配置系统时钟
 * @param   无
 * @retval  无
 */
void sys_clock_config_debug(void)
{
    RCC_OscInitTypeDef rcc_osc_init_struct = {0};
    RCC_ClkInitTypeDef rcc_clk_init_struct = {0};

    HAL_PWREx_ConfigSupply(PWR_SMPS_SUPPLY);

    rcc_osc_init_struct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    rcc_osc_init_struct.HSIState = RCC_HSI_ON;
    rcc_osc_init_struct.HSIDiv = RCC_HSI_DIV1;
    rcc_osc_init_struct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    rcc_osc_init_struct.PLL1.PLLState = RCC_PLL_NONE;
    rcc_osc_init_struct.PLL2.PLLState = RCC_PLL_NONE;
    rcc_osc_init_struct.PLL3.PLLState = RCC_PLL_NONE;
    rcc_osc_init_struct.PLL4.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&rcc_osc_init_struct);

    HAL_RCC_GetClockConfig(&rcc_clk_init_struct);
    if ((rcc_clk_init_struct.CPUCLKSource == RCC_CPUCLKSOURCE_IC1) || (rcc_clk_init_struct.SYSCLKSource == RCC_SYSCLKSOURCE_IC2_IC6_IC11))
    {
        rcc_clk_init_struct.ClockType = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_SYSCLK;
        rcc_clk_init_struct.CPUCLKSource = RCC_CPUCLKSOURCE_HSI;
        rcc_clk_init_struct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
        HAL_RCC_ClockConfig(&rcc_clk_init_struct);
    }

    rcc_osc_init_struct.OscillatorType = RCC_OSCILLATORTYPE_NONE;
    rcc_osc_init_struct.PLL1.PLLState = RCC_PLL_ON;
    rcc_osc_init_struct.PLL1.PLLSource = RCC_PLLSOURCE_HSI;
    rcc_osc_init_struct.PLL1.PLLM = 4;
    rcc_osc_init_struct.PLL1.PLLN = 75;
    rcc_osc_init_struct.PLL1.PLLFractional = 0;
    rcc_osc_init_struct.PLL1.PLLP1 = 1;
    rcc_osc_init_struct.PLL1.PLLP2 = 1;
    rcc_osc_init_struct.PLL2.PLLState = RCC_PLL_NONE;
    rcc_osc_init_struct.PLL3.PLLState = RCC_PLL_NONE;
    rcc_osc_init_struct.PLL4.PLLState = RCC_PLL_NONE;
    HAL_RCC_OscConfig(&rcc_osc_init_struct);

    rcc_clk_init_struct.ClockType = RCC_CLOCKTYPE_CPUCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_PCLK5 | RCC_CLOCKTYPE_PCLK4;
    rcc_clk_init_struct.CPUCLKSource = RCC_CPUCLKSOURCE_IC1;
    rcc_clk_init_struct.SYSCLKSource = RCC_SYSCLKSOURCE_IC2_IC6_IC11;
    rcc_clk_init_struct.AHBCLKDivider = RCC_HCLK_DIV2;
    rcc_clk_init_struct.APB1CLKDivider = RCC_APB1_DIV1;
    rcc_clk_init_struct.APB2CLKDivider = RCC_APB2_DIV1;
    rcc_clk_init_struct.APB4CLKDivider = RCC_APB4_DIV1;
    rcc_clk_init_struct.APB5CLKDivider = RCC_APB5_DIV1;
    rcc_clk_init_struct.IC1Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    rcc_clk_init_struct.IC1Selection.ClockDivider = 2;
    rcc_clk_init_struct.IC2Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    rcc_clk_init_struct.IC2Selection.ClockDivider = 3;
    rcc_clk_init_struct.IC6Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    rcc_clk_init_struct.IC6Selection.ClockDivider = 4;
    rcc_clk_init_struct.IC11Selection.ClockSelection = RCC_ICCLKSOURCE_PLL1;
    rcc_clk_init_struct.IC11Selection.ClockDivider = 3;
    HAL_RCC_ClockConfig(&rcc_clk_init_struct);
}

/**
 * @brief   Configure the audio clock tree used by SAI1.
 * @param   sai1_kernel_hz  Target SAI1 kernel clock (must divide the fixed
 *                          245.76 MHz PLL2 exactly, e.g. 12288000/24576000).
 * @retval  1: clock is ready, 0: clock configuration failed
 */
uint8_t sys_audio_clock_config(uint32_t sai1_kernel_hz)
{
    RCC_OscInitTypeDef rcc_osc_init_struct = {0};
    RCC_PeriphCLKInitTypeDef periph_clk_init_struct = {0};
    uint32_t divider;

    if ((sai1_kernel_hz == 0U) ||
        ((SYS_AUDIO_PLL2_FREQ_HZ % sai1_kernel_hz) != 0U))
    {
        return 0U;
    }
    divider = SYS_AUDIO_PLL2_FREQ_HZ / sai1_kernel_hz;
    if ((divider == 0U) || (divider > 256U))
    {
        return 0U;
    }

    if ((HAL_RCCEx_GetPLL2CLKFreq() == SYS_AUDIO_PLL2_FREQ_HZ) &&
        (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI1) == sai1_kernel_hz))
    {
        return 1U;
    }

    if (HAL_RCCEx_GetPLL2CLKFreq() != SYS_AUDIO_PLL2_FREQ_HZ)
    {
        rcc_osc_init_struct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
        rcc_osc_init_struct.HSIState = RCC_HSI_ON;
        rcc_osc_init_struct.HSIDiv = RCC_HSI_DIV1;
        rcc_osc_init_struct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
        rcc_osc_init_struct.PLL1.PLLState = RCC_PLL_NONE;
        rcc_osc_init_struct.PLL2.PLLState = RCC_PLL_ON;
        rcc_osc_init_struct.PLL2.PLLSource = RCC_PLLSOURCE_HSI;
        rcc_osc_init_struct.PLL2.PLLM = 5;
        rcc_osc_init_struct.PLL2.PLLN = 96;
        rcc_osc_init_struct.PLL2.PLLFractional = 0;
        rcc_osc_init_struct.PLL2.PLLP1 = 5;
        rcc_osc_init_struct.PLL2.PLLP2 = 1;
        rcc_osc_init_struct.PLL3.PLLState = RCC_PLL_NONE;
        rcc_osc_init_struct.PLL4.PLLState = RCC_PLL_NONE;
        if (HAL_RCC_OscConfig(&rcc_osc_init_struct) != HAL_OK)
        {
            return 0U;
        }
    }

    periph_clk_init_struct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
    periph_clk_init_struct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_IC7;
    periph_clk_init_struct.ICSelection[RCC_IC7].ClockSelection = RCC_ICCLKSOURCE_PLL2;
    periph_clk_init_struct.ICSelection[RCC_IC7].ClockDivider = divider;
    if (HAL_RCCEx_PeriphCLKConfig(&periph_clk_init_struct) != HAL_OK)
    {
        return 0U;
    }

    return ((HAL_RCCEx_GetPLL2CLKFreq() == SYS_AUDIO_PLL2_FREQ_HZ) &&
            (HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_SAI1) == sai1_kernel_hz)) ? 1U : 0U;
}
