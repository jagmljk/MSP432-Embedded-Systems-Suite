/*
 * Timer.h
 *
 * System timing and software timer implementation for MSP432.
 * Uses Timer32 hardware to track elapsed time with millisecond precision.
 */

#ifndef HAL_TIMER_H_
#define HAL_TIMER_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define MS_DIVISION_FACTOR 1000
#define US_DIVISION_FACTOR 1000000

// System runs at 48MHz
#define SYSTEM_CLOCK 48000000
#define CLOCK_CYCLES_IN_MS (SYSTEM_CLOCK / 1000)

#define LOADVALUE 0xFFFFFFFF
#define PRESCALER 1

/*
 * Software timer struct - tracks time using hardware timer rollovers.
 * Use SWTimer_construct() to create, then SWTimer_start() before checking expiration.
 */
struct _SWTimer {
  uint64_t cyclesToWait;     // How many cycles until timer expires
  uint32_t startCounter;     // Timer value when started
  uint32_t startRollovers;   // Rollover count when started
};
typedef struct _SWTimer SWTimer;

// Create a software timer with given wait time in milliseconds
SWTimer SWTimer_construct(uint64_t waitTime_ms);

// Start (or restart) a timer - call this before checking expiration
void SWTimer_start(SWTimer* timer_p);

// Get cycles elapsed since timer started (internal use mainly)
uint64_t SWTimer_elapsedCycles(SWTimer* timer_p);

// Check if the timer has expired
bool SWTimer_expired(SWTimer* timer_p);

// Initialize system clock to 48MHz and start the hardware timer
void InitSystemTiming();

// One-shot hardware timer functions
void startHWTimer(uint32_t waitTime_ms);
bool HWTimerExpired();

#endif /* HAL_TIMER_H_ */
