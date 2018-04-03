#include "GPIO.h"

extern "C" void main() {
  GPIO_A.initialize();

  GPIO_A.setMode(4, GPIO_MODE_OUTPUT);

  while (true) {
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
    GPIO_A.set(4);
    GPIO_A.clear(4);
  }
}
