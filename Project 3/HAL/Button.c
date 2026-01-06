/*
 * Button.c
 *
 * Handles button input using GPIO interrupts and software debouncing.
 * Each button has a 300ms debounce window to prevent multiple triggers.
 */

#include "HAL/Button.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "HAL/LED.h"
#include "HAL/Timer.h"

// Flags set by interrupt handlers when a button press is detected
volatile static bool JSBmodified;
volatile static bool BB1modified;
volatile static bool BB2modified;
volatile static bool LB1modified;

#define DEBOUNCE_WAIT 300  // Debounce time in milliseconds

// Helper to configure a single button pin with pull-up and falling edge interrupt
void initButton(uint_fast8_t selectedPort, uint_fast16_t selectedPins) {
    GPIO_setAsInputPinWithPullUpResistor(selectedPort, selectedPins);
    GPIO_clearInterruptFlag(selectedPort, selectedPins);
    GPIO_enableInterrupt(selectedPort, selectedPins);
    GPIO_interruptEdgeSelect(selectedPort, selectedPins, GPIO_HIGH_TO_LOW_TRANSITION);
}

void initButtons() {
    // Joystick button on P4.1
    initButton(GPIO_PORT_P4, GPIO_PIN1);
    Interrupt_enableInterrupt(INT_PORT4);
    JSBmodified = false;

    // Boosterpack button 1 on P5.1
    initButton(GPIO_PORT_P5, GPIO_PIN1);
    Interrupt_enableInterrupt(INT_PORT5);
    BB1modified = false;

    // Boosterpack button 2 on P3.5
    initButton(GPIO_PORT_P3, GPIO_PIN5);
    Interrupt_enableInterrupt(INT_PORT3);
    BB2modified = false;

    // Launchpad button 1 on P1.1
    initButton(GPIO_PORT_P1, GPIO_PIN1);
    Interrupt_enableInterrupt(INT_PORT1);
    LB1modified = false;
}

// Interrupt handlers - just set the flag and clear the interrupt
void PORT4_IRQHandler() {
    if (GPIO_getInterruptStatus(GPIO_PORT_P4, GPIO_PIN1)) {
        JSBmodified = true;
        GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN1);
    }
}

void PORT5_IRQHandler(void) {
    if (GPIO_getInterruptStatus(GPIO_PORT_P5, GPIO_PIN1)) {
        BB1modified = true;
        GPIO_clearInterruptFlag(GPIO_PORT_P5, GPIO_PIN1);
    }
}

void PORT3_IRQHandler(void) {
    if (GPIO_getInterruptStatus(GPIO_PORT_P3, GPIO_PIN5)) {
        BB2modified = true;
        GPIO_clearInterruptFlag(GPIO_PORT_P3, GPIO_PIN5);
    }
}

void PORT1_IRQHandler(void) {
    if (GPIO_getInterruptStatus(GPIO_PORT_P1, GPIO_PIN1)) {
        LB1modified = true;
        GPIO_clearInterruptFlag(GPIO_PORT_P1, GPIO_PIN1);
    }
}

// Debounced tap detection for joystick button
bool JSBtapped() {
    static bool debouncing = false;
    static SWTimer debounceTimer;
    bool tapped = false;

    if (debouncing && SWTimer_expired(&debounceTimer)) debouncing = false;

    if (!debouncing && JSBmodified) {
        tapped = true;
        debouncing = true;
        debounceTimer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&debounceTimer);
    }

    JSBmodified = false;
    return tapped;
}

// Debounced tap detection for boosterpack button 1
bool BB1tapped() {
    static bool debouncing = false;
    static SWTimer timer;
    bool tapped = false;

    if (debouncing && SWTimer_expired(&timer)) {
        debouncing = false;
    }

    if (!debouncing && BB1modified) {
        tapped = true;
        debouncing = true;
        timer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&timer);
    }

    BB1modified = false;
    return tapped;
}

// Debounced tap detection for boosterpack button 2
bool BB2tapped() {
    static bool debouncing = false;
    static SWTimer timer;
    bool tapped = false;

    if (debouncing && SWTimer_expired(&timer)) {
        debouncing = false;
    }

    if (!debouncing && BB2modified) {
        tapped = true;
        debouncing = true;
        timer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&timer);
    }

    BB2modified = false;
    return tapped;
}

// Debounced tap detection for launchpad button 1
bool LB1tapped() {
    static bool debouncing = false;
    static SWTimer timer;
    bool tapped = false;

    if (debouncing && SWTimer_expired(&timer)) {
        debouncing = false;
    }

    if (!debouncing && LB1modified) {
        tapped = true;
        debouncing = true;
        timer = SWTimer_construct(DEBOUNCE_WAIT);
        SWTimer_start(&timer);
    }

    LB1modified = false;
    return tapped;
}

// Poll all buttons and return their tap status
buttons_t updateButtons() {
    buttons_t buttons;
    buttons.JSBtapped = JSBtapped();
    buttons.BB1tapped = BB1tapped();
    buttons.BB2tapped = BB2tapped();
    buttons.LB1tapped = LB1tapped();
    return buttons;
}
