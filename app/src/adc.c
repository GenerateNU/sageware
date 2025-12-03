// // ADC setup and current sensing logic (including the 2.4A threshold detection for the cutting motor)

// // ADC3;
// // five channels in adc pin
// // figure out how to read current and temperature with the pins
// // when we reach 3.2V, turn off motor driver
// // 16-bit adc
#include "adc.h"
 #include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(adc_sense, LOG_LEVEL_INF);

/* ADC device + sequence */
static const struct device *adc_dev;
static struct adc_dt_spec current_adc;
static int16_t adc_buf;

void CurrentSense_Init(void)
{
    /* Get ADC device spec using device tree */
    current_adc.dev = DEVICE_DT_GET(CURRENT_SENSE_ADC_NODE);
    current_adc.channel_id = CURRENT_SENSE_CHANNEL;
    current_adc.resolution = ADC_RESOLUTION;

    if (!device_is_ready(current_adc.dev)) {
        LOG_ERR("ADC device not ready");
        return;
    }

    /* Setup the ADC channel */
    if (adc_channel_setup_dt(&current_adc) != 0) {
        LOG_ERR("Failed to setup ADC channel");
        return;
    }

    LOG_INF("ADC device ready: %p, channel: %d", current_adc.dev, CURRENT_SENSE_CHANNEL);
}

float CurrentSense_ReadCurrent(void)
{
    int ret;
    int32_t raw_mv;
    struct adc_sequence seq = {
        .channels = BIT(current_adc.channel_id),
        .buffer = &adc_buf,
        .buffer_size = sizeof(adc_buf),
        .resolution = current_adc.resolution,
    };

    ret = adc_read_dt(&current_adc, &seq);
    if (ret != 0) {
        LOG_WRN("ADC read failed (%d)", ret);
        return 0.0f;
    }

    /* Convert raw ADC to mV */
    raw_mv = adc_raw_to_millivolts_dt(&current_adc, adc_buf);
    float voltage = raw_mv / 1000.0f;

    /* Convert voltage to current */
    return voltage / CURRENT_SENSE_GAIN;
}

bool CurrentSense_IsOvercurrent(void)
{
    float current = CurrentSense_ReadCurrent();
    return (current >= MOTOR_OVERCURRENT_LIMIT_A);
}
