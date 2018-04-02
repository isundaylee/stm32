#include <stdint.h>

extern uint32_t __attribute__((may_alias)) __bss_start__;
extern uint32_t __attribute__((may_alias)) __bss_end__;
extern uint8_t __attribute__((may_alias)) __data_start__;
extern uint8_t __attribute__((may_alias)) __data_start_load__;
extern uint8_t __attribute__((may_alias)) __data_end__;

extern "C" void main(void);

extern "C" void startup(void) {
  // Zero the BSS section
  for (uint32_t *p = &__bss_start__; p != &__bss_end__; p++) {
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

  main();
}
