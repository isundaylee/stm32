#include <DeviceHeader.h>

#include <stdint.h>

extern "C" void _startup(void);

extern "C" void _reset() { _startup(); }

extern "C" void _hardFault() {
  while (true) {
    asm volatile("nop");
  }
}

extern "C" void _spin() {
  while (true) {
    asm volatile("nop");
  }
}

extern "C" void _spin2() {
  while (true) {
    asm volatile("nop");
  }
}

extern "C" void _spin3() {
  while (true) {
    asm volatile("nop");
  }
}

extern "C" void _spin4() {
  while (true) {
    asm volatile("nop");
  }
}

#include "Vectors.inl"

extern uint32_t __attribute__((may_alias)) __bss_start__;
extern uint32_t __attribute__((may_alias)) __bss_end__;
extern uint8_t __attribute__((may_alias)) __data_start__;
extern uint8_t __attribute__((may_alias)) __data_start_load__;
extern uint8_t __attribute__((may_alias)) __data_end__;

extern "C" void main(void);

extern "C" void _startup(void) {
  // Zero the BSS section
  for (uint32_t* p = &__bss_start__; p != &__bss_end__; p++) {
    *p = 0;
  }

  // Copy the DATA section
  for (uint8_t *dst = &__data_start__, *src = &__data_start_load__;
       dst != &__data_end__; dst++, src++) {
    *dst = *src;
  }

  // Calling global initializers
  extern void (*__init_array_start__)();
  extern void (*__init_array_end__)();

  for (void (**p)() = &__init_array_start__; p < &__init_array_end__; p++) {
    (*p)();
  }

  // Enable FPU
  SCB->CPACR |= (3UL << 20) | (3UL << 22);
  __DSB();
  __ISB();

  main();
}
