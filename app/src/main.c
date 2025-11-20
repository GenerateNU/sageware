// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>

#include "motor.h"

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Motor + gearbox constants
#define MOTOR_STEPS_PER_REV 200          // 1.8° stepper → 200 full steps/rev
#define GEAR_RATIO          20           // 20:1 planetary gearbox
#define STEPS_PER_REV       MOTOR_STEPS_PER_REV
#define TOTAL_STEPS         (STEPS_PER_REV * GEAR_RATIO)   // 200 * 20 = 4000

int main(void)
{
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    if (motor_init() != 0) {
        while (1) { gpio_pin_toggle_dt(&led0); k_msleep(100); }
    }

    printk("Stepper Motor Ready\n");

    while (1) {
        gpio_pin_toggle_dt(&led0);
        motor_set_dir(true);
        rotateSteps(TOTAL_STEPS, 800);
        k_msleep(500);
    }
}