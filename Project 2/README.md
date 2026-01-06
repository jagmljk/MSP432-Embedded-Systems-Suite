# Color Jump

An infinite runner game developed to demonstrate **real-time graphics programming**, **analog input processing**, and **game physics implementation** on the MSP432P401R microcontroller.

## Overview

Jump over scrolling colored floor segments by matching your character's color. Use the joystick to swap colors in real-time through an intuitive color wheel system. Features multiple difficulty modes and persistent high score tracking.

## Key Features

### Color Wheel Mechanic
- **5-color selection system**: Center color + 4 directional swaps
- **Joystick-driven swapping**: Push in any direction to instantly change color
- **Visual feedback**: On-screen color wheel shows available options
- **Strategic gameplay**: Plan color switches to survive longer runs

### Dynamic Difficulty
- **Easy Mode**: Slower scroll speed, 1 point per second
- **Hard Mode**: Faster scroll speed, 3 points per second
- **Increasing Mode**: Difficulty ramps up as score increases
- **Configurable settings**: Options menu for difficulty selection

### Game Mechanics
- **Smooth jump physics**: Gravity-based jumping with apex detection
- **Floor collision system**: Color matching determines survival
- **Falling death**: Miss the floor entirely and fall to game over
- **Endless gameplay**: Procedurally generated floor segments

## System Architecture

```
┌─────────────────────────────────────────────────────┐
│                   MSP432P401R                        │
│                                                      │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐      │
│  │  Title   │───►│   Menu   │───►│  Game    │      │
│  │  Screen  │    │  System  │    │  Loop    │      │
│  └──────────┘    └──────────┘    └──────────┘      │
│                        │              │             │
│                        ▼              ▼             │
│                  ┌──────────┐   ┌──────────┐       │
│                  │ Options  │   │ Game Over│       │
│                  │ High Scr │   │ + Score  │       │
│                  └──────────┘   └──────────┘       │
│                                                      │
└──────────┬────────────────┬────────────────┬────────┘
           │                │                │
      ┌────▼────┐     ┌────▼────┐     ┌────▼────┐
      │   SPI   │     │   ADC   │     │  Timer  │
      │   LCD   │     │Joystick │     │  System │
      └─────────┘     └─────────┘     └─────────┘
```

### State Machine Design

```
STATE_TITLE ──► STATE_MENU
                    │
        ┌───────────┼───────────┐
        ▼           ▼           ▼
   STATE_INSTR  STATE_HIGH  STATE_OPTIONS
        │           │           │
        └───────────┼───────────┘
                    ▼
              STATE_GAME
                    │
                    ▼
             STATE_GAMEOVER
                    │
                    ▼
              STATE_MENU
```

## Technologies & Peripherals

| Component | Purpose |
|-----------|---------|
| **SPI** | High-speed LCD graphics at 60Hz+ |
| **ADC** | Analog joystick position reading |
| **Timers** | Game timing, jump physics, floor scrolling |
| **GPIO** | Button input, LED feedback |

## Code Organization

```
Project 2/
├── proj2_main.c        # Entry point and initialization
├── Application.c       # Game logic, physics, rendering
├── Application.h       # Structures and declarations
└── HAL/
    ├── HAL.c/h         # Hardware abstraction layer
    ├── Graphics.c/h    # LCD primitives and text rendering
    ├── Timer.c/h       # Software timer management
    └── Joystick.c/h    # ADC-based joystick driver
```

### Key Implementation Highlights

**Color Wheel System**: Efficient 5-way color selection using joystick direction:
```c
typedef struct {
    int center;  // Current active color
    int up;      // Swap target: push up
    int down;    // Swap target: push down
    int left;    // Swap target: push left
    int right;   // Swap target: push right
} ColorWheel;

void updateColorWheel(Application* app, HAL* hal) {
    // Swap center with direction color on joystick input
    if (Joystick_isPushedUp(&hal->joystick)) {
        int temp = app->colorWheel.center;
        app->colorWheel.center = app->colorWheel.up;
        app->colorWheel.up = temp;
    }
    // ... similar for other directions
}
```

**Jump Physics**: Smooth arc with configurable height and timing:
```c
void updateCharacter(Application* app, HAL* hal) {
    if (app->jumpState == JUMP_UP) {
        app->playerY -= JUMP_VELOCITY;
        if (app->jumpProgress >= app->jumpHeight)
            app->jumpState = JUMP_DOWN;
    } else if (app->jumpState == JUMP_DOWN) {
        app->playerY += GRAVITY;
        // Ground collision detection
    }
}
```

**Floor Segment Generation**: Procedural colored floor with randomization:
```c
typedef struct {
    int x;       // Horizontal position
    int width;   // Segment width
    int color;   // Color from wheel palette
} FloorSegment;

void updateFloor(Application* app) {
    // Scroll segments left
    // Generate new segments at right edge
    // Remove off-screen segments
}
```

## Technical Challenges & Solutions

### Challenge: Smooth Graphics at High Frame Rate
**Problem**: Redrawing entire screen causes visible flicker and tears.
**Solution**: Implemented differential rendering—only redraw changed pixels. Track previous positions and use background-color overdraw for moving objects.

### Challenge: Responsive Joystick Input
**Problem**: ADC noise causes jittery or false direction readings.
**Solution**: Added dead zone around center position and debounce timing. Require sustained direction for color swap to prevent accidental changes.

### Challenge: Synchronized Timing Systems
**Problem**: Multiple timers (jump, score, floor scroll) must run independently but stay synchronized.
**Solution**: Used software timer abstraction with independent tick counters. Each system checks its own timer without blocking others:
```c
if (SWTimer_expired(&app->jumpTimer)) {
    updateCharacter(app, hal);
    SWTimer_start(&app->jumpTimer);
}
```

### Challenge: Color Matching Collision
**Problem**: Determining if player color matches floor segment during overlap.
**Solution**: Check collision at player's X position against floor segment array. Compare center color values and trigger game over on mismatch.

## Demo

*Screenshots and gameplay video coming soon*

<!--
TODO: Add media
![Title Screen](./media/title.png)
![Gameplay](./media/gameplay.gif)
![Color Wheel](./media/colorwheel.png)
-->

## Controls

| Input | Action |
|-------|--------|
| Joystick Up | Swap with top color |
| Joystick Down | Swap with bottom color |
| Joystick Left | Swap with left color |
| Joystick Right | Swap with right color |
| Button 1 | Jump |
| Button 2 | Menu select / Pause |

## Building & Running

1. Open project in Code Composer Studio
2. Connect MSP432 LaunchPad with BoosterPack MKII
3. Build and flash to device
4. Title screen auto-transitions to menu
5. Select difficulty in Options before starting
6. Jump and swap colors to survive as long as possible!
