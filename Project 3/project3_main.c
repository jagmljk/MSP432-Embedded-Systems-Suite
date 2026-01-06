#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include <stdbool.h>
#include "HAL/Button.h"
#include "HAL/LED.h"
#include "HAL/Timer.h"
#include "HAL/Graphics.h"
#include <stdio.h>

// Color definitions
#define COLOR_BLACK       0x000000
#define COLOR_WHITE       0xFFFFFF
#define COLOR_RED         0xFF0000
#define COLOR_GREEN       0x00FF00
#define COLOR_BLUE        0x0000FF

// PWM and timer definitions
#define PWM_PERIOD_CYCLES 1250
#define RED_TIMER         TIMER_A0_BASE
#define RED_CHANNEL       TIMER_A_CAPTURECOMPARE_REGISTER_3
#define GRN_TIMER         TIMER_A0_BASE
#define GRN_CHANNEL       TIMER_A_CAPTURECOMPARE_REGISTER_1
#define BLU_TIMER         TIMER_A2_BASE
#define BLU_CHANNEL       TIMER_A_CAPTURECOMPARE_REGISTER_1
#define ADC_MAX_VALUE     10000u

// Game parameters
#define MAX_SEQUENCE_COLORS 10
#define COLOR_PLAY_DELAY_MS 500
#define FLASH_FEEDBACK_MS   200

// Display parameters
#define DUTY_CYCLE_MAX        100
#define PREVIEW_X             64
#define PREVIEW_Y             39
#define PREVIEW_RADIUS        20

#define RGB_CIRCLE_Y          90
#define RGB_CIRCLE_RADIUS     9.5

// Text positions
#define VALUE_TEXT_ROW        12.6f
#define RED_TEXT_COL          3.4f
#define GREEN_TEXT_COL        9.4f
#define BLUE_TEXT_COL         15.4f

#define CURSOR_ARROW_ROW      13.9f
#define RED_ARROW_COL         4.4f
#define GREEN_ARROW_COL       10.4f
#define BLUE_ARROW_COL        16.4f

typedef enum {
    STATE_TITLE,
    STATE_MENU,
    STATE_INSTRUCTIONS,
    STATE_GAME
} AppState;

typedef enum {
    CURSOR_RED = 0,
    CURSOR_GREEN = 1,
    CURSOR_BLUE = 2
} ColorCursor;

typedef struct {
    uint8_t red;
    uint8_t green;
    uint8_t blue;
} RGBColor;

typedef struct {
    RGBColor currentColor;
    ColorCursor colorCursor;
    volatile uint16_t adcValue;
    AppState appState;
    bool screenDrawn;
    GFX gfx;
    SWTimer previewTimer;
    SWTimer titleTimer;
    SWTimer flashFeedbackTimer;
    bool flashFeedbackActive;
    struct {
        RGBColor colors[MAX_SEQUENCE_COLORS];
        uint8_t count;
        bool isPlaying;
        uint8_t playIndex;
        SWTimer playTimer;
    } colorSequence;
} AppContext;

static AppContext* isrCtx = NULL;

// Store context pointer for use in interrupt handlers
void setAppContextPtr(AppContext* ctx) {
    isrCtx = ctx;
}

void initialize(AppContext* ctx);
void sleepMode(void);
void main_loop(AppContext* ctx);
void drawTitleScreen(AppContext* ctx);
void drawMenuScreen(AppContext* ctx);
void drawInstructionsScreen(AppContext* ctx);
void drawGameScreen(AppContext* ctx);
void initADC(void);
void initADCTriggerTimer(void);
void handleGameScreen(AppContext* ctx);
uint32_t scaleToHex(uint8_t value);
void updateRGBPWM(AppContext* ctx);
void initPWMPins(void);
void updatePWM(uint32_t timerBase, uint32_t channel, float dutyCycleFrac);
void initPWM(AppContext* ctx);
void App_Screen_updateRGBCursor(AppContext* ctx);
void updatePreviewCircle(AppContext* ctx);
void saveCurrentColor(AppContext* ctx);
void playColorSequence(AppContext* ctx);
void updateSequencePlayback(AppContext* ctx);
void handleTitleState(AppContext* ctx, buttons_t b);
void handleMenuState(AppContext* ctx, buttons_t b);
void handleInstrState(AppContext* ctx, buttons_t b);
void handleGameState(AppContext* ctx, buttons_t b);

