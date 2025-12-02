// Button logic for up, down, left, right, and select with debouncing
#include "buttons.h"
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* Debounce configuration */
#define DEBOUNCE_TIME_MS 50  /* 50ms debounce time */
#define HOLD_TIME_MS 500     /* 500ms to trigger held event */

/* Button GPIO specifications - adjust pins to match your hardware */
/* PE11-PE15: UI Buttons 1-5 */
static const struct gpio_dt_spec button_specs[BUTTON_COUNT] = {
    [BUTTON_UP]     = GPIO_DT_SPEC_GET(DT_NODELABEL(button_up), gpios),     /* PE11 - UI Button 1 */
    [BUTTON_DOWN]   = GPIO_DT_SPEC_GET(DT_NODELABEL(button_down), gpios),   /* PE12 - UI Button 2 */
    [BUTTON_LEFT]   = GPIO_DT_SPEC_GET(DT_NODELABEL(button_left), gpios),   /* PE13 - UI Button 3 */
    [BUTTON_RIGHT]  = GPIO_DT_SPEC_GET(DT_NODELABEL(button_right), gpios),  /* PE14 - UI Button 4 */
    [BUTTON_SELECT] = GPIO_DT_SPEC_GET(DT_NODELABEL(button_select), gpios), /* PE15 - UI Button 5 */
};

/* Button state tracking */
typedef struct {
    bool current_state;      /* Current debounced state */
    bool last_state;         /* Previous state for edge detection */
    bool raw_state;          /* Raw GPIO reading */
    uint32_t last_change_time; /* Time of last state change */
    uint32_t press_time;     /* Time when button was pressed */
    button_event_t event;    /* Pending event */
    bool held_fired;         /* Whether hold event has been fired */
} button_state_t;

static button_state_t button_states[BUTTON_COUNT];
static button_callback_t event_callback = NULL;

/* Initialize button GPIO and debouncing */
int buttons_init(void)
{
    int ret;

    /* Initialize all button GPIOs */
    for (int i = 0; i < BUTTON_COUNT; i++) {
        if (!gpio_is_ready_dt(&button_specs[i])) {
            printk("Button %d GPIO not ready\n", i);
            return -1;
        }

        ret = gpio_pin_configure_dt(&button_specs[i], GPIO_INPUT | GPIO_PULL_DOWN);
        if (ret < 0) {
            printk("Failed to configure button %d GPIO\n", i);
            return ret;
        }

        /* Initialize state */
        button_states[i].current_state = false;
        button_states[i].last_state = false;
        button_states[i].raw_state = false;
        button_states[i].last_change_time = 0;
        button_states[i].press_time = 0;
        button_states[i].event = BUTTON_EVENT_NONE;
        button_states[i].held_fired = false;
    }

    printk("Buttons initialized\n");
    return 0;
}

/* Poll all buttons - call this regularly (e.g., every 10ms) */
void buttons_poll(void)
{
    uint32_t now = k_uptime_get_32();

    for (int i = 0; i < BUTTON_COUNT; i++) {
        button_state_t *state = &button_states[i];

        /* Read raw button state (active low with pull-up) */
        bool raw = gpio_pin_get_dt(&button_specs[i]);

        /* Debounce logic */
        if (raw != state->raw_state) {
            state->raw_state = raw;
            state->last_change_time = now;
        }

        /* Update debounced state if enough time has passed */
        if ((now - state->last_change_time) >= DEBOUNCE_TIME_MS) {
            if (state->raw_state != state->current_state) {
                state->current_state = state->raw_state;

                /* Detect edge transitions */
                if (state->current_state && !state->last_state) {
                    /* Button pressed */
                    state->event = BUTTON_EVENT_PRESSED;
                    state->press_time = now;
                    state->held_fired = false;

                    if (event_callback) {
                        event_callback(i, BUTTON_EVENT_PRESSED);
                    }
                } else if (!state->current_state && state->last_state) {
                    /* Button released */
                    state->event = BUTTON_EVENT_RELEASED;

                    if (event_callback) {
                        event_callback(i, BUTTON_EVENT_RELEASED);
                    }
                }

                state->last_state = state->current_state;
            }
        }

        /* Check for held button */
        if (state->current_state && !state->held_fired) {
            if ((now - state->press_time) >= HOLD_TIME_MS) {
                state->event = BUTTON_EVENT_HELD;
                state->held_fired = true;

                if (event_callback) {
                    event_callback(i, BUTTON_EVENT_HELD);
                }
            }
        }
    }
}

/* Get current state of a button (true = pressed) */
bool button_is_pressed(button_id_t button)
{
    if (button >= BUTTON_COUNT) {
        return false;
    }
    return button_states[button].current_state;
}

/* Get button event (clears event after reading) */
button_event_t button_get_event(button_id_t button)
{
    if (button >= BUTTON_COUNT) {
        return BUTTON_EVENT_NONE;
    }

    button_event_t event = button_states[button].event;
    button_states[button].event = BUTTON_EVENT_NONE;
    return event;
}

/* Register a callback for button events */
void buttons_set_callback(button_callback_t callback)
{
    event_callback = callback;
}
