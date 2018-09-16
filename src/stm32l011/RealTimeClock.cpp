#include "RealTimeClock.h"

/* static */ void (*RealTimeClock::wakeupTimerHandler)(void) = 0;

extern "C" void isrRTC() {
  RealTimeClock::wakeupTimerHandler();

  RTC->ISR &= ~RTC_ISR_WUTF;
  EXTI->PR |= (1UL << 20);
  NVIC_ClearPendingIRQ(RTC_IRQn);
}

/* static */ void RealTimeClock::enable(RTCClock clock) {
  RCC->APB1ENR |= RCC_APB1ENR_PWREN;
  PWR->CR |= PWR_CR_DBP;

  switch (clock) {
  case RTCClock::LSI:
    RCC->CSR |= (0b10 << RCC_CSR_RTCSEL_Pos);
    break;
  }

  RCC->CSR |= RCC_CSR_RTCEN;
}

/* static */ void RealTimeClock::disableWriteProtection() {
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;
}

/* static */ void RealTimeClock::enableWriteProtection() {
  RTC->WPR = 0xFE;
  RTC->WPR = 0x64;
}

/* static */ void RealTimeClock::setupWakeupTimer(size_t count, WakeupTimerClock clock, void (*handler)(void)) {
  wakeupTimerHandler = handler;

  disableWriteProtection();

  RTC->CR &= ~RTC_CR_WUTE;
  WAIT_UNTIL((RTC->ISR & RTC_ISR_WUTWF) != 0);

  RTC->WUTR = (count - 1);

  switch (clock) {
  case WakeupTimerClock::CK_SPRE:
    RTC->CR = RTC_CR_WUTE | RTC_CR_WUTIE | RTC_CR_WUCKSEL_2;
    break;
  }

  RTC->ISR &= ~RTC_ISR_WUTF;
  WAIT_UNTIL((RTC->ISR & RTC_ISR_WUTF) == 0);

  enableWriteProtection();

  EXTI->IMR |= EXTI_IMR_IM20;
  EXTI->EMR |= ~EXTI_EMR_EM20;
  EXTI->RTSR |= EXTI_RTSR_RT20;

  NVIC_ClearPendingIRQ(RTC_IRQn);
  NVIC_EnableIRQ(RTC_IRQn);
}