// Process button inputs and handle current application state
void main_loop(AppContext* ctx) {
    buttons_t b = updateButtons();

    if (b.LB1tapped) Toggle_LL1();
    if (ctx->appState != STATE_GAME && b.JSBtapped) {
        Toggle_BLG();
    }
    switch (ctx->appState) {
        case STATE_TITLE:        handleTitleState(ctx, b); break;
        case STATE_MENU:         handleMenuState(ctx, b); break;
        case STATE_INSTRUCTIONS: handleInstrState(ctx, b); break;
        case STATE_GAME:         handleGameState(ctx, b); break;
    }
}

// Program entry point - initialize system and run main loop
int main(void) {
    AppContext ctx = {0};
    ctx.appState = STATE_TITLE;

    setAppContextPtr(&ctx);

    initialize(&ctx);
    ctx.gfx = GFX_construct(FG_COLOR, BG_COLOR);
    drawTitleScreen(&ctx);
    ctx.screenDrawn = true;

    while (1) {
        sleepMode();
        main_loop(&ctx);
    }
}

// Initialize all hardware components and timers
void initialize(AppContext* ctx) {
    WDT_A_hold(WDT_A_BASE);
    InitSystemTiming();
    initLEDs();
    initButtons();
    initADC();
    initADCTriggerTimer();
    Interrupt_enableMaster();
    initPWMPins();
    initPWM(ctx);

    ctx->previewTimer = SWTimer_construct(1000);
    SWTimer_start(&ctx->previewTimer);

    ctx->titleTimer = SWTimer_construct(2000);
    SWTimer_start(&ctx->titleTimer);

    ctx->flashFeedbackTimer = SWTimer_construct(FLASH_FEEDBACK_MS);
    ctx->colorSequence.playTimer = SWTimer_construct(COLOR_PLAY_DELAY_MS);
}

// Enter low power mode between operations
void sleepMode(void) {
    TurnOn_LLG();
    PCM_gotoLPM0();
    TurnOff_LLG();
}

// Draw the title screen
void drawTitleScreen(AppContext* ctx) {
    GFX_clear(&ctx->gfx);
    GFX_print(&ctx->gfx, "Color Mixer", 4, 6);
    GFX_print(&ctx->gfx, "By Jagan Muralitharan", 6, 0);
}

// Draw the main menu screen
void drawMenuScreen(AppContext* ctx) {
    GFX_clear(&ctx->gfx);
    GFX_print(&ctx->gfx, "Main Menu", 4, 5);
    GFX_print(&ctx->gfx, "BB1: Start Game", 8, 1);
    GFX_print(&ctx->gfx, "BB2: Instructions", 10, 1);
}

// Draw the instructions screen
void drawInstructionsScreen(AppContext* ctx) {
    GFX_clear(&ctx->gfx);
    GFX_print(&ctx->gfx, "How To Use", 1, 5);
    GFX_print(&ctx->gfx, "Use the potentiometer", 3, 0);
    GFX_print(&ctx->gfx, "to adjust the LED.", 4, 0);
    GFX_print(&ctx->gfx, "Press BB1 to select", 5, 0);
    GFX_print(&ctx->gfx, "which color to adjust.", 6, 0);
    GFX_print(&ctx->gfx, "Press BB2 to save & ", 7, 0);
    GFX_print(&ctx->gfx, "show current color", 8, 0);
    GFX_print(&ctx->gfx, "mix on the BLED.", 9, 0);
    GFX_print(&ctx->gfx, "Hold JSB to play ", 10, 0);
    GFX_print(&ctx->gfx, "sequence (max 10).", 11, 0);
    GFX_print(&ctx->gfx, "BB2 to return", 13, 5);
}

// Draw the game/RGB mixer screen
void drawGameScreen(AppContext* ctx) {
    GFX_clear(&ctx->gfx);
    GFX_print(&ctx->gfx, "RGB Mixer", 1, 2);
    GFX_setForeground(&ctx->gfx, COLOR_WHITE);
    GFX_drawSolidCircle(&ctx->gfx, PREVIEW_X, PREVIEW_Y, PREVIEW_RADIUS);

    GFX_setForeground(&ctx->gfx, FG_COLOR);
    GFX_print(&ctx->gfx, "Preview", 8, 7);

    GFX_setForeground(&ctx->gfx, COLOR_RED);
    GFX_drawSolidCircle(&ctx->gfx, 28, RGB_CIRCLE_Y, RGB_CIRCLE_RADIUS);
    GFX_setForeground(&ctx->gfx, COLOR_GREEN);
    GFX_drawSolidCircle(&ctx->gfx, 64, RGB_CIRCLE_Y, RGB_CIRCLE_RADIUS);
    GFX_setForeground(&ctx->gfx, COLOR_BLUE);
    GFX_drawSolidCircle(&ctx->gfx, 100, RGB_CIRCLE_Y, RGB_CIRCLE_RADIUS);

    char str[4];
    uint8_t* colorValues[3] = {
        &ctx->currentColor.red,
        &ctx->currentColor.green,
        &ctx->currentColor.blue
    };
    uint32_t colors[3] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE};
    float cols[3] = {RED_TEXT_COL, GREEN_TEXT_COL, BLUE_TEXT_COL};

    int i;
    for (i = 0; i < 3; i++) {
        GFX_setForeground(&ctx->gfx, colors[i]);
        sprintf(str, "%03d", *colorValues[i]);
        GFX_print(&ctx->gfx, str, VALUE_TEXT_ROW, cols[i]);
    }

    char seqStr[20];
    GFX_setForeground(&ctx->gfx, FG_COLOR);
    sprintf(seqStr, "Sequence: %d/10", ctx->colorSequence.count);
    GFX_print(&ctx->gfx, seqStr, 0, 7);

    App_Screen_updateRGBCursor(ctx);
}

