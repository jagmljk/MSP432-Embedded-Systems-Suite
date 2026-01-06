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

// Main HAL struct containing all hardware peripherals
struct _HAL {
    // Left launchpad LED (red only)
    LED launchpadLED1;

    // Right launchpad RGB LED
    LED launchpadLED2Red;
    LED launchpadLED2Blue;
    LED launchpadLED2Green;

    // Boosterpack RGB LED
    LED boosterpackRed;
    LED boosterpackBlue;
    LED boosterpackGreen;

    // Launchpad buttons
    Button launchpadS1;
    Button launchpadS2;

    // Boosterpack buttons and joystick
    Button boosterpackS1;
    Button boosterpackS2;
    Button boosterpackJS;

    UART uart;
    GFX gfx;
};
typedef struct _HAL HAL;

// Create and initialize HAL
HAL HAL_construct();

// Update all inputs
void HAL_refresh(HAL* api);

#endif /* HAL_HAL_H_ */
