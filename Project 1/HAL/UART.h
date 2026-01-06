/*
 * UART.h - Serial communication
 */

#ifndef HAL_UART_H_
#define HAL_UART_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define UART_Config eUSCI_UART_ConfigV1

#define USB_UART_PORT GPIO_PORT_P1
#define USB_UART_PINS (GPIO_PIN2 | GPIO_PIN3)
#define USB_UART_INSTANCE EUSCI_A0_BASE

// Baud rate options
enum _UART_Baudrate {
    BAUD_9600,
    BAUD_19200,
    BAUD_38400,
    BAUD_57600,
    NUM_BAUD_CHOICES
};
typedef enum _UART_Baudrate UART_Baudrate;

// UART struct
struct _UART {
    UART_Config config;
    uint32_t moduleInstance;
    uint32_t port;
    uint32_t pins;
};
typedef struct _UART UART;

// Create UART instance
UART UART_construct(uint32_t moduleInstance, uint32_t port, uint32_t pins);

// Set baud rate and enable UART
void UART_SetBaud_Enable(UART*, UART_Baudrate baudrate);

// Check if character available to read
bool UART_hasChar(UART* uart_p);

// Read a character
char UART_getChar(UART* uart_p);

// Check if ready to send
bool UART_canSend(UART* uart_p);

// Send a character
void UART_sendChar(UART* uart_p, char c);

// Update baud rate
void UART_updateBaud(UART* uart_p, UART_Baudrate baudChoice);

// Send a string
void UART_sendString(UART* uart_p, const char* str);

#endif /* HAL_UART_H_ */
