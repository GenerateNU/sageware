// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "motor_mold.h"

// Motor 2 - Molding (from devicetree aliases)
#define MOTOR2_PUL   DT_ALIAS(motor2pul)
#define MOTOR2_DIR   DT_ALIAS(motor2dir)

// GPIO device tree specifications
static const struct gpio_dt_spec stepPin = GPIO_DT_SPEC_GET(MOTOR2_PUL, gpios);
static const struct gpio_dt_spec dirPin  = GPIO_DT_SPEC_GET(MOTOR2_DIR, gpios);

// Thread control variables (file-local)
static volatile bool mold_motor_running   = false;
static volatile bool mold_motor_direction = CLOCKWISE;
static volatile int  mold_motor_delay_us  = 1000;

// Per-motor thread + stack (file-local)
static struct k_thread mold_motor_thread_data;
static k_tid_t         mold_motor_thread_id = NULL;

// Thread stack (1KB should be plenty for this simple task)
#define MOLD_MOTOR_STACK_SIZE 1024
static K_THREAD_STACK_DEFINE(mold_motor_stack, MOLD_MOTOR_STACK_SIZE);

int mold_motor_init(void)
{
    if (!device_is_ready(stepPin.port) ||
        !device_is_ready(dirPin.port)) {
        printk("Motor 2 (Molding) GPIO not ready\n");
        return -ENODEV;
    }

    int r = 0;
    r |= gpio_pin_configure_dt(&stepPin, GPIO_OUTPUT_INACTIVE);
    r |= gpio_pin_configure_dt(&dirPin,  GPIO_OUTPUT_INACTIVE);
    if (r) {
        printk("Motor 2 (Molding) GPIO config failed (%d)\n", r);
        return r;
    }

    printk("Motor 2 (Molding) initialized\n");
    return 0;
}

// Thread function that runs the motor continuously
static void mold_motor_thread_func(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("Mold motor thread started\n");

    // Set direction
    gpio_pin_set_dt(&dirPin, mold_motor_direction ? 1 : 0);

    while (mold_motor_running) {
        // Step pulse HIGH
        gpio_pin_set_dt(&stepPin, 1);
        k_usleep(mold_motor_delay_us);

        // Step pulse LOW
        gpio_pin_set_dt(&stepPin, 0);
        k_usleep(mold_motor_delay_us);
    }

    printk("Mold motor thread stopped\n");
}

// Single function to control motor: turn on/off, set direction and speed
void mold_motor_run(bool on, bool direction, int delay_us)
{
    if (on) {
        // Update speed and direction if motor is already running
        if (mold_motor_running) {
            mold_motor_delay_us  = delay_us;
            mold_motor_direction = direction;
            gpio_pin_set_dt(&dirPin, direction ? 1 : 0);
            printk("Mold motor speed/direction changed: delay_us=%d, dir=%s\n",
                   delay_us, direction ? "CW" : "CCW");
            return;
        }

        // Start the motor
        mold_motor_delay_us  = delay_us;
        mold_motor_direction = direction;
        mold_motor_running   = true;

        mold_motor_thread_id = k_thread_create(
            &mold_motor_thread_data,
            mold_motor_stack,
            K_THREAD_STACK_SIZEOF(mold_motor_stack),
            mold_motor_thread_func,
            NULL, NULL, NULL,
            K_PRIO_PREEMPT(7),
            0,
            K_NO_WAIT
        );

        if (mold_motor_thread_id == NULL) {
            mold_motor_running = false;
            printk("Failed to create mold motor thread\n");
            return;
        }

        k_thread_name_set(mold_motor_thread_id, "mold_motor_thread");
        printk("Mold motor started: delay_us=%d, dir=%s\n",
               delay_us, direction ? "CW" : "CCW");

    } else {
        // Stop the motor
        if (!mold_motor_running) {
            return;
        }

        printk("Stopping mold motor...\n");
        mold_motor_running = false;

        if (mold_motor_thread_id != NULL) {
            k_thread_join(mold_motor_thread_id, K_FOREVER);
            mold_motor_thread_id = NULL;
        }
    }
}

// Check if motor is currently running
bool mold_motor_is_running(void)
{
    return mold_motor_running;
}

// Blocking function to rotate a specific number of steps
void mold_rotateSteps(int steps, bool direction, int delay_us)
{
    // Set direction
    gpio_pin_set_dt(&dirPin, direction ? 1 : 0);

    // Step through
    for (int i = 0; i < steps; i++) {
        gpio_pin_set_dt(&stepPin, 1);
        k_usleep(delay_us);
        gpio_pin_set_dt(&stepPin, 0);
        k_usleep(delay_us);
    }
}
