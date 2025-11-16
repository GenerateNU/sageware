// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

// Stepper Motor aliases (from devicetree aliases)
#define STEP_A   DT_ALIAS(step)
#define DIR_A    DT_ALIAS(dir)
#define ENABLE_A DT_ALIAS(enable)

// GPIO device tree specifications
static const struct gpio_dt_spec stepPin   = GPIO_DT_SPEC_GET(STEP_A, gpios);
static const struct gpio_dt_spec dirPin    = GPIO_DT_SPEC_GET(DIR_A, gpios);
static const struct gpio_dt_spec enablePin = GPIO_DT_SPEC_GET(ENABLE_A, gpios);

// Motor + gearbox constants
#define MOTOR_STEPS_PER_REV 200          // 1.8° stepper → 200 full steps/rev
#define GEAR_RATIO          20           // 20:1 planetary gearbox
#define STEPS_PER_REV       MOTOR_STEPS_PER_REV
#define TOTAL_STEPS         (STEPS_PER_REV * GEAR_RATIO)   // 200 * 20 = 4000

void rotateSteps(int steps, int delay_us)
{
    for (int i = 0; i < steps; i++) {
        gpio_pin_set_dt(&stepPin, 1);
        k_usleep(delay_us);

        gpio_pin_set_dt(&stepPin, 0);
        k_usleep(delay_us);
    }
}

int main(void)
{
    
    // Configure pins
    gpio_pin_configure_dt(&stepPin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&dirPin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&enablePin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    printk("Stepper Motor Ready\n");

    while (1) {
        // Blink LED so you know loop is running
        gpio_pin_toggle_dt(&led0);
        gpio_pin_set_dt(&dirPin, 1);
        rotateSteps(TOTAL_STEPS, 800);
        k_msleep(500);                      // 2 s pause between revolutions
    }
    
    /*
    printk("GPIO PC14 / PC15 Test Start\n");

    gpio_pin_configure_dt(&stepPin, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&dirPin,  GPIO_OUTPUT_INACTIVE);

    int state = 0;

    while (1) {
        state = !state;

        gpio_pin_set_dt(&stepPin, state);
        gpio_pin_set_dt(&dirPin,  state);

        printk("Toggled: STEP=%d   DIR=%d\n", state, state);

        k_msleep(2000);
    }
    */
}
