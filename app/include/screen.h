#ifndef SCREEN_H_
#define SCREEN_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>

/**
 * Initialize the ILI9340 display.
 *
 * Returns 0 on success, negative errno on error.
 */
int screen_init(void);

/**
 * Fill the whole screen with a solid RGB565 color.
 */
void screen_fill(uint16_t color);

/**
 * Simple test: draw a vertical color gradient.
 */
void screen_draw_test_pattern(void);

#endif /* SCREEN_H_ */
