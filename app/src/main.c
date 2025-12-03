// #include <stdio.h>
#include <zephyr/kernel.h>
// zephyr/device.h is included eventually through kernel.h
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);   // register your module


#include "motor.h"
#include "screen.h"
#include "adc.h"
// #include "heating_motor.h"   // Will have different gear ratio
// #include "molding_motor.h"   // Will have different gear ratio  
// #include "cutting_motor.h"   // Will have different gear ratio

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
 
     CurrentSense_Init();
    while (1) {
        float current = CurrentSense_ReadCurrent();
        bool overcurrent = CurrentSense_IsOvercurrent();

        int current_mA = (int)(current * 1000.0f);

LOG_INF("Current: %d mA, Overcurrent: %s",
        current_mA,
        overcurrent ? "YES" : "NO");

        k_msleep(500);
    
    }
    gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);

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
    
    // Main loop for your bead project
    while (1) {
        gpio_pin_toggle_dt(&led0);
        k_msleep(1000);
        
        // Your bead-making workflow:
        // 1. Wait for user to select bead count on LCD
        // 2. Wait for heaters to warm up
        // 3. Wait for start button press
        // 4. Start heating motor: motor_run(true, 800);
        // 5. Start molding motor: molding_motor_run(true, 800);
        // 6. Wait for photoelectric sensor
        // 7. After 3 seconds, stop motors for cutting
        // 8. Do cutting with current sensing
        // 9. Restart motors and repeat until bead count reached

    }
}