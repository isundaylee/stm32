#include "GPIO.h"

extern "C" void main() {
  GPIO_A.initialize();

  GPIO_A.setMode(4, GPIO_MODE_OUTPUT);

  while (true) {
    for (int i = 0; i < 100000; i++)
      asm volatile("nop");
    GPIO_A.toggle(4);
    for (int i = 0; i < 100000; i++)
      asm volatile("nop");
    GPIO_A.toggle(4);
  }
}
