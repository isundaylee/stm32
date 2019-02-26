#include "Tick.h"
#include "Clock.h"
#include "GPIO.h"

/* static */ size_t volatile Tick::value = 0;

// TODO: This REALLY doesn't belong here.
volatile unsigned int pico_ms_tick;

extern "C" void isrSysTick() {
  Tick::value++;
  pico_ms_tick++;
}

/* static */ void Tick::enable() {
  // TODO: Get the value dynamically.
  SysTick_Config(Clock::sysclkFrequency / 1000);
}
