#include <Clock.h>

/* static */ void Clock::switchSysclk(uint32_t systemClock) {
  FIELD_SET(RCC->CFGR, RCC_CFGR_SW, systemClock);
  WAIT_UNTIL(FIELD_GET(RCC->CFGR, RCC_CFGR_SWS) == systemClock);
}

/* static */ void Clock::configurePLL(uint32_t pllM, uint32_t pllN) {
  bool wasPLL = false;

  if (FIELD_GET(RCC->CFGR, RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) {
    wasPLL = true;
    Clock::switchSysclk(CLOCK_SYSCLK_HSI);
  }

  BIT_CLEAR(RCC->CR, RCC_CR_PLLON);

  FIELD_SET(RCC->PLLCFGR, RCC_PLLCFGR_PLLM, pllM);
  FIELD_SET(RCC->PLLCFGR, RCC_PLLCFGR_PLLN, pllN);

  BIT_SET(RCC->CR, RCC_CR_PLLON);
  WAIT_UNTIL(BIT_IS_SET(RCC->CR, RCC_CR_PLLRDY));

  if (wasPLL) {
    Clock::switchSysclk(CLOCK_SYSCLK_PLL);
  }
}

/* static */ void Clock::configureMCO1(uint32_t source, uint32_t prescalar) {
  FIELD_SET(RCC->CFGR, RCC_CFGR_MCO1, source);
  FIELD_SET(RCC->CFGR, RCC_CFGR_MCO1PRE, 0);

  if (prescalar != 1) {
    FIELD_SET(RCC->CFGR, RCC_CFGR_MCO1PRE, 4 + (prescalar - 2));
  }
}

/* static */ void Clock::configureAHBPrescaler(uint32_t prescaler) {
  FIELD_SET(RCC->CFGR, RCC_CFGR_HPRE, prescaler);
}

/* static */ void Clock::configureAPB1Prescaler(uint32_t prescaler) {
  FIELD_SET(RCC->CFGR, RCC_CFGR_PPRE1, prescaler);
}

/* static */ void Clock::configureAPB2Prescaler(uint32_t prescaler) {
  FIELD_SET(RCC->CFGR, RCC_CFGR_PPRE2, prescaler);
}
