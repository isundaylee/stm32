#pragma once

#include <stdint.h>

typedef uint32_t size_t;

#define FIELD_GET(reg, field) ((reg & field) >> (field##_Pos))
#define FIELD_SET(reg, field, value)                                           \
  do {                                                                         \
    reg &= ~(field);                                                           \
    reg |= (value) << (field##_Pos);                                           \
  } while (0)

#define BIT_SET(reg, bit) (reg |= bit)
#define BIT_CLEAR(reg, bit) (reg &= ~bit)
#define BIT_IS_SET(reg, field) ((reg & field) != 0)

#define WAIT_UNTIL(condition)                                                  \
  do {                                                                         \
    asm volatile("nop");                                                       \
  } while (!(condition))

#define FORCE_READ(reg) ((void) reg)

#if 0

const char *HexString(uint32_t n);
const char *DecString(uint32_t n);

#endif
