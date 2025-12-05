// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "motor_laminate.h"

// Motor 1 - Laminating (from devicetree aliases)
#define MOTOR1_PUL   DT_ALIAS(motor1pul)
#define MOTOR1_DIR   DT_ALIAS(motor1dir)

// GPIO device tree specifications
static const struct gpio_dt_spec stepPin = GPIO_DT_SPEC_GET(MOTOR1_PUL, gpios);
static const struct gpio_dt_spec dirPin  = GPIO_DT_SPEC_GET(MOTOR1_DIR, gpios);

// Thread control variables (file-local)
static volatile bool laminate_motor_running   = false;
static volatile bool laminate_motor_direction = CLOCKWISE;
static volatile int  laminate_motor_delay_us  = 1000;

// Per-motor thread + stack (file-local)
static struct k_thread laminate_motor_thread_data;
static k_tid_t         laminate_motor_thread_id = NULL;

// Thread stack (1KB should be plenty for this simple task)
#define LAMINATE_MOTOR_STACK_SIZE 1024
static K_THREAD_STACK_DEFINE(laminate_motor_stack, LAMINATE_MOTOR_STACK_SIZE);

int laminate_motor_init(void)
{
    if (!device_is_ready(stepPin.port) ||
        !device_is_ready(dirPin.port)) {
        printk("Motor 1 (Laminating) GPIO not ready\n");
        return -ENODEV;
    }

    int r = 0;
    r |= gpio_pin_configure_dt(&stepPin, GPIO_OUTPUT_INACTIVE);
    r |= gpio_pin_configure_dt(&dirPin,  GPIO_OUTPUT_INACTIVE);
    if (r) {
        printk("Motor 1 (Laminating) GPIO config failed (%d)\n", r);
        return r;
    }

    printk("Motor 1 (Laminating) initialized\n");
    return 0;
}

// Thread function that runs the motor continuously
static void laminate_motor_thread_func(void *p1, void *p2, void *p3)
{
    ARG_UNUSED(p1);
    ARG_UNUSED(p2);
    ARG_UNUSED(p3);

    printk("Laminate motor thread started\n");

    // Set direction
    gpio_pin_set_dt(&dirPin, laminate_motor_direction ? 1 : 0);

    while (laminate_motor_running) {
        // Step pulse HIGH
        gpio_pin_set_dt(&stepPin, 1);
        k_usleep(laminate_motor_delay_us);

        // Step pulse LOW
        gpio_pin_set_dt(&stepPin, 0);
        k_usleep(laminate_motor_delay_us);
    }

    printk("Laminate motor thread stopped\n");
}

// Single function to control motor: turn on/off, set direction and speed
void laminate_motor_run(bool on, bool direction, int delay_us)
{
    if (on) {
        // Update speed and direction if motor is already running
        if (laminate_motor_running) {
            laminate_motor_delay_us  = delay_us;
            laminate_motor_direction = direction;
            gpio_pin_set_dt(&dirPin, direction ? 1 : 0);
            printk("Laminate motor speed/direction changed: delay_us=%d, dir=%s\n",
                   delay_us, direction ? "CW" : "CCW");
            return;
        }

        // Start the motor
        laminate_motor_delay_us  = delay_us;
        laminate_motor_direction = direction;
        laminate_motor_running   = true;

        laminate_motor_thread_id = k_thread_create(
            &laminate_motor_thread_data,
            laminate_motor_stack,
            K_THREAD_STACK_SIZEOF(laminate_motor_stack),
            laminate_motor_thread_func,
            NULL, NULL, NULL,
            K_PRIO_PREEMPT(7),
            0,
            K_NO_WAIT
        );

        if (laminate_motor_thread_id == NULL) {
            laminate_motor_running = false;
            printk("Failed to create laminate motor thread\n");
            return;
        }

        k_thread_name_set(laminate_motor_thread_id, "laminate_motor_thread");
        printk("Laminate motor started: delay_us=%d, dir=%s\n",
               delay_us, direction ? "CW" : "CCW");

    } else {
        // Stop the motor
        if (!laminate_motor_running) {
            return;
        }

        printk("Stopping laminate motor...\n");
        laminate_motor_running = false;

        if (laminate_motor_thread_id != NULL) {
            k_thread_join(laminate_motor_thread_id, K_FOREVER);
            laminate_motor_thread_id = NULL;
        }
    }
}

// Check if motor is currently running
bool laminate_motor_is_running(void)
{
    return laminate_motor_running;
}

// Blocking function to rotate a specific number of steps
void laminate_rotateSteps(int steps, bool direction, int delay_us)
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
