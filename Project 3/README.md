# Color Mixer

An interactive RGB color mixing system built to demonstrate interrupt-driven architecture, PWM control, ADC input processing, and low-power design on the MSP432P401R microcontroller.

## Overview

Create custom colors by adjusting RGB values with a potentiometer and joystick. Your color shows up in real-time on both the LCD and a physical RGB LED using PWM. There's also a sequence playback feature that lets you save and cycle through your favorite colors.

## Key Features

### Interrupt-Driven Design
- Timer_A interrupts trigger periodic ADC sampling without polling
- ADC14 interrupts handle analog conversion efficiently
- Low-power mode keeps the CPU asleep between interrupts
- No busy-waiting anywhere in the code

### PWM Color Control
- Two Timer_A modules generate PWM for all three RGB channels
- Values range from 0-100 for easy percentage-based adjustment
- LCD circle previews the exact mixed color
- Physical RGB LED displays your creation

### Color Sequence Feature
- Save up to 10 colors to a sequence
- Playback mode cycles through saved colors automatically
- Both the LCD and RGB LED show the sequence

## System Architecture

```
┌────────────────────────────────────────────────────────────┐
│                      MSP432P401R                            │
│                                                             │
│  ┌─────────────┐     ┌─────────────┐     ┌─────────────┐  │
│  │  Timer_A    │────►│   ADC14     │────►│    ISR      │  │
│  │  Trigger    │     │  Conversion │     │   Handler   │  │
│  └─────────────┘     └─────────────┘     └──────┬──────┘  │
│                                                  │         │
│                                          ┌──────▼──────┐  │
│  ┌─────────────┐                         │    Main     │  │
│  │   LPM0      │◄────── Wake ────────────│    Loop     │  │
│  │   Sleep     │                         └──────┬──────┘  │
│  └─────────────┘                                │         │
│                                                  │         │
│         ┌────────────────┬──────────────────────┤         │
│         ▼                ▼                      ▼         │
│  ┌─────────────┐  ┌─────────────┐      ┌─────────────┐   │
│  │  Timer_A0   │  │  Timer_A2   │      │    SPI      │   │
│  │  PWM (R,G)  │  │  PWM (B)    │      │    LCD      │   │
│  └──────┬──────┘  └──────┬──────┘      └─────────────┘   │
│         │                │                                │
└─────────┼────────────────┼────────────────────────────────┘
          ▼                ▼
    ┌─────────────────────────┐
    │       RGB LED           │
    │    (Physical Output)    │
    └─────────────────────────┘
```

### Interrupt Flow

```
Timer_A Overflow
       │
       ▼
  Trigger ADC14
       │
       ▼
  ADC Complete IRQ
       │
       ▼
  Update RGB Value
       │
       ▼
  Wake from LPM0
       │
       ▼
  Update Display + PWM
       │
       ▼
  Return to LPM0
```

## Technologies Used

| Component | Purpose |
|-----------|---------|
| **ADC14** | 14-bit potentiometer reading (P4.7) |
| **Timer_A0** | PWM generation for Red and Green channels |
| **Timer_A2** | PWM generation for Blue channel |
| **SPI** | LCD display communication |
| **GPIO** | Joystick buttons, status LEDs |
| **LPM0** | Low-power sleep mode between updates |

## Code Organization

```
Project 3/
├── project3_main.c     # Entry point, interrupt handlers, main loop
├── Application.c       # UI logic, color mixing, sequence management
├── Application.h       # Structures and declarations
└── HAL/
    ├── HAL.c/h         # Hardware abstraction layer
    ├── Graphics.c/h    # LCD drawing and color preview
    ├── Timer.c/h       # Timer configuration and PWM setup
    └── LED.c/h         # RGB LED PWM control
```

### Implementation Highlights

**Interrupt-Driven ADC Sampling**:
```c
void ADC14_IRQHandler(void) {
    // Read conversion result
    adcResult = ADC14_getResult(ADC_MEM0);

    // Clear interrupt flag
    ADC14_clearInterruptFlag(ADC14_INT0);

    // Signal main loop
    adcReady = true;
}

void TA0_0_IRQHandler(void) {
    // Trigger next ADC conversion
    ADC14_toggleConversionTrigger();
    Timer_A_clearCaptureCompareInterrupt(TIMER_A0_BASE,
                                          TIMER_A_CAPTURECOMPARE_REGISTER_0);
}
```

**Low-Power Mode Integration**:
```c
void sleepMode(void) {
    TurnOn_LLG();           // Visual indicator: entering sleep
    PCM_gotoLPM0();         // Enter low-power mode
    TurnOff_LLG();          // Indicator off: woke up
}

// Main loop
while (1) {
    if (!adcReady) {
        sleepMode();        // Sleep until interrupt
    }
    processNewReading();    // Handle ADC result
    updateDisplay();        // Refresh UI
    adcReady = false;
}
```

**PWM Configuration for RGB**:
```c
// Timer_A0 for Red (CCR1) and Green (CCR2)
Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfigRed);
Timer_A_generatePWM(TIMER_A0_BASE, &pwmConfigGreen);

// Timer_A2 for Blue (CCR1)
Timer_A_generatePWM(TIMER_A2_BASE, &pwmConfigBlue);

// Update duty cycle (0-100 maps to PWM period)
void setRGBColor(int r, int g, int b) {
    Timer_A_setCompareValue(TIMER_A0_BASE, CCR1, r * PWM_PERIOD / 100);
    Timer_A_setCompareValue(TIMER_A0_BASE, CCR2, g * PWM_PERIOD / 100);
    Timer_A_setCompareValue(TIMER_A2_BASE, CCR1, b * PWM_PERIOD / 100);
}
```

## Technical Challenges

### Coordinating Multiple Interrupts
Timer and ADC interrupts need to work together without race conditions or missed samples. I set up Timer_A overflow to trigger ADC conversion, then the ADC completion interrupt signals the main loop. Keeping responsibilities separate prevents conflicts.

### Smooth PWM Color Transitions
Direct value changes can cause visible stepping or flickering in the LED. Using a high PWM frequency (above the visible flicker threshold) and updating all three channels at the same time keeps transitions smooth.

### Power Efficiency
Continuous polling wastes power and generates heat, which matters for battery-powered applications. The interrupt-driven architecture with LPM0 sleep means the CPU only wakes up when there's actual work to do, cutting average current consumption significantly.

### ADC Noise Filtering
Raw potentiometer readings jump around due to electrical noise. I added software averaging over multiple samples and hysteresis to stop values from oscillating at the boundaries.

## Demo

*Screenshots and demo video coming soon*

## Controls

| Input | Action |
|-------|--------|
| Potentiometer | Adjust selected RGB channel (0-100) |
| Joystick Up/Down | Select R, G, or B channel |
| Joystick Left/Right | Fine-tune value |
| Button 1 | Save color to sequence |
| Button 2 | Play/Stop sequence playback |

## Building and Running

1. Open project in Code Composer Studio
2. Connect MSP432 LaunchPad with BoosterPack MKII
3. Make sure potentiometer is connected to P4.7
4. Build and flash to device
5. Use potentiometer to adjust color values
6. Watch real-time preview on LCD and RGB LED
7. Save colors and play back sequences!

## Power Consumption

This project shows embedded low-power design in action:

| Mode | Description |
|------|-------------|
| **Active** | Processing ADC, updating display |
| **LPM0** | CPU sleeping, peripherals active |
| **Wake Sources** | Timer_A overflow, ADC completion |

The design keeps the CPU in low-power mode most of the time, only waking briefly to handle new readings.