// Handle title screen transitions
void handleTitleState(AppContext* ctx, buttons_t b) {
    if (SWTimer_expired(&ctx->titleTimer) || b.BB1tapped || b.BB2tapped) {
        ctx->appState = STATE_MENU;
        ctx->screenDrawn = false;
    }
}

// Handle main menu button inputs
void handleMenuState(AppContext* ctx, buttons_t b) {
    if (!ctx->screenDrawn) {
        drawMenuScreen(ctx);
        ctx->screenDrawn = true;
        return;
    }
    if (b.BB1tapped) {
        ctx->appState = STATE_GAME;
        ctx->screenDrawn = false;
    } else if (b.BB2tapped) {
        ctx->appState = STATE_INSTRUCTIONS;
        ctx->screenDrawn = false;
    }
}

// Handle instructions screen button inputs
void handleInstrState(AppContext* ctx, buttons_t b) {
    if (!ctx->screenDrawn) {
        drawInstructionsScreen(ctx);
        ctx->screenDrawn = true;
        return;
    }
    if (b.BB2tapped) {
        ctx->appState = STATE_MENU;
        ctx->screenDrawn = false;
    }
}

// Handle RGB mixer game inputs and updates
void handleGameState(AppContext* ctx, buttons_t b) {
    if (!ctx->screenDrawn) {
        drawGameScreen(ctx);
        ctx->screenDrawn = true;
        ctx->colorCursor = CURSOR_RED;
    }
    if (b.BB1tapped) {
        ctx->colorCursor = (ctx->colorCursor + 1) % 3;
        App_Screen_updateRGBCursor(ctx);
    }
    if (b.BB2tapped) {
        saveCurrentColor(ctx);
        updateRGBPWM(ctx);
    }
    if (b.JSBtapped) {
        playColorSequence(ctx);
    }
    if (SWTimer_expired(&ctx->previewTimer)) {
        SWTimer_start(&ctx->previewTimer);
        updatePreviewCircle(ctx);
    }
    if (ctx->flashFeedbackActive && SWTimer_expired(&ctx->flashFeedbackTimer)) {
        ctx->flashFeedbackActive = false;
        updatePreviewCircle(ctx);
    }
    updateSequencePlayback(ctx);
    handleGameScreen(ctx);
}

// Initialize ADC for potentiometer reading
void initADC(void) {
    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P4, GPIO_PIN7, GPIO_TERTIARY_MODULE_FUNCTION);
    ADC14_enableModule();
    ADC14_initModule(ADC_CLOCKSOURCE_MCLK, ADC_PREDIVIDER_1, ADC_DIVIDER_1, 0);
    ADC14_configureSingleSampleMode(ADC_MEM0, false);
    ADC14_configureConversionMemory(ADC_MEM0, ADC_VREFPOS_AVCC_VREFNEG_VSS, ADC_INPUT_A6, false);
    ADC14_enableInterrupt(ADC_INT0);
    Interrupt_enableInterrupt(INT_ADC14);
    ADC14_enableConversion();
}

// Set up timer to trigger ADC conversions periodically
void initADCTriggerTimer(void) {
    Timer_A_UpModeConfig upConfig = {
        TIMER_A_CLOCKSOURCE_SMCLK,
        TIMER_A_CLOCKSOURCE_DIVIDER_64,
        46875,
        TIMER_A_TAIE_INTERRUPT_DISABLE,
        TIMER_A_CCIE_CCR0_INTERRUPT_ENABLE,
        TIMER_A_DO_CLEAR
    };
    Timer_A_configureUpMode(TIMER_A1_BASE, &upConfig);
    Timer_A_startCounter(TIMER_A1_BASE, TIMER_A_UP_MODE);
    Interrupt_enableInterrupt(INT_TA1_0);
}

