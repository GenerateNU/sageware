#include "sensor.h"
#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>

#define SENSOR_PIN_COUNT 2

// GPIOE pins
static const uint32_t sensor_pins[SENSOR_PIN_COUNT] = {5, 6};
static const struct device *gpioe_dev;

static uint16_t debounce_ms;
static bool initialized = false;
static bool stable_state = false;
static bool last_raw_state = false;
static uint16_t debounce_counter = 0;

// Helper: read both pins ORed
static bool read_pins(void)
{
    bool state = false;
    for (int i = 0; i < SENSOR_PIN_COUNT; i++) {
        state |= gpio_pin_get(gpioe_dev, sensor_pins[i]);
    }
    return state;
}

void photo_sensor_init(uint16_t db_ms)
{
    gpioe_dev = device_get_binding("GPIOE");
    if (!gpioe_dev) {
        printk("ERROR: GPIOE device not found\n");
        return;
    }

    debounce_ms = db_ms;

    for (int i = 0; i < SENSOR_PIN_COUNT; i++) {
        gpio_pin_configure(gpioe_dev, sensor_pins[i], GPIO_INPUT);
    }

    last_raw_state = read_pins();
    stable_state = last_raw_state;

    initialized = true;
}

bool photo_sensor_read(void)
{
    if (!initialized) return false;
    return read_pins();
}

bool photo_sensor_read_debounced(void)
{
    return stable_state;
}

void photo_sensor_tick_1ms(void)
{
    if (!initialized) return;

    bool raw = read_pins();

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
