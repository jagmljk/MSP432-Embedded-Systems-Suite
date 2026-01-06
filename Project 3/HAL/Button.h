/*
 * Button.h
 *
 * Interface for handling button inputs with debouncing.
 * Supports 5 buttons: LB1, LB2, BB1, BB2, and joystick button (JSB).
 */

#ifndef HAL_BUTTON_H_
#define HAL_BUTTON_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

// Holds the tap status for all buttons - true if tapped this cycle
typedef struct {
  bool LB1tapped;
  bool LB2tapped;
  bool BB1tapped;
  bool BB2tapped;
  bool JSBtapped;
} buttons_t;

// Sets up GPIO pins and interrupts for all buttons
void initButtons();

// Checks all buttons and returns which ones were tapped
buttons_t updateButtons();

#endif /* HAL_BUTTON_H_ */
