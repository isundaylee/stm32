#include "GPIO.h"

extern "C" void main() {
  GPIO_B.initialize();

  GPIO_B.setMode(12, GPIO_MODE_OUTPUT);

  while (true) {
    GPIO_B.toggle(12);
  }
}
