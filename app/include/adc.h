// ADC setup and current sensing logic (including the 2.4A threshold detection for the cutting motor)
// at 3600 level count, we've hit our 2.4A threshold (w/ 0.075Ohm shunt)
#ifndef ADC_CURRENT_SENSE_H
#define ADC_CURRENT_SENSE_H

#include "stm32h7xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/* ============================================================
   Configuration Section — ADC hardware
   ============================================================ */

/**
 * @brief ADC instance used for current sensing.
 */
#define CURRENT_SENSE_ADC          ADC3

/**
 * @brief ADC channel connected to the current-sense line.
 * Modify if using a different pin.
 */
#define CURRENT_SENSE_ADC_CHANNEL  ADC_CHANNEL_3


/* ------------------------------------------------------------
   16-bit ADC Characteristics
   ------------------------------------------------------------ */

/**
 * @brief Raw count range for 16-bit ADC.
 * 
 * For true 16-bit resolution:
 *   Counts = 0 to 65535 → 65536 steps
 */
#define ADC_RESOLUTION_COUNTS      65536.0f

/**
 * @brief Analog reference (VDDA)
 */
#define ADC_REFERENCE_VOLTAGE      3.300f   // 3.3V


/* ------------------------------------------------------------
   Current-Sense Hardware Parameters
   ------------------------------------------------------------ */

/**
 * @brief Gain of the current-sense amplifier front-end.
 * 
 * Example:
 *   0.075Ω shunt, 20× gain → V = I * 0.1 * 20 = I * 2.0
 */
#define CURRENT_SENSE_GAIN_V_PER_A 2.0f


/* ------------------------------------------------------------
   Overcurrent Threshold (Cutting Motor)
   ------------------------------------------------------------ */

#define MOTOR_OVERCURRENT_LIMIT_A   2.4f

#define MOTOR_OVERCURRENT_LIMIT_V  \
    (MOTOR_OVERCURRENT_LIMIT_A * CURRENT_SENSE_GAIN_V_PER_A)

/**
 * @brief Convert threshold voltage to 16-bit counts.
 */
#define MOTOR_OVERCURRENT_LIMIT_ADC \
    ((uint16_t)((MOTOR_OVERCURRENT_LIMIT_V / ADC_REFERENCE_VOLTAGE) * ADC_RESOLUTION_COUNTS))


/* ============================================================
   Public Functions
   ============================================================ */

/**
 * @brief Initialize ADC hardware for current sensing.
 */
void CurrentSense_ADC_Init(void);


/**
 * @brief Perform one ADC conversion and return raw 16-bit value.
 *
 * @return uint16_t Raw ADC value (0–65535)
 */
uint16_t CurrentSense_ReadRaw(void);


/**
 * @brief Convert raw 16-bit ADC reading to current (Amps).
 */
static inline float CurrentSense_ToAmps(uint16_t adc_val)
{
    float voltage = (adc_val * ADC_REFERENCE_VOLTAGE) / ADC_RESOLUTION_COUNTS;
    return voltage / CURRENT_SENSE_GAIN_V_PER_A;
}


/**
 * @brief Check if the 2.4 A overcurrent threshold is exceeded.
 */
static inline bool CurrentSense_IsOvercurrent(uint16_t adc_val)
{
    return (adc_val >= MOTOR_OVERCURRENT_LIMIT_ADC);
}

#endif // ADC_CURRENT_SENSE_H
