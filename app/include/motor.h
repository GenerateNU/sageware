#ifndef MOTOR_H_
#define MOTOR_H_

#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/*
gpio_dt_spec is a struct that has attributes that can potentially include ports or pins
GPIO_DT_SPEC_GET is a macro that basically gets the value from the struct
*/

#ifdef __cplusplus
extern "C" {
#endif

/* Simple, blocking motor API (easy to call from main or tests) */
int  motor_init(void);                 // config pins, wake/enable driver
void motor_enable(bool en);            // enable/disable outputs
void motor_set_dir(bool cw);           // set direction
// void motor_rotate_rev(float revolutions, int delay_us_per_edge); // helper 

#ifdef __cplusplus
}
#endif

#endif