#include "Tick.h"
#include "GPIO.h"

/* static */ size_t Tick::value = 0;

extern "C" void isrSysTick() { Tick::value++; }

/* static */ void Tick::enable() {
  // TODO: Get the value dynamically.
  SysTick_Config(16000);
}
