// Button.h - Button input with debouncing

#ifndef HAL_BUTTON_H_
#define HAL_BUTTON_H_

#include <HAL/Timer.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define DEBOUNCE_TIME_MS 5
#define PRESSED 0
#define RELEASED 1

// Button port/pin definitions
#define LAUNCHPAD_S1_PORT GPIO_PORT_P1
#define LAUNCHPAD_S1_PIN GPIO_PIN1

#define LAUNCHPAD_S2_PORT GPIO_PORT_P1
#define LAUNCHPAD_S2_PIN GPIO_PIN4

#define BOOSTERPACK_S1_PORT GPIO_PORT_P5
#define BOOSTERPACK_S1_PIN GPIO_PIN1

#define BOOSTERPACK_S2_PORT GPIO_PORT_P3
#define BOOSTERPACK_S2_PIN GPIO_PIN5

#define BOOSTERPACK_JS_PORT GPIO_PORT_P4
#define BOOSTERPACK_JS_PIN GPIO_PIN1

// Debounce FSM states
enum _DebounceState { StableP, TransitionPR, TransitionRP, StableR };
typedef enum _DebounceState DebounceState;

struct _Button {
  uint8_t port;
  uint16_t pin;
  DebounceState debounceState;
  SWTimer timer;
  int pushState;
  bool isTapped;
};
typedef struct _Button Button;

Button Button_construct(uint8_t port, uint16_t pin);
bool Button_isPressed(Button* button);
bool Button_isTapped(Button* button);
void Button_refresh(Button* button);

#endif /* HAL_BUTTON_H_ */
