/*
 * UART.c - Serial communication
 */

#include <HAL/Timer.h>
#include <HAL/UART.h>

// Create and initialize UART
UART UART_construct(uint32_t moduleInstance, uint32_t port, uint32_t pins) {
    UART uart;

    uart.moduleInstance = moduleInstance;
    uart.port = port;
    uart.pins = pins;

    GPIO_setAsPeripheralModuleFunctionInputPin(uart.port, uart.pins,
                                               GPIO_PRIMARY_MODULE_FUNCTION);

    uart.config.parity = EUSCI_A_UART_NO_PARITY;
    uart.config.dataLength = EUSCI_A_UART_8_BIT_LEN;
    uart.config.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;
    uart.config.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;
    uart.config.uartMode = EUSCI_A_UART_MODE;

    return uart;
}

// Set baud rate and enable UART
void UART_SetBaud_Enable(UART* uart_p, UART_Baudrate baudChoice) {
    uart_p->config.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;
    uart_p->config.overSampling = EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION;

    uint32_t clockPrescalerMapping[NUM_BAUD_CHOICES] = {312, 156, 78, 52};
    uint32_t firstModRegMapping[NUM_BAUD_CHOICES] = {8, 4, 2, 1};
    uint32_t secondModRegMapping[NUM_BAUD_CHOICES] = {0, 0, 0, 0x25};

    uart_p->config.clockPrescalar = clockPrescalerMapping[baudChoice];
    uart_p->config.firstModReg = firstModRegMapping[baudChoice];
    uart_p->config.secondModReg = secondModRegMapping[baudChoice];

    UART_initModule(uart_p->moduleInstance, &(uart_p->config));
    UART_enableModule(uart_p->moduleInstance);
}

// Check if character is available
bool UART_hasChar(UART* uart_p) {
    uint8_t status = UART_getInterruptStatus(uart_p->moduleInstance,
                                             EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
    return (status == EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG);
}

// Read a character
char UART_getChar(UART* uart_p) {
    return (char)UART_receiveData(uart_p->moduleInstance);
}

// Check if ready to send
bool UART_canSend(UART* uart_p) {
    uint8_t status = UART_getInterruptStatus(uart_p->moduleInstance,
                                             EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG);
    return (status == EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG);
}

// Send a character
void UART_sendChar(UART* uart_p, char c) {
    if (UART_getInterruptStatus(uart_p->moduleInstance,
                                EUSCI_A_UART_TRANSMIT_INTERRUPT_FLAG)) {
        UART_transmitData(uart_p->moduleInstance, c);
    }
}

// Send a string with newline
void UART_sendString(UART* uart_p, const char* str) {
    while (*str) {
        while (!UART_canSend(uart_p));
        UART_sendChar(uart_p, *str);
        str++;
    }
    while (!UART_canSend(uart_p));
    UART_sendChar(uart_p, '\r');

    while (!UART_canSend(uart_p));
    UART_sendChar(uart_p, '\n');
}
