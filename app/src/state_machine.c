#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>

#include "state_machine.h"
#include "motor_despensing.h"
#include "motor_laminate.h"
#include "motor_mold.h"
#include "motor_cutting.h"

// State enum
typedef enum {
    ST_HEATER_ON;                       // heater turns on when power is on 
    ST_WAIT_START_BUTTON;               // heating is ready, waiting until start button pressed
    ST_START_LAMINATE_MOLD_DISPENSE;    // start button pressed, begin laminate/mold/dispense, exit when beam breaks
    ST_STOP_LAMINATE_MOLD_DISPENSE;              // bream has broken, stop laminate/mold/dispense after a set amount of time
    ST_CUTTER;                          // cutter motor activated for static down then up
    ST_MAX_STATES;                      // this is a COUNT, NOT a state
} MyState;

// State table
static const state_handlers state_table[ST_MAX_STATES] = {
    ST_HEATER_ON =                      {st_heater_on_run},
    ST_WAIT_START_BUTTON =              {st_wait_start_button_run},
    ST_START_LAMINATE_MOLD_DISPENSE =   {st_start_laminate_mold_dispense_run},
    ST_STOP_LAMINATE_MOLD_DISPENSE; =   {st_stop_laminate_mold_dispense_run},
    ST_CUTTER =                         {st_cutter_run},
}

static MyState current_state;

/* NOT sure if we need this bc its not actually called in the state machine
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
}*/

// Overheating check
// use correct value for max temp
// write heater pin low to turn off heater
void st_overheat(int temp) {
    if (temp > max) {
        // shut off power to heating
    }
    else return false;
}

// ST_HEATER_ON
// use correct logic for measuring heater temp
// use correct values for min/max of heater temp
// write heater enable pin high to turn on heater
void st_heater_on_run(int temp) {
    // heater runs until temperature is in range
    while(!st_overheat()) {
        // turn on heater
        if (min < temp < max) {
            current_state = ST_WAIT_START_BUTTON;
        }
    }
} 

// ST_WAIT_START_BUTTON
// use correct logic for start button
void st_wait_start_run(bool start_button) {
    // wait until user presses start button
    // we should be checking temperature to make sure it's in range while waiting
    // but for simplicity let's assume our user presses the start button immediately
    while(!st_overheat()) {
        if (start_button) {
            current_state = ST_START_LAMINATE_MOLD_DISPENSE;
        }
    }
}

// ST_LAMINATE_MOLD_DISPENSE
// update direction/delay of motors
void st_start_laminate_mold_dispense_run(int beam_breaker) {
    // turn on all three motors simultaneously and run
    while(!st_overheat()) {
        laminate_motor_run(true, direction, delay_us);
        mold_motor_run(true, direction, delay_us);
        despensing_motor_run(true, direction, delay_us);
        if (beam_breaker) {
            current_state = ST_STOP_LAMINATE_MOLD_DISPENSE;
        }
    }
}

// ST_STOP_LAMINATE_MOLD_DISPENSE
// update direction/delay of motors
void st_stop_laminate_mold_dispense_run(void) {
    while(!st_overheat()) {
        laminate_motor_run(false, direction, delay_us);
        mold_motor_run(false, direction, delay_us);
        despensing_motor_run(false, direction, delay_us);
        current_state = ST_CUTTER;
    }
}

// ST_CUTTER
// update direction/delay of motor
// update switch variable type
void st_cutter_run(bool bottom_switch, bool top_switch) {
    while(!st_overheat()) {
        while (!bottom_switch) {
            cutting_motor_run(true, direction, delay_us);
        }
        // switch direction
        cutting_motor_run(true, opposite direction, delay_us);
        if (top_switch) {
            cutting_motor_run(false, opposite direction, delay_us);
        }
        current_state =  ST_START_LAMINATE_MOLD_DISPENSE;
    }
}

// State Machine
void state_machine_thread(void) {
    while (1) {
        switch (current_state) {
            case ST_HEATER_ON:
                st_heater_on_run(temp);
            case ST_WAIT_START_BUTTON:
                st_wait_start_run(start_button);
            case ST_START_LAMINATE_MOLD_DISPENSE:
                st_start_laminate_mold_dispense_run(beam_breaker);
            case ST_BEAM_BREAKER_TIMER:
                st_beam_breaker_timer_run();
            case ST_CUTTER:
                st_cutter_run();
        }
    }
}
