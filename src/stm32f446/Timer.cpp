#include <Timer.h>

#include "GPIO.h"

#define DEFINE_TIMER_ISR(n)                                                    \
  extern "C" void isrTimer##n() {                                              \
    (*Timer_##n.handler_)();                                                   \
                                                                               \
    TIM##n->SR = ~TIM_SR_UIF;                                                  \
    NVIC_ClearPendingIRQ(TIM##n##_IRQn);                                       \
  }

DEFINE_TIMER_ISR(2);
DEFINE_TIMER_ISR(3);
DEFINE_TIMER_ISR(4);
DEFINE_TIMER_ISR(5);

void Timer::enable(uint32_t prescaler, uint32_t overflow, void (*handler)()) {
  handler_ = handler;

  if (timer_ == TIM2) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_TIM2EN);
  } else if (timer_ == TIM3) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_TIM3EN);
  } else if (timer_ == TIM4) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_TIM4EN);
  } else if (timer_ == TIM5) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_TIM5EN);
  }

  BIT_SET(timer_->DIER, TIM_DIER_UIE);

  timer_->ARR = overflow;
  timer_->PSC = prescaler - 1;

  if (timer_ == TIM2) {
    NVIC_EnableIRQ(TIM2_IRQn);
  } else if (timer_ == TIM3) {
    NVIC_EnableIRQ(TIM3_IRQn);
  } else if (timer_ == TIM4) {
    NVIC_EnableIRQ(TIM4_IRQn);
  } else if (timer_ == TIM5) {
    NVIC_EnableIRQ(TIM5_IRQn);
  }

  BIT_SET(timer_->CR1, TIM_CR1_CEN);
}

Timer Timer_2(TIM2);
Timer Timer_3(TIM3);
Timer Timer_4(TIM4);
Timer Timer_5(TIM5);
