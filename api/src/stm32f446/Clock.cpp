#include <Clock.h>

/* static */ uint32_t Clock::sysclkFrequency = 16000000;

/* static */ void Clock::switchSysclk(SysclkSource systemClock) {
  FIELD_SET(RCC->CFGR, RCC_CFGR_SW, E2I(systemClock));
  WAIT_UNTIL(FIELD_GET(RCC->CFGR, RCC_CFGR_SWS) == E2I(systemClock));

  updateSysclkFrequency();
}

/* static */ void Clock::updateSysclkFrequency() {
  switch (FIELD_GET(RCC->CFGR, RCC_CFGR_SW)) {
  case E2I(SysclkSource::HSI):
    sysclkFrequency = 16000000;
    break;
  case E2I(SysclkSource::HSE):
    // TODO
    break;
  case E2I(SysclkSource::PLL):
    sysclkFrequency = getPLLCLKFrequency();
    break;
  case E2I(SysclkSource::PLLR):
    // TODO
    break;
  }
}

/* static */ void Clock::configurePLL(uint32_t pllM, uint32_t pllN) {
  bool wasPLL = false;

  if (FIELD_GET(RCC->CFGR, RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) {
    wasPLL = true;
    Clock::switchSysclk(SysclkSource::HSI);
  }

  BIT_CLEAR(RCC->CR, RCC_CR_PLLON);

  FIELD_SET(RCC->PLLCFGR, RCC_PLLCFGR_PLLM, pllM);
  FIELD_SET(RCC->PLLCFGR, RCC_PLLCFGR_PLLN, pllN);

  BIT_SET(RCC->CR, RCC_CR_PLLON);
  WAIT_UNTIL(BIT_IS_SET(RCC->CR, RCC_CR_PLLRDY));

  if (wasPLL) {
    Clock::switchSysclk(SysclkSource::PLL);
  }
}

/* static */ void Clock::configureMCO1(MCO1Source source, uint32_t prescalar) {
  FIELD_SET(RCC->CFGR, RCC_CFGR_MCO1, E2I(source));
  FIELD_SET(RCC->CFGR, RCC_CFGR_MCO1PRE, 0);

  if (prescalar != 1) {
    FIELD_SET(RCC->CFGR, RCC_CFGR_MCO1PRE, 4 + (prescalar - 2));
  }
}

/* static */ void Clock::configureMCO2(MCO2Source source, uint32_t prescalar) {
  FIELD_SET(RCC->CFGR, RCC_CFGR_MCO2, E2I(source));
  FIELD_SET(RCC->CFGR, RCC_CFGR_MCO2PRE, 0);

  if (prescalar != 1) {
    FIELD_SET(RCC->CFGR, RCC_CFGR_MCO2PRE, 4 + (prescalar - 2));
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

/* static */ uint32_t Clock::getPLLCLKFrequency() {
  uint32_t baseFrequency = 16000000;

  // TODO: Supports PLLSRC here

  uint32_t pllM = FIELD_GET(RCC->PLLCFGR, RCC_PLLCFGR_PLLM);
  uint32_t pllN = FIELD_GET(RCC->PLLCFGR, RCC_PLLCFGR_PLLN);
  uint32_t pllP = 2 * (FIELD_GET(RCC->PLLCFGR, RCC_PLLCFGR_PLLP) + 1);

  baseFrequency /= pllM;
  baseFrequency *= pllN;
  baseFrequency /= pllP;

  return baseFrequency;
}

/* static */ uint32_t Clock::getAHBFrequency() {
  uint32_t ahbPrescaler = FIELD_GET(RCC->CFGR, RCC_CFGR_HPRE);

  if (ahbPrescaler == CLOCK_AHB_PRESCALER_1) {
    return sysclkFrequency;
  } else {
    return (sysclkFrequency >> (ahbPrescaler - CLOCK_AHB_PRESCALER_2 + 1));
  }
}

/* static */ uint32_t Clock::getAPB1Frequency() {
  uint32_t apb1Prescaler = FIELD_GET(RCC->CFGR, RCC_CFGR_PPRE1);

  if (apb1Prescaler == CLOCK_APB_PRESCALER_1) {
    return getAHBFrequency();
  } else {
    return (getAHBFrequency() >> (apb1Prescaler - CLOCK_APB_PRESCALER_2 + 1));
  }
}

/* static */ uint32_t Clock::getAPB1TimerFrequency() {
  uint32_t apb1Prescaler = FIELD_GET(RCC->CFGR, RCC_CFGR_PPRE1);

  if (apb1Prescaler == CLOCK_APB_PRESCALER_1) {
    return getAPB1Frequency();
  } else {
    return getAPB1Frequency() * 2;
  }
}

/* static */ uint32_t Clock::getAPB2Frequency() {
  uint32_t apb2Prescaler = FIELD_GET(RCC->CFGR, RCC_CFGR_PPRE2);

  if (apb2Prescaler == CLOCK_APB_PRESCALER_1) {
    return getAHBFrequency();
  } else {
    return (getAHBFrequency() >> (apb2Prescaler - CLOCK_APB_PRESCALER_2 + 1));
  }
}

/* static */ uint32_t Clock::getAPB2TimerFrequency() {
  uint32_t apb2Prescaler = FIELD_GET(RCC->CFGR, RCC_CFGR_PPRE2);

  if (apb2Prescaler == CLOCK_APB_PRESCALER_1) {
    return getAPB2Frequency();
  } else {
    return getAPB2Frequency() * 2;
  }
}
