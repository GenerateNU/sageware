// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>

#include "motor.h"
#include "buttons.h"
// #include "heating_motor.h"   // Will have different gear ratio
// #include "molding_motor.h"   // Will have different gear ratio  
// #include "cutting_motor.h"   // Will have different gear ratio

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* Button event callback */
void button_event_handler(button_id_t button, button_event_t event)
{
    const char *button_names[] = {"UP", "DOWN", "LEFT", "RIGHT", "SELECT"};
    const char *event_names[] = {"NONE", "PRESSED", "RELEASED", "HELD"};
    
    printk("Button %s %s\n", button_names[button], event_names[event]);
}

int main(void)
{
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

    /* Initialize buttons */
    printk("Initializing buttons...\n");
    if (buttons_init() != 0) {
        printk("Button init failed!\n");
        while (1) { gpio_pin_toggle_dt(&led0); k_msleep(100); }
    }
    
    /* Register button callback */
    buttons_set_callback(button_event_handler);
    printk("Buttons ready - press any button!\n");

    /* Comment out motor tests for now */
    /*
    if (motor_init() != 0) {
        while (1) { gpio_pin_toggle_dt(&led0); k_msleep(100); }
    }

    printk("Stepper Motor Ready\n");

    // Example 1: Start motor clockwise
    motor_run(true, CLOCKWISE, 800);     // Turn ON, clockwise, 800us delay
    
    k_msleep(3000);                      // Run for 3 seconds
    
    // Example 2: Change speed while running (keep same direction)
    motor_run(true, CLOCKWISE, 500);     // Still ON, CW, but faster
    
    k_msleep(2000);                      // Run for 2 more seconds
    
    // Example 3: Change direction while running
    motor_run(true, COUNTER_CLOCKWISE, 500);  // Still ON, but now CCW
    
    k_msleep(2000);
    
    // Example 4: Stop the motor
    motor_run(false, CLOCKWISE, 0);      // Turn OFF (direction/delay don't matter)
    
    k_msleep(1000);                      // Pause
    
    // Example 5: Use blocking rotation for precise movement
    rotateSteps(TOTAL_STEPS, CLOCKWISE, 1000);  // Rotate exactly 1 full revolution
    
    k_msleep(500);
    
    // Example 6: Restart continuous operation
    motor_run(true, COUNTER_CLOCKWISE, 1000);
    
    k_msleep(3000);
    
    motor_run(false, CLOCKWISE, 0);      // Turn OFF
    */
    
    // Main loop - poll buttons and blink LED
    while (1) {
        /* Poll buttons every 10ms for responsive input */
        buttons_poll();
        
        gpio_pin_toggle_dt(&led0);
        k_msleep(10);  /* 10ms polling interval */
    }
}