/*
 * Button.c - Button input with debouncing
 */

#include <HAL/Button.h>

// Create and initialize a button
Button Button_construct(uint8_t port, uint16_t pin) {
    Button button;

    button.port = port;
    button.pin = pin;

    // Use pullup resistor for all buttons
    GPIO_setAsInputPinWithPullUpResistor(port, pin);

    // Start in released state
    button.debounceState = StableR;
    button.timer = SWTimer_construct(DEBOUNCE_TIME_MS);
    SWTimer_start(&button.timer);

    button.pushState = RELEASED;
    button.isTapped = false;

    return button;
}

// Check if button is currently pressed
bool Button_isPressed(Button* button) {
    return button->pushState == PRESSED;
}

// Check if button was tapped
bool Button_isTapped(Button* button) {
    return button->isTapped;
}

// Update button state with debouncing
void Button_refresh(Button* button) {
    uint8_t port = button->port;
    uint16_t pin = button->pin;

    uint16_t rawButtonStatus = GPIO_getInputPinValue(port, pin);
    int newPushState = RELEASED;

    // Debounce state machine
    switch (button->debounceState) {
        case StableR:
            if (rawButtonStatus == PRESSED) {
                SWTimer_start(&button->timer);
                button->debounceState = TransitionRP;
            }
            newPushState = RELEASED;
            break;

        case StableP:
            if (rawButtonStatus == RELEASED) {
                SWTimer_start(&button->timer);
                button->debounceState = TransitionPR;
            }
            newPushState = PRESSED;
            break;

        case TransitionRP:
            if (rawButtonStatus == RELEASED) {
                button->debounceState = StableR;
            } else if (SWTimer_expired(&button->timer)) {
                button->debounceState = StableP;
            }
            newPushState = RELEASED;
            break;

        case TransitionPR:
            if (rawButtonStatus == PRESSED) {
                button->debounceState = StableP;
            } else if (SWTimer_expired(&button->timer)) {
                button->debounceState = StableR;
            }
            newPushState = PRESSED;
    }

    // Detect tap: was released, now pressed
    button->isTapped = newPushState == PRESSED && button->pushState == RELEASED;
    button->pushState = newPushState;
}
