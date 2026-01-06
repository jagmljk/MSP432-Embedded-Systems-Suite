#include <HAL/HAL.h>

HAL HAL_construct() {
    HAL hal;

    hal.launchpadLED1 = LED_construct(LAUNCHPAD_LED1_PORT, LAUNCHPAD_LED1_PIN);
    hal.launchpadLED2Red = LED_construct(LAUNCHPAD_LED2_RED_PORT, LAUNCHPAD_LED2_RED_PIN);
    hal.launchpadLED2Green = LED_construct(LAUNCHPAD_LED2_GREEN_PORT, LAUNCHPAD_LED2_GREEN_PIN);
    hal.launchpadLED2Blue = LED_construct(LAUNCHPAD_LED2_BLUE_PORT, LAUNCHPAD_LED2_BLUE_PIN);

    hal.boosterpackRed = LED_construct(BOOSTERPACK_LED_RED_PORT, BOOSTERPACK_LED_RED_PIN);
    hal.boosterpackGreen = LED_construct(BOOSTERPACK_LED_GREEN_PORT, BOOSTERPACK_LED_GREEN_PIN);
    hal.boosterpackBlue = LED_construct(BOOSTERPACK_LED_BLUE_PORT, BOOSTERPACK_LED_BLUE_PIN);

    hal.launchpadS1 = Button_construct(LAUNCHPAD_S1_PORT, LAUNCHPAD_S1_PIN);
    hal.launchpadS2 = Button_construct(LAUNCHPAD_S2_PORT, LAUNCHPAD_S2_PIN);

    hal.boosterpackS1 = Button_construct(BOOSTERPACK_S1_PORT, BOOSTERPACK_S1_PIN);
    hal.boosterpackS2 = Button_construct(BOOSTERPACK_S2_PORT, BOOSTERPACK_S2_PIN);
    hal.boosterpackJS = Button_construct(BOOSTERPACK_JS_PORT, BOOSTERPACK_JS_PIN);

    hal.uart = UART_construct(USB_UART_INSTANCE, USB_UART_PORT, USB_UART_PINS);
    UART_SetBaud_Enable(&hal.uart, BAUD_9600);

    hal.gfx = GFX_construct(GRAPHICS_COLOR_WHITE, GRAPHICS_COLOR_BLACK);

    return hal;
}

void HAL_refresh(HAL* hal) {
    Button_refresh(&hal->launchpadS1);
    Button_refresh(&hal->launchpadS2);

    Button_refresh(&hal->boosterpackS1);
    Button_refresh(&hal->boosterpackS2);
    Button_refresh(&hal->boosterpackJS);
}
