#include <Clock.h>

/* static */ void Clock::switchSysclk(uint32_t systemClock) {
  RCC->CFGR =
      (RCC->CFGR & (~RCC_CFGR_SWS_Msk)) | (systemClock << RCC_CFGR_SW_Pos);

  while (FIELD_GET(RCC->CFGR, RCC_CFGR_SWS) != systemClock)
    asm volatile("nop");
}

/* static */ void Clock::configurePLL(uint32_t pllM, uint32_t pllN) {
  bool wasPLL = false;

  if (FIELD_GET(RCC->CFGR, RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL) {
    wasPLL = true;
    Clock::switchSysclk(CLOCK_SYSCLK_HSI);
  }

  RCC->CR &= ~RCC_CR_PLLON;
  while (FIELD_IS_SET(RCC->CR, RCC_CR_PLLRDY))
    asm volatile("nop");

  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLM_Msk;
  RCC->PLLCFGR |= (pllM << RCC_PLLCFGR_PLLM_Pos);

  RCC->PLLCFGR &= ~RCC_PLLCFGR_PLLN_Msk;
  RCC->PLLCFGR |= (pllN << RCC_PLLCFGR_PLLN_Pos);

  RCC->CR |= RCC_CR_PLLON;
  while (!FIELD_IS_SET(RCC->CR, RCC_CR_PLLRDY))
    asm volatile("nop");

  if (wasPLL) {
    Clock::switchSysclk(CLOCK_SYSCLK_PLL);
  }
}

/* static */ void Clock::configureMCO1(uint32_t source, uint32_t prescalar) {
  RCC->CFGR &= ~RCC_CFGR_MCO1_Msk;
  RCC->CFGR |= (source << RCC_CFGR_MCO1_Pos);
  RCC->CFGR &= ~RCC_CFGR_MCO1PRE_Msk;

  if (prescalar != 1) {
    RCC->CFGR |= ((4 + (prescalar - 2)) << RCC_CFGR_MCO1PRE_Pos);
  }
}
