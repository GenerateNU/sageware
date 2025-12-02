// ADC setup and current sensing logic (including the 2.4A threshold detection for the cutting motor)

// ADC3; each pin
// five channels in adc pin
// figure out how to read current and temperature with the pins
// when we reach 3.2V, turn off motor driver
// 16-bit adc

#include "adc.h"
#include "stm32h7xx_hal.h"

// ==========================================================
// Internal handles
// ==========================================================
static ADC_HandleTypeDef hadc1;


// ==========================================================
// ADC Initialization (STM32H753ZIT6 / STM32H7 HAL)
// ==========================================================
void ADC_Init(void)
{
    __HAL_RCC_ADC12_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    // -------------------------------------------
    // 1. Configure GPIO (PC2 -> ADC1_IN3)
    // -------------------------------------------
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin  = GPIO_PIN_2;          // PC2
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;    
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    // -------------------------------------------
    // 2. Configure ADC1 instance
    // -------------------------------------------
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler           = ADC_CLOCK_ASYNC_DIV2;
    hadc1.Init.Resolution               = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode             = ADC_SCAN_DISABLE;
    hadc1.Init.EOCSelection             = ADC_EOC_SINGLE_CONV;
    hadc1.Init.LowPowerAutoWait         = DISABLE;
    hadc1.Init.ContinuousConvMode       = DISABLE;    // single conversion mode
    hadc1.Init.NbrOfConversion          = 1;
    hadc1.Init.ExternalTrigConv         = ADC_SOFTWARE_START;
    hadc1.Init.ExternalTrigConvEdge     = ADC_EXTERNALTRIGCONVEDGE_NONE;
    hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
    hadc1.Init.Overrun                  = ADC_OVR_DATA_PRESERVED;
    hadc1.Init.RightBitShift            = ADC_RIGHTBITSHIFT_NONE;
    hadc1.Init.OversamplingMode         = DISABLE;

    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();    // You should define this
    }

    // -------------------------------------------
    // 3. Configure Channel 3 (PC2_C)
    // -------------------------------------------
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel      = ADC_CHANNEL_3;
    sConfig.Rank         = ADC_REGULAR_RANK_1;
    sConfig.SamplingTime = ADC_SAMPLETIME_16CYCLES; // safe default
    sConfig.OffsetNumber = ADC_OFFSET_NONE;
    sConfig.Offset       = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}


// ==========================================================
// Read one conversion from ADC1 Channel 3
// ==========================================================
uint16_t ADC_ReadCurrentRaw(void)
{
    HAL_ADC_Start(&hadc1);

    if (HAL_ADC_PollForConversion(&hadc1, 5) != HAL_OK)
        return 0;   // timeout or error

    uint16_t val = HAL_ADC_GetValue(&hadc1);

    HAL_ADC_Stop(&hadc1);
    return val;
}
