#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>

// State enum
typedef enum {
    ST_POWER_ON = 0;                    // default power off
    ST_HEATER_ON;                       // heater turns on when power is on 
    ST_WAIT_START_BUTTON;               // heating is ready, waiting until start button pressed
    ST_START_LAMINATE_MOLD_DISPENSE;    // start button pressed, begin laminate/mold/dispense
    ST_BEAM_BREAKER;                    // start beam breaker (light sensor)
    ST_STOP_LAMINATE_MOLD_DISPENSE;     // deactivate laminate/mold/dispense
    ST_CUTTER;                          // cutter motor activated for static down then up
    ST_MAX_STATES;                      // this is a COUNT, NOT a state
} MyState;

// State table
static const state_handlers state_table[ST_MAX_STATES] = {
    ST_POWER_ON =                       {st_power_on_run, NULL},
    ST_HEATER_ON =                      {st_heater_on_run, st_heater_on_exit},
    ST_WAIT_START_BUTTON=               {st_wait_start_button_run, st_wait_start_button_exit},
    ST_START_LAMINATE_MOLD_DISPENSE =   {st_start_laminate_mold_dispense_run, st_start_laminate_mold_dispense_exit},
    ST_BEAM_BREAKER =                   {st_beam_breaker_run, st_beam_breaker_exit},
    ST_STOP_LAMINATE_MOLD_DISPENSE =    {NULL, NULL},
    ST_CUTTER =                         {st_cutter_run, st_cutter_exit},
}

static MyState current_state;

void change_state(MyState new_state) {
    if (new_state >= ST_MAX_STATES) return; // exit when reaching end of state machine

    // Exit current state
    if (state_table[current_state].exit)
        state_table[current_state].exit();

    next_state = new_state;

    // Enter new state
    if (state_table[next_state].entry)
        state_table[next_state].entry();

    current_state = next_state;
}

/* State transition variables
- power on (?)
- heater hot enough
- start button pressed
- [activate on timer signal, nothing needed]
- beam breaker signal
- [deactivate also on timer signal]
- [cutter also on timer signal]
*/

// ST_POWER_ON

void st_power_on_run(void) {
    // immediately go to next state, no exit function
    current_state = ST_HEATER_ON;
}

//ST_HEATER_ON

void st_heater_on_run, st_heater_on_exit


void state_machine_thread(void) {
    while (1) {

    }
}
