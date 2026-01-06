# Color Mixer

An interactive RGB color mixing system developed to demonstrate **interrupt-driven architecture**, **PWM control**, **ADC input processing**, and **low-power design** on the MSP432P401R microcontroller.

## Overview

Create custom colors by adjusting RGB values with a potentiometer and joystick. See your color displayed in real-time on both the LCD and a physical RGB LED using PWM. Features a sequence playback system to save and display your favorite color combinations.

## Key Features

### Interrupt-Driven Design
- **Timer_A interrupts**: Periodic ADC sampling without polling
- **ADC14 interrupts**: Efficient analog conversion handling
- **Low-power mode**: CPU sleeps between interrupts, waking only when needed
- **Zero busy-wait**: All operations triggered by hardware events

### PWM Color Control
- **Dual Timer_A PWM**: Independent control of RGB channels
- **0-100 value range**: User-friendly percentage-based adjustment
- **Real-time preview**: LCD circle shows exact mixed color
- **Physical LED output**: RGB LED displays created color

### Color Sequence Feature
- **Save up to 10 colors**: Store favorite color combinations
- **Playback mode**: Cycle through saved sequence automatically
- **Visual feedback**: Both LCD and RGB LED show sequence

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

## Technologies & Peripherals

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

### Key Implementation Highlights

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

## Technical Challenges & Solutions

### Challenge: Coordinating Multiple Interrupts
**Problem**: Timer and ADC interrupts must work together without race conditions or missed samples.
**Solution**: Used Timer_A overflow to trigger ADC conversion, and ADC completion interrupt to signal main loop. Clear separation of responsibilities prevents conflicts.

### Challenge: Smooth PWM Color Transitions
**Problem**: Direct value changes cause visible stepping/flickering in LED output.
**Solution**: Used high PWM frequency (above visible flicker threshold) and ensured atomic updates to all three channels simultaneously.

### Challenge: Power Efficiency
**Problem**: Continuous polling wastes power and generates heat in battery-powered applications.
**Solution**: Implemented true interrupt-driven architecture with LPM0 sleep. CPU only wakes for actual work, reducing average current consumption significantly.

### Challenge: ADC Noise Filtering
**Problem**: Raw potentiometer readings fluctuate due to electrical noise.
**Solution**: Applied software averaging over multiple samples and added hysteresis to prevent value oscillation at boundaries.

## Demo

*Screenshots and demo video coming soon*

<!--
TODO: Add media
![Color Mixer UI](./media/mixer_ui.png)
![RGB LED Output](./media/rgb_led.jpg)
![Sequence Playback](./media/sequence.gif)
-->

## Controls

| Input | Action |
|-------|--------|
| Potentiometer | Adjust selected RGB channel (0-100) |
| Joystick Up/Down | Select R, G, or B channel |
| Joystick Left/Right | Fine-tune value |
| Button 1 | Save color to sequence |
| Button 2 | Play/Stop sequence playback |

## Building & Running

1. Open project in Code Composer Studio
2. Connect MSP432 LaunchPad with BoosterPack MKII
3. Ensure potentiometer is connected to P4.7
4. Build and flash to device
5. Use potentiometer to adjust color values
6. Watch real-time preview on LCD and RGB LED
7. Save colors and play back sequences!

## Power Consumption

This project demonstrates embedded low-power design principles:

| Mode | Description |
|------|-------------|
| **Active** | Processing ADC, updating display |
| **LPM0** | CPU sleeping, peripherals active |
| **Wake Sources** | Timer_A overflow, ADC completion |

The architecture ensures the CPU spends the majority of time in low-power mode, waking only briefly to process new readings.
