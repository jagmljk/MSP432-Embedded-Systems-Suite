// Joystick.h - Joystick input via ADC

#ifndef HAL_JOYSTICK_H_
#define HAL_JOYSTICK_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

struct _Joystick {
    uint_fast16_t x;
    uint_fast16_t y;
};
typedef struct _Joystick Joystick;

Joystick Joystick_construct();
bool Joystick_isPressedToLeft(Joystick* Joystick);
bool Joystick_isTappedToLeft(Joystick* Joystick);
void Joystick_refresh(Joystick* Joystick);

#endif /* HAL_JOYSTICK_H_ */
