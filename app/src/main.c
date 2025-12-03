#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>   // ✅ Needed for LOG_MODULE_REGISTER / LOG_INF
#include <zephyr/drivers/gpio.h>

#include "motor.h"
#include "screen.h"

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

void main(void)
{
    int ret;

    // Optional: init LED so you can blink if something goes wrong later
    if (!device_is_ready(led0.port)) {
        printk("LED0 device not ready\n");
    } else {
        gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    }

    LOG_INF("Firmware start");

    ret = screen_init();
    LOG_INF("screen_init() returned %d", ret);

    if (ret != 0) {
        // If init failed, blink LED slowly so you see it on the board
        while (1) {
            if (device_is_ready(led0.port)) {
                gpio_pin_toggle_dt(&led0);
            }
            k_msleep(500);
        }
    }

    // Simple color test loop: red → green → blue → gradient (repeat)
    while (1) {
        LOG_INF("Fill RED");
        screen_fill(0xF800);      // Red
        k_msleep(1000);

        LOG_INF("Fill GREEN");
        screen_fill(0x07E0);      // Green
        k_msleep(1000);

        LOG_INF("Fill BLUE");
        screen_fill(0x001F);      // Blue
        k_msleep(1000);

        LOG_INF("Draw gradient");
        screen_draw_test_pattern();
        k_msleep(3000);
    }
}
