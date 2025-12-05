#ifndef SENSOR_H
#define SENSOR_H

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Initialize the photoelectric sensor on PE5.
 * @param debounce_ms Debounce time in milliseconds.
 */
void photo_sensor_init(uint16_t debounce_ms);

/**
 * @brief Read the raw sensor state (non-debounced).
 */
bool photo_sensor_read(void);

/**
 * @brief Read the debounced sensor state.
 */
bool photo_sensor_read_debounced(void);

/**
 * @brief Call every 1 ms to update debouncing.
 */
void photo_sensor_tick_1ms(void);

#endif // SENSOR_H
