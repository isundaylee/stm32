#pragma once

#include <DeviceHeader.h>

const uint32_t CLOCK_SYSCLK_HSI = RCC_CFGR_SW_HSI;
const uint32_t CLOCK_SYSCLK_HSE = RCC_CFGR_SW_HSE;
const uint32_t CLOCK_SYSCLK_PLL = RCC_CFGR_SW_PLL;
const uint32_t CLOCK_SYSCLK_PLLR = RCC_CFGR_SW_PLLR;

const uint32_t CLOCK_MCO_SOURCE_HSI = 0b00;
const uint32_t CLOCK_MCO_SOURCE_LSE = 0b01;
const uint32_t CLOCK_MCO_SOURCE_HSE = 0b10;
const uint32_t CLOCK_MCO_SOURCE_PLL = 0b11;

class Clock {
public:
  uint32_t systemClockFrequency = 16000000;

  static void switchSysclk(uint32_t systemClock);
  static void configurePLL(uint32_t pllM, uint32_t pllN);
  static void configureMCO1(uint32_t source, uint32_t prescalar);
};
