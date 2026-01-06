/*
 * LED.h
 *
 * Controls all LEDs on the MSP432 launchpad and boosterpack.
 * LL = Launchpad LED, BL = Boosterpack LED
 * R/G/B = Red/Green/Blue color channels
 */

#ifndef HAL_LED_H_
#define HAL_LED_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

void initLEDs();

// Launchpad LED 1 (single red LED)
void TurnOn_LL1();
void TurnOff_LL1();
void Toggle_LL1();

// Launchpad RGB LED - Blue channel
void TurnOn_LLB();
void TurnOff_LLB();
void Toggle_LLB();

// Launchpad RGB LED - Red channel
void TurnOn_LLR();
void TurnOff_LLR();
void Toggle_LLR();

// Launchpad RGB LED - Green channel
void TurnOn_LLG();
void TurnOff_LLG();
void Toggle_LLG();

// Boosterpack RGB LED - Red channel
void TurnOn_BLR();
void TurnOff_BLR();
void Toggle_BLR();

// Boosterpack RGB LED - Green channel
void TurnOn_BLG();
void TurnOff_BLG();
void Toggle_BLG();

// Boosterpack RGB LED - Blue channel
void TurnOn_BLB();
void TurnOff_BLB();
void Toggle_BLB();

#endif /* HAL_LED_H_ */
