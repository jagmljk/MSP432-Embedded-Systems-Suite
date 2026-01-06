// Timer.c - Software timer implementation

#include <HAL/Timer.h>

static volatile uint64_t hwTimerRollovers = 0;

// Timer interrupt handler - counts rollovers
void T32_INT1_IRQHandler() {
  hwTimerRollovers++;
  Timer32_clearInterruptFlag(TIMER32_0_BASE);
}

// Initialize system clock and hardware timer
void InitSystemTiming() {
  Interrupt_disableMaster();

  // Set flash wait states before changing clock (prevents bricking)
  FlashCtl_setWaitState(FLASH_BANK0, 2);
  FlashCtl_setWaitState(FLASH_BANK1, 2);

  CS_setDCOFrequency(SYSTEM_CLOCK);

  CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
  CS_initClockSignal(CS_ACLK, CS_REFOCLK_SELECT, CS_CLOCK_DIVIDER_1);

  Timer32_initModule(TIMER32_0_BASE, TIMER32_PRESCALER_1, TIMER32_32BIT,
                     TIMER32_PERIODIC_MODE);
  Timer32_setCount(TIMER32_0_BASE, LOADVALUE);
  Timer32_startTimer(TIMER32_0_BASE, false);

  Interrupt_enableMaster();
  Interrupt_enableInterrupt(INT_T32_INT1);
}

// Create a timer with given wait time in ms
SWTimer SWTimer_construct(uint64_t waitTime_ms) {
  SWTimer timer;
  timer.startCounter = 0;
  timer.startRollovers = 0;

  uint64_t counterClock = SYSTEM_CLOCK / PRESCALER;
  uint64_t cyclesPerMillisecond = counterClock / MS_DIVISION_FACTOR;
  timer.cyclesToWait = cyclesPerMillisecond * waitTime_ms;

  return timer;
}

// Start the timer
void SWTimer_start(SWTimer* timer_p) {
  timer_p->startCounter = Timer32_getValue(TIMER32_0_BASE);
  timer_p->startRollovers = hwTimerRollovers;
}

// Get elapsed cycles since timer started
uint64_t SWTimer_elapsedCycles(SWTimer* timer_p) {
  uint64_t rollovers = hwTimerRollovers - timer_p->startRollovers;
  uint64_t startCounter = timer_p->startCounter;
  uint64_t currentCounter = Timer32_getValue(TIMER32_0_BASE);
  uint64_t elapsedCycles =
      (rollovers * (LOADVALUE + 1)) + startCounter - currentCounter;

  return elapsedCycles;
}

// Check if timer has expired
bool SWTimer_expired(SWTimer* timer_p) {
  uint64_t elapsedCycles = SWTimer_elapsedCycles(timer_p);
  return elapsedCycles >= timer_p->cyclesToWait;
}

// Get elapsed time in microseconds
uint64_t SWTimer_elapsedTimeUS(SWTimer* timer_p) {
  uint64_t elapsedCycles = SWTimer_elapsedCycles(timer_p);
  uint64_t counterClock = SYSTEM_CLOCK / PRESCALER;
  uint64_t cyclesPerMicrosecond = counterClock / US_DIVISION_FACTOR;
  return elapsedCycles / cyclesPerMicrosecond;
}

// Get progress as percentage (0.0 to 1.0)
double SWTimer_percentElapsed(SWTimer* timer_p) {
  if (timer_p->cyclesToWait == 0) return 1.0;

  uint64_t elapsedCycles = SWTimer_elapsedCycles(timer_p);
  double result = (double)elapsedCycles / (double)timer_p->cyclesToWait;

  if (result > 1.0) return 1.0;
  return result;
}
