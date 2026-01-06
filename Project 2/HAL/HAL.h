// HAL.h - Hardware Abstraction Layer

#ifndef HAL_HAL_H_
#define HAL_HAL_H_

#include <HAL/Button.h>
#include <HAL/LED.h>
#include <HAL/Timer.h>
#include <HAL/UART.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include <HAL/Graphics.h>
#include <HAL/Joystick.h>

// Main HAL struct - contains all hardware peripherals
struct _HAL {
  // Launchpad LEDs
  LED launchpadLED1;
  LED launchpadLED2Red;
  LED launchpadLED2Blue;
  LED launchpadLED2Green;

  // Boosterpack LEDs
  LED boosterpackRed;
  LED boosterpackBlue;
  LED boosterpackGreen;

  // Launchpad buttons
  Button launchpadS1;
  Button launchpadS2;

  // Boosterpack buttons
  Button boosterpackS1;
  Button boosterpackS2;
  Button boosterpackJS;

  Joystick joystick;
  UART uart;
  GFX gfx;
};
typedef struct _HAL HAL;

HAL HAL_construct();
void HAL_refresh(HAL* api);

#endif /* HAL_HAL_H_ */
