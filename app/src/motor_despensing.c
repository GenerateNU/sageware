// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "motor_despensing.h"

// Motor 4 - Dispensing (from devicetree aliases)
#define MOTOR4_PUL   DT_ALIAS(motor4pul)
#define MOTOR4_DIR   DT_ALIAS(motor4dir)

// GPIO device tree specifications
static const struct gpio_dt_spec stepPin = GPIO_DT_SPEC_GET(MOTOR4_PUL, gpios);
static const struct gpio_dt_spec dirPin  = GPIO_DT_SPEC_GET(MOTOR4_DIR, gpios);

// Thread control variables (file-local)
static volatile bool despensing_motor_running   = false;
static volatile bool despensing_motor_direction = CLOCKWISE;
static volatile int  despensing_motor_delay_us  = 1000;

// Per-motor thread + stack (file-local)
static struct k_thread despensing_motor_thread_data;
static k_tid_t         despensing_motor_thread_id = NULL;

// Thread stack (1KB should be plenty for this simple task)
#define DESPENSING_MOTOR_STACK_SIZE 1024
static K_THREAD_STACK_DEFINE(despensing_motor_stack, DESPENSING_MOTOR_STACK_SIZE);

int despensing_motor_init(void)
{
    if (!device_is_ready(stepPin.port) ||
        !device_is_ready(dirPin.port)) {
        printk("Motor 4 (Dispensing) GPIO not ready\n");
        return -ENODEV;
    }

    int r = 0;
    r |= gpio_pin_configure_dt(&stepPin, GPIO_OUTPUT_INACTIVE);
    r |= gpio_pin_configure_dt(&dirPin,  GPIO_OUTPUT_INACTIVE);
    if (r) {
        printk("Motor 4 (Dispensing) GPIO config failed (%d)\n", r);
        return r;
    }

    printk("Motor 4 (Dispensing) initialized\n");
    return 0;
}

// Thread function that runs the motor continuously
static void despensing_motor_thread_func(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("Despensing motor thread started\n");

    // Set direction
    gpio_pin_set_dt(&dirPin, despensing_motor_direction ? 1 : 0);

    while (despensing_motor_running) {
        // Step pulse HIGH
        gpio_pin_set_dt(&stepPin, 1);
        k_usleep(despensing_motor_delay_us);

        // Step pulse LOW
        gpio_pin_set_dt(&stepPin, 0);
        k_usleep(despensing_motor_delay_us);
    }

    printk("Despensing motor thread stopped\n");
}

// Single function to control motor: turn on/off, set direction and speed
void despensing_motor_run(bool on, bool direction, int delay_us)
{
    if (on) {
        // Update speed and direction if motor is already running
        if (despensing_motor_running) {
            despensing_motor_delay_us  = delay_us;
            despensing_motor_direction = direction;
            gpio_pin_set_dt(&dirPin, direction ? 1 : 0);
            printk("Despensing motor speed/direction changed: delay_us=%d, dir=%s\n",
                   delay_us, direction ? "CW" : "CCW");
            return;
        }

        // Start the motor
        despensing_motor_delay_us  = delay_us;
        despensing_motor_direction = direction;
        despensing_motor_running   = true;

        despensing_motor_thread_id = k_thread_create(
            &despensing_motor_thread_data,
            despensing_motor_stack,
            K_THREAD_STACK_SIZEOF(despensing_motor_stack),
            despensing_motor_thread_func,
            NULL, NULL, NULL,
            K_PRIO_PREEMPT(7),
            0,
            K_NO_WAIT
        );

        if (despensing_motor_thread_id == NULL) {
            despensing_motor_running = false;
            printk("Failed to create despensing motor thread\n");
            return;
        }

        k_thread_name_set(despensing_motor_thread_id, "despensing_motor_thread");
        printk("Despensing motor started: delay_us=%d, dir=%s\n",
               delay_us, direction ? "CW" : "CCW");

    } else {
        // Stop the motor
        if (!despensing_motor_running) {
            return;
        }

        printk("Stopping despensing motor...\n");
        despensing_motor_running = false;

        if (despensing_motor_thread_id != NULL) {
            k_thread_join(despensing_motor_thread_id, K_FOREVER);
            despensing_motor_thread_id = NULL;
        }
    }
}

// Check if motor is currently running
bool despensing_motor_is_running(void)
{
    return despensing_motor_running;
}

// Blocking function to rotate a specific number of steps
void despensing_rotateSteps(int steps, bool direction, int delay_us)
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
