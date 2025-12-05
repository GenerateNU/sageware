#ifndef MOTOR_H_
#define MOTOR_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

// Motor + gearbox constants for this specific motor
#define MOTOR_STEPS_PER_REV 200          // 1.8° stepper → 200 full steps/rev
#define GEAR_RATIO          20           // 20:1 planetary gearbox
#define STEPS_PER_REV       MOTOR_STEPS_PER_REV
#define TOTAL_STEPS         (STEPS_PER_REV * GEAR_RATIO)   // 200 * 20 = 4000

// Direction definitions
#define CLOCKWISE           true
#define COUNTER_CLOCKWISE   false

/* Simple motor API */
int  cutting_motor_init(void);                                    // config pins, wake/enable driver
void cutting_motor_enable(bool en);                               // enable/disable outputs
void cutting_motor_run(bool on, bool direction, int delay_us);    // Turn motor on/off, set direction and speed
void cutting_rotateSteps(int steps, bool direction, int delay_us);// Blocking rotate for specific steps
bool cutting_motor_is_running(void);                              // Check if motor is currently running

#endif