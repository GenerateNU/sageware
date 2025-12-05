#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>

#include "motor_cutting.h"

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* PD11 control pin from devicetree */
#define SOME_CTRL_NODE DT_NODELABEL(some_control_pin)
static const struct gpio_dt_spec some_ctrl =
    GPIO_DT_SPEC_GET(SOME_CTRL_NODE, gpios);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

void main(void)
{
    int ret;

    /* LED init */
    if (device_is_ready(led0.port)) {
        gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    }

    printk("=== Firmware start ===\n");
    LOG_INF("Firmware start");

    /* Configure PD11 */
    printk("Configuring PD11...\n");

    if (!device_is_ready(some_ctrl.port)) {
        printk("ERROR: PD11 port NOT ready!\n");
        LOG_ERR("some_ctrl port not ready");
    } else {
        ret = gpio_pin_configure_dt(&some_ctrl, GPIO_OUTPUT_INACTIVE);
        printk("gpio_pin_configure_dt returned %d\n", ret);

        if (ret == 0) {
            gpio_pin_set_dt(&some_ctrl, 1);  // HIGH
            printk("PD11 set HIGH!\n");
            LOG_INF("PD11 HIGH");
        } else {
            printk("Failed to configure PD11: %d\n", ret);
            LOG_ERR("Failed to configure some_ctrl: %d", ret);
        }
    }

    /* DEBUG: toggle PD11 to see a waveform on the oscilloscope */
    printk("Starting PD11 toggle test...\n");
    LOG_INF("Starting PD11 toggle test");

    for (int i = 0; i < 20; i++) {
        gpio_pin_toggle_dt(&some_ctrl);
        printk("Toggling PD11: iteration %d\n", i);
        k_msleep(100);  // 10Hz square wave
    }

    printk("PD11 toggle test DONE\n");
    LOG_INF("PD11 toggle test DONE");

    /* Cutting motor init */
    ret = cutting_motor_init();
    LOG_INF("cutting_motor_init() returned %d", ret);

    if (ret != 0) {
        printk("cutting_motor_init FAILED!\n");
        while (1) {
            gpio_pin_toggle_dt(&led0);
            k_msleep(500);
        }
    }

    LOG_INF("Starting cutting motor clockwise...");
    printk("Starting cutting motor...\n");

    cutting_motor_run(true, CLOCKWISE, 800);
    k_msleep(3000);
    cutting_motor_run(false, CLOCKWISE, 0);

    LOG_INF("Cutting motor stopped");
    printk("Cutting motor stopped\n");

    /* Idle loop */
    while (1) {
        k_msleep(1000);
    }
}
