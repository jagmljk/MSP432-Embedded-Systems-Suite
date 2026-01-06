// LED.c - LED control implementation

#include <HAL/LED.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

LED LED_construct(uint8_t port, uint16_t pin) {
  LED led;
  led.isLit = false;
  led.port = port;
  led.pin = pin;

  GPIO_setAsOutputPin(led.port, led.pin);
  GPIO_setOutputLowOnPin(led.port, led.pin);

  return led;
}

void LED_turnOn(LED* led) {
  led->isLit = true;
  GPIO_setOutputHighOnPin(led->port, led->pin);
}

void LED_turnOff(LED* led) {
  led->isLit = false;
  GPIO_setOutputLowOnPin(led->port, led->pin);
}

void LED_toggle(LED* led) {
  led->isLit = !led->isLit;
  GPIO_toggleOutputOnPin(led->port, led->pin);
}

bool LED_isLit(LED* led) {
  return led->isLit;
}
