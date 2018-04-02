#include "stm32l0xx.h"

extern "C" void main() {
  RCC->IOPENR |= RCC_IOPENR_IOPAEN;

  GPIOA->MODER &= ~(0b11 << 8);
  GPIOA->MODER |= (0b01 << 8);

  while (true) {
    GPIOA->BSRR |= (0b1 << 4);
    for (int i=0; i<100000; i++) asm volatile ("nop");
    GPIOA->BSRR |= (0b1 << 20);
    for (int i=0; i<100000; i++) asm volatile ("nop");
  }
}
