// Timer.h - Software timer system

#ifndef HAL_TIMER_H_
#define HAL_TIMER_H_

#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define MS_DIVISION_FACTOR 1000
#define US_DIVISION_FACTOR 1000000
#define SYSTEM_CLOCK 48000000
#define LOADVALUE 0xFFFFFFFF
#define PRESCALER 1

struct _SWTimer {
  uint64_t cyclesToWait;
  uint32_t startCounter;
  uint32_t startRollovers;
};
typedef struct _SWTimer SWTimer;

SWTimer SWTimer_construct(uint64_t waitTime_ms);
void SWTimer_start(SWTimer* timer);
uint64_t SWTimer_elapsedCycles(SWTimer* timer);
bool SWTimer_expired(SWTimer* timer);
void InitSystemTiming();

#endif /* HAL_TIMER_H_ */
