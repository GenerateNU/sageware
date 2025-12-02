// Button logic for up, down, left, right, and select with debouncing
#ifndef BUTTONS_H_
#define BUTTONS_H_

#include <stdint.h>
#include <stdbool.h>

/* Button identifiers */
typedef enum {
    BUTTON_UP,
    BUTTON_DOWN,
    BUTTON_LEFT,
    BUTTON_RIGHT,
    BUTTON_SELECT,
    BUTTON_COUNT  /* Total number of buttons */
} button_id_t;

/* Button event types */
typedef enum {
    BUTTON_EVENT_NONE,
    BUTTON_EVENT_PRESSED,   /* Button was just pressed */
    BUTTON_EVENT_RELEASED,  /* Button was just released */
    BUTTON_EVENT_HELD       /* Button is being held down */
} button_event_t;

/* Button callback function type */
typedef void (*button_callback_t)(button_id_t button, button_event_t event);

/* Initialize button GPIO and debouncing */
int buttons_init(void);

/* Poll all buttons - call this regularly (e.g., every 10ms) */
void buttons_poll(void);

/* Get current state of a button (true = pressed) */
bool button_is_pressed(button_id_t button);

/* Get button event (clears event after reading) */
button_event_t button_get_event(button_id_t button);

/* Register a callback for button events */
void buttons_set_callback(button_callback_t callback);

#endif /* BUTTONS_H_ */