# Maze Game

A terminal-based maze navigation game built to demonstrate UART serial communication, finite state machine design, and real-time embedded systems programming on the MSP432P401R microcontroller.

## Overview

The player navigates through a maze while avoiding an enemy that hunts them down. The game uses real-time serial communication with a terminal emulator, displays visual feedback on an LCD, and tracks high scores.

## Key Features

### Serial Communication
- Supports multiple baud rates: 9600, 19200, 38400, and 57600 bps
- Visual LED indicators show the current baud rate
- Bidirectional UART receives player commands and transmits game prompts
- Handles both valid and invalid input with appropriate feedback

### Game Mechanics
- 10x10 maze with wall collision detection
- Enemy AI with randomized movement that tracks the player
- Win by reaching the goal, lose by colliding with the enemy
- Move counter tracks efficiency (lower is better)
- Top 5 high scores saved and sorted

### User Interface
- Real-time LCD rendering of the maze, player, and enemy
- Menu system with main menu, instructions, high scores, and game screens
- Joystick navigation for menu selection
- LED patterns confirm baud rate settings

## System Architecture

```
┌─────────────────┐     UART      ┌──────────────────┐
│  Terminal       │◄────────────►│   MSP432P401R    │
│  (PuTTY/etc)    │               │                  │
└─────────────────┘               │  ┌────────────┐  │
                                  │  │    FSM     │  │
                                  │  │            │  │
┌─────────────────┐               │  │ MAIN_MENU  │  │
│   LCD Display   │◄──── SPI ────│  │ INSTRUCT.  │  │
│   128x128       │               │  │ HIGHSCORE  │  │
└─────────────────┘               │  │ GAME       │  │
                                  │  └────────────┘  │
┌─────────────────┐               │                  │
│   Joystick      │───── ADC ────│                  │
│   + Buttons     │               └──────────────────┘
└─────────────────┘
```

### State Machine

```
MAIN_MENU ──────► SEE_INSTRUCTIONS ──────┐
    │                                     │
    ├────────► SEE_HIGHSCORE ─────────────┤
    │                                     │
    └────────► START_GAME                 │
                   │                      │
                   ▼                      │
              GAME_RUNNING                │
                   │                      │
              ┌────┴────┐                 │
              ▼         ▼                 │
          GAME_WIN   GAME_OVER           │
              │         │                 │
              └────┬────┘                 │
                   ▼                      │
              MAIN_MENU ◄─────────────────┘
```

## Technologies Used

| Component | Purpose |
|-----------|---------|
| **UART** | Serial communication with terminal (TX/RX) |
| **SPI** | LCD display rendering |
| **ADC** | Joystick input for menu navigation |
| **GPIO** | Button inputs, LED outputs |
| **Timers** | Game loop timing, debouncing |

## Code Organization

```
Project 1/
├── proj1_main.c        # Main entry point and HAL initialization
├── Application.c       # Core game logic and state machine
├── Application.h       # Application structures and function declarations
└── HAL/
    ├── HAL.c/h         # Hardware abstraction layer
    ├── Graphics.c/h    # LCD drawing primitives
    ├── Timer.c/h       # Software timer implementation
    └── UART.c/h        # Serial communication driver
```

### Implementation Highlights

**Non-Blocking Architecture**: The main loop polls for input without blocking, keeping gameplay responsive:
```c
void Application_loop(Application* app, HAL* hal) {
    // Non-blocking UART check
    if (UART_hasChar(&hal->uart)) {
        char rxChar = UART_getChar(&hal->uart);
        processPlayerCommand(app, hal, rxChar);
    }
    // State-based screen handling
    switch(app->state) { ... }
}
```

**Maze Collision Detection**: Validates moves against wall positions before updating the player location:
```c
bool isValidMove(Application* app, GFX* gfx, int new_x, int new_y) {
    // Boundary and wall collision checks
    return (maze[new_y][new_x] != '#');
}
```

## Technical Challenges

### Baud Rate Synchronization
The terminal and MCU need matching baud rates for reliable communication. I implemented a visual LED feedback system where each baud rate displays a unique color pattern so users can verify their settings at a glance.

### Enemy AI Movement
Creating engaging enemy behavior without complex pathfinding on limited resources was tricky. I went with randomized movement that has a tendency to track the player, which creates unpredictable but challenging gameplay.

### Non-Blocking Game Loop
UART reads can block execution and cause missed inputs or unresponsive UI. Using a `UART_hasChar()` polling pattern to check for data without blocking keeps the game running smoothly.

## Demo

*Screenshots and gameplay video coming soon*

## Controls

| Input | Action |
|-------|--------|
| `W` | Move up |
| `A` | Move left |
| `S` | Move down |
| `D` | Move right |
| Joystick | Menu navigation |
| Button | Menu selection |

## Building and Running

1. Open project in Code Composer Studio
2. Connect MSP432 LaunchPad with BoosterPack
3. Build and flash to device
4. Open terminal emulator (PuTTY, Tera Term, etc.)
5. Configure serial port with matching baud rate, 8N1
6. Navigate menu with joystick, play game via terminal commands
