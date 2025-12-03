#include <stdint.h>
// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/adc.h>
#include <zephyr/sys/printk.h>

#include <stdio.h>

#define ADC_NODE DT_ALIAS(adc3)
#if !DT_NODE_HAS_STATUS(ADC_NODE, okay)
#error "ADC1 not enabled in device tree"
#endif

#define ADC_RESOLUTION 16
#define ADC_GAIN ADC_GAIN_1
#define ADC_REFERENCE ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT

#define ADC_CHANNEL_ID 1   // Channel you want to read
#define BUFFER_SIZE 1

static int16_t sample_buffer[BUFFER_SIZE];

void main(void)
{
    const struct device *adc_dev = DEVICE_DT_GET(ADC_NODE);
    if (!device_is_ready(adc_dev)) {
        printk("ADC device not ready\n");
        return;
    }

    struct adc_channel_cfg channel_cfg = {
        .gain             = ADC_GAIN,
        .reference        = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id       = ADC_CHANNEL_ID,
        .differential     = 0,
    };

    int ret = adc_channel_setup(adc_dev, &channel_cfg);
    if (ret) {
        printk("Error setting up ADC channel %d\n", ret);
        return;
    }

    struct adc_sequence sequence = {
        .channels    = BIT(ADC_CHANNEL_ID),
        .buffer      = sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution  = ADC_RESOLUTION,
    };

    while (1) {
        ret = adc_read(adc_dev, &sequence);
        if (ret == 0) {
            printk("ADC Value: %d\n", sample_buffer[0]);
        } else {
            printk("ADC read error: %d\n", ret);
        }
        k_sleep(K_MSEC(1000));
    }
}












/******** */

/*
#define ADC_NODE        DT_ALIAS(adc_ch1)
#define ADC_RESOLUTION  16
#define NUM_CHANNELS    5*/

/* channels 0,1,2,3,4 
#define MY_CHANNELS (BIT(0) | BIT(1) | BIT(2) | BIT(3) | BIT(4))*/
/*
static int16_t samples[NUM_CHANNELS];

struct adc_sequence {
    int channels;
    int resolution;
    int buffer;
    int buffer_size
};

void main(void)
{
    const struct device *adc_dev = DT_NODELABEL(adc3);

    if (!device_is_ready(adc_dev)) {
        printk("ADC not ready\n");
        return;
    }

    adc_sequence seq = {
        .channels = MY_CHANNELS,
        .resolution = ADC_RESOLUTION,
        .buffer = samples,
        .buffer_size = sizeof(samples),
    };

    while (1) {
        int ret = adc_read(adc_dev, &seq);

        if (ret == 0) { // no errors
            for (int i = 0; i < NUM_CHANNELS; i++) {
                printk("Channel %d: %d\n", i, samples[i]); // prints the raw ADC value measured at each channel
            }
        } else {
            printk("ADC read error %d\n", ret);
        }

        k_sleep(K_MSEC(500));
    }
}
*/