// ADC setup and current sensing logic (including the 2.4A threshold detection for the cutting motor)

// ADC3;
// five channels in adc pin
// figure out how to read current and temperature with the pins
// when we reach 3.2V, turn off motor driver
// 16-bit adc
#include "adc.h"
#include <zephyr/logging/log.h>



LOG_MODULE_REGISTER(adc_sense, LOG_LEVEL_INF);

/* ADC device and buffer */
static const struct device *adc_dev;
static struct adc_sequence seq;
static int16_t adc_buf;

/* ------------------------------------------------------------
   Initialize ADC
   ------------------------------------------------------------ */
void CurrentSense_Init(void)
{
    adc_dev = DEVICE_DT_GET(CURRENT_SENSE_ADC_NODE);

    printk("ADC device: %p", adc_dev);

    if (!device_is_ready(adc_dev)) {
        LOG_ERR("ADC device not ready");
    
        return;
    }

    /* Configure ADC channel */
    struct adc_channel_cfg ch_cfg = {
        .gain             = ADC_GAIN_1,
        .reference        = ADC_REF_INTERNAL,
        .acquisition_time = ADC_ACQ_TIME_DEFAULT,
        .channel_id       = CURRENT_SENSE_CHANNEL,
    };

    if (adc_channel_setup(adc_dev, &ch_cfg) != 0) {
        LOG_ERR("Failed to setup ADC channel");
        return;
    }

    /* Prepare ADC sequence */
    seq.channels    = BIT(CURRENT_SENSE_CHANNEL);
    seq.buffer      = &adc_buf;
    seq.buffer_size = sizeof(adc_buf);
    seq.resolution  = ADC_RESOLUTION;

    LOG_INF("ADC sequence channel mask: 0x%x", seq.channels);
}

/* ------------------------------------------------------------
   Read current (Amps)
   ------------------------------------------------------------ */
float CurrentSense_ReadCurrent(void)
{
    if (adc_read(adc_dev, &seq) == 0) {
        float voltage = ((float)adc_buf * ADC_REFERENCE_VOLTAGE) / (1 << ADC_RESOLUTION);
        return voltage / CURRENT_SENSE_GAIN;
    } else {
        LOG_WRN("ADC read failed");
        return 0.0f;
    }
}

/* ------------------------------------------------------------
   Check overcurrent
   ------------------------------------------------------------ */
bool CurrentSense_IsOvercurrent(void)
{
    float current = CurrentSense_ReadCurrent();
    return (current >= MOTOR_OVERCURRENT_LIMIT_A);
}
