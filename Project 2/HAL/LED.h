// LED.h - LED control

#ifndef HAL_LED_H_
#define HAL_LED_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

// LED port/pin definitions
#define LAUNCHPAD_LED1_PORT GPIO_PORT_P1
#define LAUNCHPAD_LED1_PIN GPIO_PIN0

#define LAUNCHPAD_LED2_RED_PORT GPIO_PORT_P2
#define LAUNCHPAD_LED2_RED_PIN GPIO_PIN0

#define LAUNCHPAD_LED2_GREEN_PORT GPIO_PORT_P2
#define LAUNCHPAD_LED2_GREEN_PIN GPIO_PIN1

#define LAUNCHPAD_LED2_BLUE_PORT GPIO_PORT_P2
#define LAUNCHPAD_LED2_BLUE_PIN GPIO_PIN2

#define BOOSTERPACK_LED_RED_PORT GPIO_PORT_P2
#define BOOSTERPACK_LED_RED_PIN GPIO_PIN6

#define BOOSTERPACK_LED_GREEN_PORT GPIO_PORT_P2
#define BOOSTERPACK_LED_GREEN_PIN GPIO_PIN4

#define BOOSTERPACK_LED_BLUE_PORT GPIO_PORT_P5
#define BOOSTERPACK_LED_BLUE_PIN GPIO_PIN6

struct _LED {
  uint8_t port;
  uint16_t pin;
  bool isLit;
};
typedef struct _LED LED;

LED LED_construct(uint8_t port, uint16_t pin);
void LED_turnOn(LED* led);
void LED_turnOff(LED* led);
void LED_toggle(LED* led);
bool LED_isLit(LED* led);

#endif /* HAL_LED_H_ */
