#pragma once

#include <DeviceHeader.h>

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
private:
  static void updateSysclkFrequency();

public:
  enum class SysclkSource {
    HSI = RCC_CFGR_SW_HSI,
    HSE = RCC_CFGR_SW_HSE,
    PLL = RCC_CFGR_SW_PLL,
    PLLR = RCC_CFGR_SW_PLLR,
  };

  enum class MCO1Source {
    HSI = 0b00,
    LSE = 0b01,
    HSE = 0b10,
    PLL = 0b11,
  };

  enum class MCO2Source {
    SYSCLK = 0b00,
    PLLI2S = 0b01,
    HSE = 0b10,
    PLL = 0b11,
  };

  static uint32_t sysclkFrequency;

  static void switchSysclk(SysclkSource systemClock);

  static void configurePLL(uint32_t pllM, uint32_t pllN);
  static void configureMCO1(MCO1Source source, uint32_t prescalar);
  static void configureMCO2(MCO2Source source, uint32_t prescalar);
  static void configureAHBPrescaler(uint32_t prescaler);
  static void configureAPB1Prescaler(uint32_t prescaler);
  static void configureAPB2Prescaler(uint32_t prescaler);

  static uint32_t getPLLCLKFrequency();
  static uint32_t getAHBFrequency();
  static uint32_t getAPB1Frequency();
  static uint32_t getAPB1TimerFrequency();
  static uint32_t getAPB2Frequency();
  static uint32_t getAPB2TimerFrequency();
};
