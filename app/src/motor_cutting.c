// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "motor_cutting.h"

// Motor 3 - Cutting (from devicetree aliases)
#define MOTOR3_PUL   DT_ALIAS(motor3pul)
#define MOTOR3_DIR   DT_ALIAS(motor3dir)

// GPIO device tree specifications
static const struct gpio_dt_spec stepPin   = GPIO_DT_SPEC_GET(MOTOR3_PUL, gpios);
static const struct gpio_dt_spec dirPin    = GPIO_DT_SPEC_GET(MOTOR3_DIR, gpios);

// Thread control variables (make them file-local)
static volatile bool cutting_motor_running   = false;
static volatile bool cutting_motor_direction = CLOCKWISE;
static volatile int  cutting_motor_delay_us  = 1000;

// Per-motor thread + stack (also file-local)
static struct k_thread cutting_motor_thread_data;
static k_tid_t         cutting_motor_thread_id = NULL;

// Thread stack (1KB should be plenty for this simple task)
#define CUTTING_MOTOR_STACK_SIZE 1024
static K_THREAD_STACK_DEFINE(cutting_motor_stack, CUTTING_MOTOR_STACK_SIZE);

int cutting_motor_init(void)
{
    if (!device_is_ready(stepPin.port) ||
        !device_is_ready(dirPin.port)) {
        printk("Motor 3 (Cutting) GPIO not ready\n");
        return -ENODEV;
    }

    int r = 0;
    r |= gpio_pin_configure_dt(&stepPin, GPIO_OUTPUT_INACTIVE);
    r |= gpio_pin_configure_dt(&dirPin,  GPIO_OUTPUT_INACTIVE);
    if (r) {
        printk("Motor 3 (Cutting) GPIO config failed (%d)\n", r);
        return r;
    }

    printk("Motor 3 (Cutting) initialized\n");
    return 0;
}

// Thread function that runs the motor continuously
static void cutting_motor_thread_func(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("Cutting motor thread started\n");

    // Set direction
    gpio_pin_set_dt(&dirPin, cutting_motor_direction ? 1 : 0);

    while (cutting_motor_running) {
        // Step pulse HIGH
        gpio_pin_set_dt(&stepPin, 1);
        k_usleep(cutting_motor_delay_us);

        // Step pulse LOW
        gpio_pin_set_dt(&stepPin, 0);
        k_usleep(cutting_motor_delay_us);
    }

    printk("Cutting motor thread stopped\n");
}

// Single function to control motor: turn on/off, set direction and speed
void cutting_motor_run(bool on, bool direction, int delay_us)
{
    if (on) {
        // Update speed and direction if motor is already running
        if (cutting_motor_running) {
            cutting_motor_delay_us  = delay_us;
            cutting_motor_direction = direction;
            gpio_pin_set_dt(&dirPin, direction ? 1 : 0);
            printk("Cutting motor speed/direction changed: delay_us=%d, dir=%s\n",
                   delay_us, direction ? "CW" : "CCW");
            return;
        }

        // Start the motor
        cutting_motor_delay_us  = delay_us;
        cutting_motor_direction = direction;
        cutting_motor_running   = true;

        cutting_motor_thread_id = k_thread_create(
            &cutting_motor_thread_data,
            cutting_motor_stack,
            K_THREAD_STACK_SIZEOF(cutting_motor_stack),
            cutting_motor_thread_func,
            NULL, NULL, NULL,
            K_PRIO_PREEMPT(7),
            0,
            K_NO_WAIT
        );

        if (cutting_motor_thread_id == NULL) {
            cutting_motor_running = false;
            printk("Failed to create cutting motor thread\n");
            return;
        }

        k_thread_name_set(cutting_motor_thread_id, "cutting_motor_thread");
        printk("Cutting motor started: delay_us=%d, dir=%s\n",
               delay_us, direction ? "CW" : "CCW");

    } else {
        // Stop the motor
        if (!cutting_motor_running) {
            return;
        }

        printk("Stopping cutting motor...\n");
        cutting_motor_running = false;

        if (cutting_motor_thread_id != NULL) {
            k_thread_join(cutting_motor_thread_id, K_FOREVER);
            cutting_motor_thread_id = NULL;
        }
    }
}

// Check if motor is currently running
bool cutting_motor_is_running(void)
{
    return cutting_motor_running;
}

// Blocking function to rotate a specific number of steps
void cutting_rotateSteps(int steps, bool direction, int delay_us)
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
