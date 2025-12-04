
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
static uint16_t adc_buf;  // Changed to unsigned

void CurrentSense_Init(int current_sense_channel)
{
    /* Get ADC device spec using device tree */
    current_adc = (struct adc_dt_spec) {
        .dev = DEVICE_DT_GET(CURRENT_SENSE_ADC_NODE),
        .channel_id = current_sense_channel, // changed to variable
        .resolution = ADC_RESOLUTION,
        .oversampling = 0,
        .channel_cfg = {
            .gain = ADC_GAIN_1,
            .reference = ADC_REF_VDD_1,
            .acquisition_time = ADC_ACQ_TIME_DEFAULT,
            .differential = 0,
        },
        .vref_mv = 3300,  // 3.3V reference in millivolts
    };
    if (!device_is_ready(current_adc.dev)) {
        LOG_ERR("ADC device not ready");
        return;
    }
    /* Setup the ADC channel */
    if (adc_channel_setup_dt(&current_adc) != 0) {
        LOG_ERR("Failed to setup ADC channel");
        return;
    }
    LOG_INF("ADC device ready: %p, channel: %d, vref: %d mV",
            current_adc.dev, current_sense_channel, current_adc.vref_mv);
    //return current_adc.vref_mv;
}
 float CurrentSense_ReadCurrent(void)
 {
     int ret;
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
     /* Manual conversion: raw value / max_value * reference_voltage */
     uint32_t max_value = (1 << ADC_RESOLUTION) - 1;  // 65535 for 16-bit
     float voltage = ((float)adc_buf / (float)max_value) * ADC_REFERENCE_VOLTAGE;
     LOG_INF("ADC raw: %d, max: %u, voltage: %d mV",
             (int)adc_buf, max_value, (int)(voltage * 1000));
     /* Convert voltage to current */
    // return voltage / CURRENT_SENSE_GAIN;
     return voltage;
 }
bool CurrentSense_IsOvercurrent(void)
{
    float current = CurrentSense_ReadCurrent();
    return (current >= MOTOR_OVERCURRENT_LIMIT_A);
}