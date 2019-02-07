#pragma once

#include <stdint.h>

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

#if 1

const char* HexString(uint32_t n);
const char* DecString(uint32_t n);

#endif
