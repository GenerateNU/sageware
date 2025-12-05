#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include "motor_cutting.h"
#include "motor_laminate.h"
#include "motor_mold.h"
#include "motor_despensing.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

/* Hardware Definitions */
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec heater = GPIO_DT_SPEC_GET(DT_ALIAS(heater), gpios);
static const struct gpio_dt_spec start_btn = GPIO_DT_SPEC_GET(DT_ALIAS(start_button), gpios);
static const struct gpio_dt_spec beam_breaker = GPIO_DT_SPEC_GET(DT_ALIAS(beam_breaker), gpios);

/* Temperature Constants */
#define TEMP_DESIRED      180.0f
#define TEMP_HYSTERESIS   5.0f
#define TEMP_MAX_SAFE     220.0f
#define TEMP_SAFE_RESUME  100.0f

/* State Machine */
typedef enum {
    STATE_HEATING,
    STATE_READY,
    STATE_ROLLING,
    STATE_PAUSE,
    STATE_CUTTING,
    STATE_OVERHEAT
} system_state_t;

static system_state_t current_state = STATE_HEATING;

/* Helper Functions */
float read_temperature(void) {
    // TODO: Implement actual ADC reading
    // For now, return a simulated temperature that rises when heater is on
    static float sim_temp = 25.0f;
    if (gpio_pin_get_dt(&heater)) {
        sim_temp += 0.5f;
    } else {
        if (sim_temp > 25.0f) sim_temp -= 0.1f;
    }
    return sim_temp;
}

void set_heater(bool on) {
    gpio_pin_set_dt(&heater, on ? 1 : 0);
}

void stop_all_motors(void) {
    laminate_motor_run(false, CLOCKWISE, 0);
    mold_motor_run(false, CLOCKWISE, 0);
    despensing_motor_run(false, CLOCKWISE, 0);
    cutting_motor_run(false, CLOCKWISE, 0);
}

int main(void)
{
    /* Initialize GPIOs */
    if (device_is_ready(led0.port)) gpio_pin_configure_dt(&led0, GPIO_OUTPUT_INACTIVE);
    if (device_is_ready(heater.port)) gpio_pin_configure_dt(&heater, GPIO_OUTPUT_INACTIVE);
    if (device_is_ready(start_btn.port)) gpio_pin_configure_dt(&start_btn, GPIO_INPUT);
    if (device_is_ready(beam_breaker.port)) gpio_pin_configure_dt(&beam_breaker, GPIO_INPUT);

    /* Initialize Motors */
    laminate_motor_init();
    mold_motor_init();
    despensing_motor_init();
    cutting_motor_init();

    LOG_INF("System Initialized. Starting State Machine.");

    while (1) {
        float temp = read_temperature();
        // LOG_INF("State: %d, Temp: %d C", current_state, (int)temp);

        switch (current_state) {
            case STATE_HEATING:
                set_heater(true);
                if (temp >= TEMP_MAX_SAFE) {
                    current_state = STATE_OVERHEAT;
                    LOG_INF("Transition to OVERHEAT");
                } else if (temp >= TEMP_DESIRED) {
                    current_state = STATE_READY;
                    LOG_INF("Transition to READY");
                }
                break;

            case STATE_READY:
                // Hysteresis: Maintain Temp
                if (temp < (TEMP_DESIRED - TEMP_HYSTERESIS)) {
                    set_heater(true);
                } else if (temp > TEMP_DESIRED) {
                    set_heater(false);
                }

                if (temp >= TEMP_MAX_SAFE) {
                    current_state = STATE_OVERHEAT;
                    LOG_INF("Transition to OVERHEAT");
                } else if (gpio_pin_get_dt(&start_btn)) {
                    current_state = STATE_ROLLING;
                    LOG_INF("Transition to ROLLING");
                }
                break;

            case STATE_ROLLING:
                // Laminate ON, Mold ON, Dispense ON
                laminate_motor_run(true, CLOCKWISE, 1000);
                mold_motor_run(true, CLOCKWISE, 1000);
                despensing_motor_run(true, CLOCKWISE, 1000);

                if (temp >= TEMP_MAX_SAFE) {
                    current_state = STATE_OVERHEAT;
                    LOG_INF("Transition to OVERHEAT");
                    stop_all_motors();
                } 
                else if (gpio_pin_get_dt(&beam_breaker)) {
                    LOG_INF("Beam Breaker Triggered! Transition to PAUSE");
                    current_state = STATE_PAUSE;
                }
                break;

            case STATE_PAUSE:
                stop_all_motors();
                k_msleep(500); // Short delay to let material settle
                current_state = STATE_CUTTING;
                LOG_INF("Transition to CUTTING");
                break;

            case STATE_CUTTING:
                // Cutter Cycle: Down then Up
                LOG_INF("Cutter DOWN");
                cutting_motor_run(true, CLOCKWISE, 800);
                k_msleep(1000);
                cutting_motor_run(false, CLOCKWISE, 0);
                
                LOG_INF("Cutter UP");
                cutting_motor_run(true, COUNTER_CLOCKWISE, 800);
                k_msleep(1000);
                cutting_motor_run(false, COUNTER_CLOCKWISE, 0);

                current_state = STATE_ROLLING;
                LOG_INF("Transition to ROLLING");
                break;

            case STATE_OVERHEAT:
                set_heater(false);
                stop_all_motors();
                if (temp < TEMP_SAFE_RESUME) {
                    current_state = STATE_HEATING;
                    LOG_INF("Transition to HEATING");
                }
                break;
        }

        k_msleep(100); // Loop delay
    }
}
