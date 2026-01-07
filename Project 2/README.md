# Color Jump

An infinite runner game built to demonstrate real-time graphics programming, analog input processing, and game physics on the MSP432P401R microcontroller.

## Overview

Jump over scrolling colored floor segments by matching your character's color to the floor. Use the joystick to swap colors through an intuitive color wheel system. The game features multiple difficulty modes and keeps track of high scores.

## Key Features

### Color Wheel Mechanic
- 5-color selection system with a center color and 4 directional swaps
- Push the joystick in any direction to instantly change your color
- On-screen color wheel shows what colors are available
- Adds a layer of strategy as you plan your color switches

### Dynamic Difficulty
- Easy Mode: Slower scroll speed, 1 point per second
- Hard Mode: Faster scroll speed, 3 points per second
- Increasing Mode: Difficulty ramps up as your score grows
- Configurable through the options menu

### Game Mechanics
- Smooth gravity-based jump physics
- Floor collision system where color matching determines survival
- Fall off the floor and it's game over
- Procedurally generated floor segments for endless gameplay

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

### State Machine

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

## Technologies Used

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

### Implementation Highlights

**Color Wheel System**: The joystick controls a 5-way color selection:
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

**Jump Physics**: Smooth arc movement with configurable height:
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

**Floor Segment Generation**: Procedurally generated colored floor:
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

## Technical Challenges

### Smooth Graphics at High Frame Rate
Redrawing the entire screen causes visible flicker. I used differential rendering, which only redraws pixels that changed. By tracking previous positions and using background-color overdraw for moving objects, the display stays smooth.

### Responsive Joystick Input
ADC noise can cause jittery or false direction readings. I added a dead zone around the center position and debounce timing. The joystick needs to be held in a direction briefly before the color swap registers, which prevents accidental changes.

### Synchronized Timing Systems
Multiple timers (jump, score, floor scroll) need to run independently but stay in sync. I built a software timer abstraction with independent tick counters so each system can check its own timer without blocking the others:
```c
if (SWTimer_expired(&app->jumpTimer)) {
    updateCharacter(app, hal);
    SWTimer_start(&app->jumpTimer);
}
```

### Color Matching Collision
Figuring out if the player's color matches the floor during overlap took some work. The solution checks the player's X position against the floor segment array and compares center color values, triggering game over on a mismatch.

## Demo

*Screenshots and gameplay video coming soon*

## Controls

| Input | Action |
|-------|--------|
| Joystick Up | Swap with top color |
| Joystick Down | Swap with bottom color |
| Joystick Left | Swap with left color |
| Joystick Right | Swap with right color |
| Button 1 | Jump |
| Button 2 | Menu select / Pause |

## Building and Running

1. Open project in Code Composer Studio
2. Connect MSP432 LaunchPad with BoosterPack MKII
3. Build and flash to device
4. Title screen auto-transitions to menu
5. Select difficulty in Options before starting
6. Jump and swap colors to survive as long as possible!
