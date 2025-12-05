#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/smf.h>
// add ESTOP function
// State enum
typedef enum {
    ST_ESTOP;
    ST_HEATER_ON;                       // heater turns on when power is on 
    ST_WAIT_START_BUTTON;               // heating is ready, waiting until start button pressed
    ST_START_LAMINATE_MOLD_DISPENSE;    // start button pressed, begin laminate/mold/dispense
    ST_BEAM_BREAKER_TIMER;              // stop lamintae
    ST_CUTTER;                          // cutter motor activated for static down then up
    ST_MAX_STATES;                      // this is a COUNT, NOT a state
} MyState;

// State table
static const state_handlers state_table[ST_MAX_STATES] = {
    ST_HEATER_ON =                      {st_heater_on_run},
    ST_WAIT_START_BUTTON =              {st_wait_start_button_run},
    ST_START_LAMINATE_MOLD_DISPENSE =   {st_start_laminate_mold_dispense_run,},
    ST_BEAM_BREAKER_TIMER =             {st_beam_breaker_timer_run},
    ST_CUTTER =                         {st_cutter_run},
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

// check for overheating
void st_overheat() {
    // enter ESTOP state
    if (temp > max) {
        // shut off power
    }
    else {
        return false;
    }
}

// ST_HEATER_ON
void st_heater_on_run(int temp) {
    // heater runs until temperature is in range
    while(!st_overheat()) {
        current_state = ST_WAIT_START_BUTTON
    }
} 

// ST_WAIT_START_BUTTON
void st_wait_start_run(int start_button) {
    // wait until user presses start button
    // we should be checking temperature to make sure it's in range while waiting
    // but for simplicity lets assume our user presses the start button immediately
    while(!st_overheat()) {
        if (start_button) {
            current_state = ST_START_LAMINATE_MOLD_DISPENSE;
        }
    }
}

// ST_LAMINATE_MOLD_DISPENSE
void st_start_laminate_mold_dispense_run(int beam_breaker) {
    // turn on all three motors simultaneously and run
    while(!st_overheat()) {
        if (beam_breaker) {
            current_state = ST_BEAM_BREAKER_TIMER
        }
    }
}

// ST_BEAM_BREAKER_TIMER
void st_beam_breaker_timer_run(void) {
    while(!st_overheat()) {
        // run motors for x seconds
        current_state = ST_CUTTER;
    }
}

// ST_CUTTER
void st_cutter_run() {
    // run cutting motor for x seconds
    while(!st_overheat()) {}
}

// State Machine
void state_machine_thread(void) {
    while (1) {
        // put while loop in every case and check estop
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
