# Embedded Systems Portfolio

A collection of embedded systems projects developed on the **Texas Instruments MSP432P401R** microcontroller platform. These projects demonstrate proficiency in low-level hardware programming, real-time systems design, and hardware-software integration.

## About

I'm an embedded systems developer with hands-on experience in microcontroller programming, peripheral interfacing, and real-time application development. This portfolio showcases my ability to design and implement complete embedded solutions—from hardware abstraction layers to interactive user applications.

## Projects

| Project | Description | Key Technologies |
|---------|-------------|------------------|
| [Maze Game](./Project%201) | Terminal-based maze navigation game with enemy AI and real-time serial communication | UART, FSM, GPIO, Timers |
| [Color Jump](./Project%202) | Infinite runner game with color-matching mechanics and dynamic difficulty | SPI LCD, ADC Joystick, Timers, FSM |
| [Color Mixer](./Project%203) | Interactive RGB color mixing system with low-power design | ADC, PWM, Interrupts, LPM |

## Technology Stack

### Hardware Platform
- **MCU**: Texas Instruments MSP432P401R (ARM Cortex-M4F, 48MHz)
- **Display**: Crystalfontz 128x128 Color TFT LCD (ST7735 controller)
- **Input**: Analog joystick, pushbuttons, potentiometer
- **Output**: RGB LED, Boosterpack LEDs

### Peripherals & Protocols
- **UART** - Serial communication with configurable baud rates (9600-57600)
- **SPI** - High-speed LCD graphics rendering
- **ADC** - 14-bit analog-to-digital conversion for joystick and potentiometer input
- **PWM** - Pulse-width modulation for RGB LED color control
- **GPIO** - Digital I/O for buttons and status LEDs
- **Timers** - Hardware timers for game loops, debouncing, and time-based events

### Software Architecture
- **Hardware Abstraction Layer (HAL)** - Modular driver architecture for portable, maintainable code
- **Finite State Machines** - Structured application flow and game state management
- **Interrupt-Driven Design** - Efficient event handling with low-power mode support
- **Non-Blocking Operations** - Responsive systems without busy-wait delays

## Skills Demonstrated

- Bare-metal embedded C programming
- Hardware peripheral configuration and driver development
- Real-time system design and timing management
- Serial communication protocol implementation
- Graphics programming on resource-constrained systems
- Power-efficient embedded design
- Modular software architecture

## Repository Structure

```
├── Project 1/          # Maze Game - UART-based navigation game
├── Project 2/          # Color Jump - Joystick-controlled infinite runner
├── Project 3/          # Color Mixer - ADC/PWM color mixing system
└── README.md
```

## Getting Started

Each project is self-contained with its own build configuration. Projects were developed using **Code Composer Studio** with the MSP432 DriverLib.

### Prerequisites
- Texas Instruments MSP432P401R LaunchPad
- Educational BoosterPack MKII
- Code Composer Studio IDE
- MSP432 DriverLib

See individual project READMEs for specific setup instructions.
