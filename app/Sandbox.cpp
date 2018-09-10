#include "GPIO.h"

static const int DELAY = 100000;

void delay(int iterations) {
  for (int i=0; i<iterations; i++) {
    asm volatile("nop");
  }
}

extern "C" void main() {
  GPIO_A.enable();

  GPIO_A.setMode(4, GPIO_MODE_OUTPUT);

  while (true) {
    GPIO_A.set(4);
    delay(DELAY);
    GPIO_A.clear(4);
    delay(DELAY);
  }
}
