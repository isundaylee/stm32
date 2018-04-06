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

const uint32_t CLOCK_AHB_PRESCALER_1 = 0b0000;
const uint32_t CLOCK_AHB_PRESCALER_2 = 0b1000;
const uint32_t CLOCK_AHB_PRESCALER_4 = 0b1001;
const uint32_t CLOCK_AHB_PRESCALER_8 = 0b1010;
const uint32_t CLOCK_AHB_PRESCALER_16 = 0b1011;
const uint32_t CLOCK_AHB_PRESCALER_64 = 0b1100;
const uint32_t CLOCK_AHB_PRESCALER_128 = 0b1101;
const uint32_t CLOCK_AHB_PRESCALER_256 = 0b1110;
const uint32_t CLOCK_AHB_PRESCALER_512 = 0b1111;

const uint32_t CLOCK_APB_PRESCALER_1 = 0b000;
const uint32_t CLOCK_APB_PRESCALER_2 = 0b100;
const uint32_t CLOCK_APB_PRESCALER_4 = 0b101;
const uint32_t CLOCK_APB_PRESCALER_8 = 0b110;
const uint32_t CLOCK_APB_PRESCALER_16 = 0b111;

class Clock {
public:
  uint32_t systemClockFrequency = 16000000;

  static void switchSysclk(uint32_t systemClock);
  static void configurePLL(uint32_t pllM, uint32_t pllN);
  static void configureMCO1(uint32_t source, uint32_t prescalar);
  static void configureAHBPrescaler(uint32_t prescaler);
  static void configureAPB1Prescaler(uint32_t prescaler);
  static void configureAPB2Prescaler(uint32_t prescaler);
};
