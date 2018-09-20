#include "Timer.h"

extern "C" void isrTIM2() {
  Timer_2.updateHandler();
  BIT_CLEAR(TIM2->SR, TIM_SR_UIF);
}

void Timer::enable() {
  if (timer_ == TIM2) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
  }
}

void Timer::startPeriodic(uint32_t prescaler, uint32_t period,
                          InterruptHandler handler) {
  timer_->PSC = prescaler - 1;
  timer_->ARR = period - 1;

  BIT_SET(timer_->DIER, TIM_DIER_UIE);
  BIT_SET(timer_->CR1, TIM_CR1_CEN);

  updateHandler = handler;

  if (timer_ == TIM2) {
    NVIC_EnableIRQ(TIM2_IRQn);
  }
}

void Timer::stop() {
  BIT_CLEAR(timer_->CR1, TIM_CR1_CEN);
  NVIC_DisableIRQ(TIM2_IRQn);
}

Timer Timer_2(TIM2);
