// Button.c - Button debouncing implementation

#include <HAL/Button.h>

// Create and initialize a button
Button Button_construct(uint8_t port, uint16_t pin) {
  Button button;
  button.port = port;
  button.pin = pin;

  GPIO_setAsInputPinWithPullUpResistor(port, pin);

  button.debounceState = StableR;
  button.timer = SWTimer_construct(DEBOUNCE_TIME_MS);
  SWTimer_start(&button.timer);
  button.pushState = RELEASED;
  button.isTapped = false;

  return button;
}

bool Button_isPressed(Button* button) {
  return button->pushState == PRESSED;
}

bool Button_isTapped(Button* button) {
  return button->isTapped;
}

// Update button state - call once per main loop
void Button_refresh(Button* button) {
  uint8_t port = button->port;
  uint16_t pin = button->pin;
  uint16_t rawButtonStatus = GPIO_getInputPinValue(port, pin);
  int newPushState = RELEASED;

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

  button->isTapped = newPushState == PRESSED && button->pushState == RELEASED;
  button->pushState = newPushState;
}
