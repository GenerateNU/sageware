#ifndef ADC_CURRENT_SENSE_H
#define ADC_CURRENT_SENSE_H

#include <zephyr/device.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/kernel.h>
#include <stdbool.h>

/* ------------------------------------------------------------
   ADC Configuration
   ------------------------------------------------------------ */
#define CURRENT_SENSE_ADC_NODE      DT_NODELABEL(adc3)   // ADC3 on PF3
#define CURRENT_SENSE_CHANNEL       5                   // PF3 = ADC3_INP5
#define ADC_RESOLUTION              16                  // 16-bit ADC
#define ADC_REFERENCE_VOLTAGE       3.3f                // VDDA
#define CURRENT_SENSE_GAIN          2.0f                // V per Amp
#define MOTOR_OVERCURRENT_LIMIT_A   2.4f                // Motor threshold

/* ------------------------------------------------------------
   Public API
   ------------------------------------------------------------ */

/**
 * @brief Initialize ADC for current sensing.
 * Sets up ADC channel and prepares the sequence for sampling.
 */
void CurrentSense_Init(void);

/**
 * @brief Read the current from the ADC and convert to Amps.
 * @return Current in Amps (float)
 */
float CurrentSense_ReadCurrent(void);

/**
 * @brief Check if the current exceeds the overcurrent threshold.
 * @return true if overcurrent detected, false otherwise
 */
bool CurrentSense_IsOvercurrent(void);

#endif // ADC_CURRENT_SENSE_H
