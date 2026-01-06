# Maze Game

A terminal-based maze navigation game developed to demonstrate **UART serial communication**, **finite state machine design**, and **real-time embedded systems programming** on the MSP432P401R microcontroller.

## Overview

Navigate through a procedurally-drawn maze while avoiding an enemy AI that hunts you down. The game features real-time serial communication with a terminal emulator, visual feedback through LCD graphics, and a persistent high score system.

## Key Features

### Serial Communication
- **Multi-baud rate support**: 9600, 19200, 38400, and 57600 bps with visual LED indicators
- **Bidirectional UART**: Receives player commands, transmits game prompts and status
- **Input validation**: Robust handling of valid/invalid commands with user feedback

### Game Mechanics
- **10x10 maze navigation** with wall collision detection
- **Enemy AI** with randomized movement patterns that tracks the player
- **Win/lose conditions**: Reach the goal to win, collide with the enemy to lose
- **Move counter**: Tracks efficiency—lower moves = better score
- **High score persistence**: Top 5 scores saved and sorted

### User Interface
- **LCD maze visualization**: Real-time rendering of maze, player, and enemy positions
- **Menu system**: Main menu, instructions, high scores, and game screens
- **Joystick navigation**: Intuitive menu selection with visual cursor feedback
- **LED status indicators**: Baud rate confirmation through colored LED patterns

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

### State Machine Design

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

## Technologies & Peripherals

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

### Key Implementation Highlights

**Non-Blocking Architecture**: The main loop continuously polls for input without blocking, ensuring responsive gameplay:
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

**Maze Collision Detection**: Validates moves against wall positions before updating player location:
```c
bool isValidMove(Application* app, GFX* gfx, int new_x, int new_y) {
    // Boundary and wall collision checks
    return (maze[new_y][new_x] != '#');
}
```

## Technical Challenges & Solutions

### Challenge: Baud Rate Synchronization
**Problem**: Terminal and MCU must use matching baud rates for reliable communication.
**Solution**: Implemented visual LED feedback system—each baud rate displays a unique LED color pattern, allowing users to verify settings at a glance.

### Challenge: Enemy AI Movement
**Problem**: Creating engaging enemy behavior without complex pathfinding on limited resources.
**Solution**: Implemented randomized movement with basic player-tracking tendency, creating unpredictable but challenging gameplay.

### Challenge: Non-Blocking Game Loop
**Problem**: UART reads can block execution, causing missed inputs or unresponsive UI.
**Solution**: Used `UART_hasChar()` polling pattern to check for available data without blocking, maintaining smooth 60Hz+ update rates.

## Demo

*Screenshots and gameplay video coming soon*

<!--
TODO: Add media
![Main Menu](./media/main_menu.png)
![Gameplay](./media/gameplay.gif)
-->

## Controls

| Input | Action |
|-------|--------|
| `W` | Move up |
| `A` | Move left |
| `S` | Move down |
| `D` | Move right |
| Joystick | Menu navigation |
| Button | Menu selection |

## Building & Running

1. Open project in Code Composer Studio
2. Connect MSP432 LaunchPad with BoosterPack
3. Build and flash to device
4. Open terminal emulator (PuTTY, Tera Term, etc.)
5. Configure serial port: Select matching baud rate, 8N1
6. Navigate menu with joystick, play game via terminal commands
