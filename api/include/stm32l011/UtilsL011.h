#pragma once

#include <stddef.h>
#include <stdint.h>

extern "C" {
void* memset(void* ptr, int value, size_t num);
void* memcpy(void* __restrict dst, const void* __restrict src, size_t num);
void __aeabi_memset(void* ptr, size_t num, int value);
void __aeabi_memset4(void* ptr, size_t num, int value);
}

typedef void (*InterruptHandler)(void);

#define FIELD_GET(reg, field) ((reg & field) >> (field##_Pos))
#define FIELD_SET(reg, field, value)                                           \
  do {                                                                         \
    reg &= ~(field);                                                           \
    reg |= (value) << (field##_Pos);                                           \
  } while (0)

#define BIT_SET(reg, bit) (reg |= (bit))
#define BIT_CLEAR(reg, bit) (reg &= ~(bit))
#define BIT_IS_SET(reg, field) ((reg & (field)) != 0)
#define BIT_IS_CLEAR(reg, field) ((reg & (field)) == 0)

#define WAIT_UNTIL(condition)                                                  \
  do {                                                                         \
    asm volatile("nop");                                                       \
  } while (!(condition))

#define DELAY(cycles)                                                          \
  do {                                                                         \
    int counter = cycles;                                                      \
    WAIT_UNTIL((--counter) == 0);                                              \
  } while (false)

#define FORCE_READ(reg) ((void)reg)

#define E2I(e) (static_cast<uint32_t>(e))