/*
 * Timer.c
 *
 * Hardware timer setup and software timer implementation.
 * The hardware timer runs continuously and counts rollovers for long durations.
 */

#include <HAL/LED.h>
#include <HAL/Timer.h>

// Tracks how many times the hardware timer has rolled over
static volatile uint64_t hwTimerRollovers;

// Called automatically when timer overflows - don't call this directly
void T32_INT1_IRQHandler()
{
  hwTimerRollovers++;
  Timer32_clearInterruptFlag(TIMER32_0_BASE);
}

// Set up the 48MHz system clock and start the reference timer
void InitSystemTiming()
{
  Interrupt_disableMaster();

  // Flash needs 2 wait states at 48MHz to avoid read errors
  FlashCtl_setWaitState(FLASH_BANK0, 2);
  FlashCtl_setWaitState(FLASH_BANK1, 2);

  CS_setDCOFrequency(SYSTEM_CLOCK);

  // Route all clocks from DCO
  CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

  // Configure Timer32 as a free-running counter for software timers
  Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                     TIMER32_PERIODIC_MODE);
  Timer32_setCount(TIMER32_0_BASE, LOADVALUE);

  Timer32_clearInterruptFlag(TIMER32_0_BASE);
  Interrupt_enableInterrupt(INT_T32_INT1);

  Interrupt_enableMaster();

  Timer32_startTimer(TIMER32_0_BASE, false);
  hwTimerRollovers = 0;
}

// Create a timer that will expire after waitTime_ms milliseconds
SWTimer SWTimer_construct(uint64_t waitTime_ms)
{
  SWTimer timer;

  timer.startCounter = 0;
  timer.startRollovers = 0;

  uint64_t counterClock = SYSTEM_CLOCK / PRESCALER;
  uint64_t cyclesPerMillisecond = counterClock / MS_DIVISION_FACTOR;
  timer.cyclesToWait = cyclesPerMillisecond * waitTime_ms;

  return timer;
}

// Record the current time as the timer's start point
void SWTimer_start(SWTimer *timer_p)
{
  timer_p->startCounter = Timer32_getValue(TIMER32_0_BASE);
  timer_p->startRollovers = hwTimerRollovers;
}

// Calculate how many clock cycles have passed since the timer started
uint64_t SWTimer_elapsedCycles(SWTimer *timer_p)
{
  uint64_t rollovers = hwTimerRollovers - timer_p->startRollovers;
  uint64_t startCounter = timer_p->startCounter;
  uint64_t currentCounter = Timer32_getValue(TIMER32_0_BASE);
  uint64_t elapsedCycles = ((rollovers * ((uint64_t)LOADVALUE + (uint64_t)1)) + startCounter) - currentCounter;

  return elapsedCycles;
}

// Returns true if enough time has passed
bool SWTimer_expired(SWTimer *timer_p)
{
  uint64_t elapsedCycles = SWTimer_elapsedCycles(timer_p);
  return elapsedCycles >= timer_p->cyclesToWait;
}