// Timer interrupt that triggers ADC conversion
void TA1_0_IRQHandler(void) {
    Timer_A_clearCaptureCompareInterrupt(TIMER_A1_BASE, TIMER_A_CAPTURECOMPARE_REGISTER_0);
    ADC14_toggleConversionTrigger();
}

// Process ADC values to update RGB values and display
void handleGameScreen(AppContext* ctx) {
    if (!ctx->screenDrawn || ctx->appState != STATE_GAME)
        return;

    uint16_t val = (ctx->adcValue * DUTY_CYCLE_MAX) / ADC_MAX_VALUE;
    if (val > DUTY_CYCLE_MAX) val = DUTY_CYCLE_MAX;

    uint8_t* targetVal;
    switch (ctx->colorCursor) {
        case CURSOR_RED:   targetVal = &ctx->currentColor.red;   break;
        case CURSOR_GREEN: targetVal = &ctx->currentColor.green; break;
        default:           targetVal = &ctx->currentColor.blue;  break;
    }

    if (*targetVal != val) {
        *targetVal = val;

        float cols[3] = {RED_TEXT_COL, GREEN_TEXT_COL, BLUE_TEXT_COL};
        uint32_t clrs[3] = {COLOR_RED, COLOR_GREEN, COLOR_BLUE};

        char str[4];
        GFX_setForeground(&ctx->gfx, COLOR_BLACK);
        GFX_drawSolidRectangle(&ctx->gfx, ctx->colorCursor * 36, 50 + ctx->colorCursor * 36, 116, 124);
        GFX_setForeground(&ctx->gfx, clrs[ctx->colorCursor]);
        sprintf(str, "%03d", *targetVal);
        GFX_print(&ctx->gfx, str, VALUE_TEXT_ROW, cols[ctx->colorCursor]);
    }
}

// ADC conversion complete interrupt handler
void ADC14_IRQHandler(void) {
    if (ADC14_getInterruptStatus() & ADC_INT0) {
        ADC14_clearInterruptFlag(ADC_INT0);
        if (isrCtx != NULL) {
            isrCtx->adcValue = ADC14_getResult(ADC_MEM0);
        }
    }
}

// Convert 0-100 value to 0-255 for RGB color components
uint32_t scaleToHex(uint8_t value) {
    return (uint32_t)((value * 255) / DUTY_CYCLE_MAX);
}

// Set up GPIO pins for PWM output
void initPWMPins(void) {
    const struct {
        uint_fast8_t port;
        uint_fast16_t pin;
        uint_fast8_t function;
    } pwmPins[] = {
        {GPIO_PORT_P5, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION},
        {GPIO_PORT_P2, GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION},
        {GPIO_PORT_P2, GPIO_PIN4, GPIO_PRIMARY_MODULE_FUNCTION}
    };

    int i;
    for (i = 0; i < 3; i++) {
        GPIO_setAsOutputPin(pwmPins[i].port, pwmPins[i].pin);
        GPIO_setOutputLowOnPin(pwmPins[i].port, pwmPins[i].pin);
        GPIO_setAsPeripheralModuleFunctionOutputPin(pwmPins[i].port, pwmPins[i].pin, pwmPins[i].function);
    }
}

// Update all RGB LED PWM signals based on current color
void updateRGBPWM(AppContext* ctx) {
    updatePWM(RED_TIMER, RED_CHANNEL, ctx->currentColor.red / (float)DUTY_CYCLE_MAX);
    updatePWM(GRN_TIMER, GRN_CHANNEL, ctx->currentColor.green / (float)DUTY_CYCLE_MAX);
    updatePWM(BLU_TIMER, BLU_CHANNEL, ctx->currentColor.blue / (float)DUTY_CYCLE_MAX);
}

// Update single PWM channel's duty cycle
void updatePWM(uint32_t timerBase, uint32_t channel, float dutyCycleFrac) {
    if (dutyCycleFrac < 0.0f) dutyCycleFrac = 0.0f;
    if (dutyCycleFrac > 1.0f) dutyCycleFrac = 1.0f;
    uint16_t compareValue = (uint16_t)((1.0f - dutyCycleFrac) * PWM_PERIOD_CYCLES);
    Timer_A_setCompareValue(timerBase, channel, compareValue);
}

