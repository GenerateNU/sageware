// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "motor_cutting.h"

// Motor 3 - Cutting (from devicetree aliases)
#define MOTOR3_PUL   DT_ALIAS(motor3_pul)
#define MOTOR3_DIR   DT_ALIAS(motor3_dir)

// GPIO device tree specifications
static const struct gpio_dt_spec stepPin   = GPIO_DT_SPEC_GET(MOTOR3_PUL, gpios);
static const struct gpio_dt_spec dirPin    = GPIO_DT_SPEC_GET(MOTOR3_DIR, gpios);

// Thread control variables
static volatile bool motor_running = false;
static volatile bool motor_direction = CLOCKWISE;
static volatile int motor_delay_us = 1000;
static struct k_thread motor_thread_data;
static k_tid_t motor_thread_id = NULL;

// Thread stack (1KB should be plenty for this simple task)
#define MOTOR_STACK_SIZE 1024
K_THREAD_STACK_DEFINE(motor_stack, MOTOR_STACK_SIZE);

int motor_init(void)
{
    if (!device_is_ready(stepPin.port) ||
        !device_is_ready(dirPin.port)) {
        printk("Motor 3 (Cutting) GPIO not ready\n");
        return -ENODEV;
    }

    int r = 0;
    r |= gpio_pin_configure_dt(&stepPin, GPIO_OUTPUT_INACTIVE);
    r |= gpio_pin_configure_dt(&dirPin,  GPIO_OUTPUT_INACTIVE);
    if (r) return r;

    printk("Motor 3 (Cutting) initialized\n");
    return 0;
}

// Thread function that runs the motor continuously
static void motor_thread_func(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("Motor thread started\n");

    // Set direction
    gpio_pin_set_dt(&dirPin, motor_direction ? 1 : 0);

    while (motor_running) {
        // Step pulse HIGH
        gpio_pin_set_dt(&stepPin, 1);
        k_usleep(motor_delay_us);
        
        // Step pulse LOW
        gpio_pin_set_dt(&stepPin, 0);
        k_usleep(motor_delay_us);
    }

    printk("Motor thread stopped\n");
}

// Single function to control motor: turn on/off, set direction and speed
void motor_run(bool on, bool direction, int delay_us)
{
    if (on) {
        // Update speed and direction if motor is already running
        if (motor_running) {
            motor_delay_us = delay_us;
            motor_direction = direction;
            gpio_pin_set_dt(&dirPin, direction ? 1 : 0);
            printk("Motor speed/direction changed: delay_us=%d, dir=%s\n", 
                   delay_us, direction ? "CW" : "CCW");
            return;
        }

        // Start the motor
        motor_delay_us = delay_us;
        motor_direction = direction;
        motor_running = true;

        motor_thread_id = k_thread_create(
            &motor_thread_data,
            motor_stack,
            MOTOR_STACK_SIZE,
            motor_thread_func,
            NULL, NULL, NULL,
            K_PRIO_PREEMPT(7),
            0,
            K_NO_WAIT
        );

        if (motor_thread_id == NULL) {
            motor_running = false;
            printk("Failed to create motor thread\n");
            return;
        }

        k_thread_name_set(motor_thread_id, "motor_thread");
        printk("Motor started: delay_us=%d, dir=%s\n", 
               delay_us, direction ? "CW" : "CCW");

    } else {
        // Stop the motor
        if (!motor_running) {
            return;
        }

        printk("Stopping motor...\n");
        motor_running = false;

        if (motor_thread_id != NULL) {
            k_thread_join(motor_thread_id, K_FOREVER);
            motor_thread_id = NULL;
        }
    }
}

// Check if motor is currently running
bool motor_is_running(void)
{
    return motor_running;
}

// Blocking function to rotate a specific number of steps
void rotateSteps(int steps, bool direction, int delay_us)
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