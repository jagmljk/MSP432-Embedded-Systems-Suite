/*
 * Timer.h - Software timer based on hardware timer
 */

#ifndef HAL_TIMER_H_
#define HAL_TIMER_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define MS_DIVISION_FACTOR 1000
#define US_DIVISION_FACTOR 1000000
#define SYSTEM_CLOCK 48000000
#define LOADVALUE 0xFFFFFFFF
#define PRESCALER 1

// Software timer struct
struct _SWTimer {
    uint64_t cyclesToWait;
    uint32_t startCounter;
    uint32_t startRollovers;
};
typedef struct _SWTimer SWTimer;

// Create a timer with given wait time in ms
SWTimer SWTimer_construct(uint64_t waitTime_ms);

// Start the timer
void SWTimer_start(SWTimer* timer);

// Get cycles elapsed since timer started
uint64_t SWTimer_elapsedCycles(SWTimer* timer);

// Check if timer has expired
bool SWTimer_expired(SWTimer* timer);

// Set up system clock and hardware timer
void InitSystemTiming();

#endif /* HAL_TIMER_H_ */
