#include "sensor.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>

#define SENSOR_PIN 5   // PE5

static const struct device *gpioe_dev;
static uint16_t debounce_ms;
static bool initialized = false;
static bool stable_state = false;
static bool last_raw_state = false;
static uint16_t debounce_counter = 0;

static bool read_pin(void)
{
    return gpio_pin_get(gpioe_dev, SENSOR_PIN);
}

void photo_sensor_init(uint16_t db_ms)
{
    gpioe_dev = device_get_binding("GPIOE");
    if (!gpioe_dev) {
        printk("ERROR: GPIOE device not found\n");
        return;
    } else {
    printk("GPIOE device found!\n");
    return;
}

    debounce_ms = db_ms;

    gpio_pin_configure(gpioe_dev, SENSOR_PIN, GPIO_INPUT | GPIO_PULL_UP);
    bool val = gpio_pin_get(gpioe_dev, SENSOR_PIN);
printk("PE5 state: %d\n", val);

    last_raw_state = read_pin();
    stable_state = last_raw_state;

    initialized = true;
}

bool photo_sensor_read(void)
{
    if (!initialized) return false;
    return read_pin();
}

bool photo_sensor_read_debounced(void)
{
    return stable_state;
}

void photo_sensor_tick_1ms(void)
{
    if (!initialized) return;

    bool raw = read_pin();

    if (raw != last_raw_state) {
        last_raw_state = raw;
        debounce_counter = 0;
    } else {
        if (debounce_counter < debounce_ms)
            debounce_counter++;

        if (debounce_counter == debounce_ms)
            stable_state = raw;
    }
}
