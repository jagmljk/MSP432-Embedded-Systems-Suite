/*
 * LED.c - LED control
 */

#include <HAL/LED.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

// Create and initialize an LED
LED LED_construct(uint8_t port, uint16_t pin) {
    LED led;

    led.isLit = false;
    led.port = port;
    led.pin = pin;

    GPIO_setAsOutputPin(led.port, led.pin);
    GPIO_setOutputLowOnPin(led.port, led.pin);

    return led;
}

// Turn LED on
void LED_turnOn(LED* led) {
    uint8_t port = led->port;
    uint16_t pin = led->pin;

    led->isLit = true;
    GPIO_setOutputHighOnPin(port, pin);
}

// Turn LED off
void LED_turnOff(LED* led) {
    uint8_t port = led->port;
    uint16_t pin = led->pin;

    led->isLit = false;
    GPIO_setOutputLowOnPin(port, pin);
}

// Toggle LED state
void LED_toggle(LED* led) {
    uint8_t port = led->port;
    uint16_t pin = led->pin;

    led->isLit = !led->isLit;
    GPIO_toggleOutputOnPin(port, pin);
}

// Check if LED is on
bool LED_isLit(LED* led) {
    return led->isLit;
}
