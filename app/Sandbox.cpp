#include "Utils.h"

#include "GPIO.h"
#include "Timer.h"

void timerHandler() {
  static bool value = 0;

  value = !value;

  if (value) {
    GPIO_A.set(1);
  } else {
    GPIO_A.clear(1);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  GPIO_A.enable();
  GPIO_A.setMode(1, GPIO_MODE_OUTPUT);

  Timer_2.enable(1000, 8000, timerHandler);

  while (true) {
  }
}