// Initialize PWM timers for RGB LED control
void initPWM(AppContext* ctx) {
    struct {
        uint32_t timer;
        uint32_t channel;
        uint8_t* valueRef;
    } pwmConfigs[] = {
        {RED_TIMER, RED_CHANNEL, &ctx->currentColor.red},
        {GRN_TIMER, GRN_CHANNEL, &ctx->currentColor.green},
        {BLU_TIMER, BLU_CHANNEL, &ctx->currentColor.blue}
    };

    int i;
    for (i = 0; i < 3; i++) {
        Timer_A_PWMConfig pwmConfig = {
            .clockSource = TIMER_A_CLOCKSOURCE_SMCLK,
            .clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_24,
            .timerPeriod = PWM_PERIOD_CYCLES,
            .compareRegister = pwmConfigs[i].channel,
            .compareOutputMode = TIMER_A_OUTPUTMODE_SET_RESET,
            .dutyCycle = (uint16_t)((1.0f - *pwmConfigs[i].valueRef / 100.0f) * PWM_PERIOD_CYCLES)
        };
        Timer_A_generatePWM(pwmConfigs[i].timer, &pwmConfig);
    }
}

// Update the cursor indicator to show which color is selected
void App_Screen_updateRGBCursor(AppContext* ctx) {
    float arrowRow = CURSOR_ARROW_ROW;
    float arrowCols[3] = {RED_ARROW_COL, GREEN_ARROW_COL, BLUE_ARROW_COL};

    GFX_setForeground(&ctx->gfx, COLOR_BLACK);

    int i;
    for (i = 0; i < 3; ++i) {
        GFX_print(&ctx->gfx, "^", arrowRow, arrowCols[i]);
    }

    GFX_setForeground(&ctx->gfx, FG_COLOR);
    GFX_print(&ctx->gfx, "^", arrowRow, arrowCols[ctx->colorCursor]);
}

// Update the preview circle with current mixed color
void updatePreviewCircle(AppContext* ctx) {
    uint32_t mixedColor = (scaleToHex(ctx->currentColor.red) << 16) |
                          (scaleToHex(ctx->currentColor.green) << 8) |
                           scaleToHex(ctx->currentColor.blue);
    GFX_setForeground(&ctx->gfx, mixedColor);
    GFX_drawSolidCircle(&ctx->gfx, 64, 39, 20);
    GFX_setForeground(&ctx->gfx, FG_COLOR);
}

// Save current color to the sequence memory
void saveCurrentColor(AppContext* ctx) {
    if (ctx->colorSequence.count >= MAX_SEQUENCE_COLORS)
        return;

    ctx->colorSequence.colors[ctx->colorSequence.count] = ctx->currentColor;
    ctx->colorSequence.count++;

    char seqStr[20];
    GFX_setForeground(&ctx->gfx, FG_COLOR);
    sprintf(seqStr, "Sequence: %d/10", ctx->colorSequence.count);
    GFX_print(&ctx->gfx, seqStr, 0, 7);

    updatePreviewCircle(ctx);
    ctx->flashFeedbackActive = false;
}

// Start playback of saved color sequence
void playColorSequence(AppContext* ctx) {
    if (ctx->colorSequence.count == 0 || ctx->colorSequence.isPlaying)
        return;

    ctx->colorSequence.isPlaying = true;
    ctx->colorSequence.playIndex = 0;

    SWTimer_start(&ctx->colorSequence.playTimer);

    RGBColor* firstColor = &ctx->colorSequence.colors[0];
    updatePWM(RED_TIMER, RED_CHANNEL, firstColor->red / 100.0f);
    updatePWM(GRN_TIMER, GRN_CHANNEL, firstColor->green / 100.0f);
    updatePWM(BLU_TIMER, BLU_CHANNEL, firstColor->blue / 100.0f);
}

// Update sequence playback, moving to next color when timer expires
void updateSequencePlayback(AppContext* ctx) {
    if (!ctx->colorSequence.isPlaying)
        return;

    if (SWTimer_expired(&ctx->colorSequence.playTimer)) {
        ctx->colorSequence.playIndex++;

        if (ctx->colorSequence.playIndex >= ctx->colorSequence.count) {
            ctx->colorSequence.isPlaying = false;
            updateRGBPWM(ctx);
            return;
        }

        RGBColor* nextColor = &ctx->colorSequence.colors[ctx->colorSequence.playIndex];
        updatePWM(RED_TIMER, RED_CHANNEL, nextColor->red / 100.0f);
        updatePWM(GRN_TIMER, GRN_CHANNEL, nextColor->green / 100.0f);
        updatePWM(BLU_TIMER, BLU_CHANNEL, nextColor->blue / 100.0f);

        SWTimer_start(&ctx->colorSequence.playTimer);
    }
}
