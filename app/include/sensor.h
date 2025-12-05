#ifndef SENSOR_H
#define SENSOR_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdint.h>
#include <stdbool.h>

/* Photoelectric sensor driver for two pins (PE5 and PE6) */
void photo_sensor_init(uint16_t debounce_ms);
bool photo_sensor_read(void);              // raw state
bool photo_sensor_read_debounced(void);    // debounced state
void photo_sensor_tick_1ms(void);

#endif // SENSOR_H
