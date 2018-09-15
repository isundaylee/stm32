#include "Clock.h"

/* static */ void Clock::enableHSI() {
  RCC->CR |= RCC_CR_HSION;
  WAIT_UNTIL((RCC->CR & RCC_CR_HSIRDY) != 0);
}

/* static */ void Clock::switchSysclk(Sysclk sysclk) {
  switch (sysclk) {
  case Sysclk::HSI:
    RCC->CFGR = (RCC->CFGR & ~RCC_CFGR_SW_Msk) | RCC_CFGR_SW_HSI;
    break;
  }
}
