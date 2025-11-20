// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "motor.h"

// Stepper Motor aliases (from devicetree aliases)
#define STEP_A   DT_ALIAS(step)
#define DIR_A    DT_ALIAS(dir)
#define ENABLE_A DT_ALIAS(enable)   

// GPIO device tree specifications
static const struct gpio_dt_spec stepPin   = GPIO_DT_SPEC_GET(STEP_A, gpios);
static const struct gpio_dt_spec dirPin    = GPIO_DT_SPEC_GET(DIR_A,  gpios);
static const struct gpio_dt_spec enablePin = GPIO_DT_SPEC_GET(ENABLE_A, gpios);

// *** CHANGE THIS BASED ON YOUR BOARD ***
// If your motor driver enable pin is nEN (active LOW): set to 1
// If your motor driver enable pin is EN  (active HIGH): set to 0
#define NEN_ACTIVE_LOW 1

// Helper: control enable pin polarity
static inline void drv_enable(bool en)
{
#if NEN_ACTIVE_LOW
    gpio_pin_set_dt(&enablePin, en ? 0 : 1);   // nEN low = enable
#else
    gpio_pin_set_dt(&enablePin, en ? 1 : 0);   // EN  high = enable
#endif
}

int motor_init(void)
{
    if (!device_is_ready(stepPin.port) ||
        !device_is_ready(dirPin.port)  ||
        !device_is_ready(enablePin.port)) {
        printk("Motor GPIO not ready\n");
        return -ENODEV;
    }

    int r = 0;
    r |= gpio_pin_configure_dt(&stepPin, GPIO_OUTPUT_INACTIVE);
    r |= gpio_pin_configure_dt(&dirPin,  GPIO_OUTPUT_INACTIVE);
    r |= gpio_pin_configure_dt(&enablePin, GPIO_OUTPUT_INACTIVE);
    if (r) return r;

    drv_enable(true); // enable outputs
    return 0;
}

// Same rotateSteps you originally wrote
void rotateSteps(int steps, int delay_us)
{
    for (int i = 0; i < steps; i++) {
        gpio_pin_set_dt(&stepPin, 1);
        k_usleep(delay_us);
        gpio_pin_set_dt(&stepPin, 0);
        k_usleep(delay_us);
    }
}

// Simple wrappers so main.c stays clean
void motor_set_dir(bool dir_high) { gpio_pin_set_dt(&dirPin, dir_high ? 1 : 0); }
void motor_enable(bool on) { drv_enable(on); }
