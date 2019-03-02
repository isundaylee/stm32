#include <Clock.h>
#include <Timer.h>

#define DEFINE_TIMER_ISR(n)                                                    \
  extern "C" void isrTimer##n() {                                              \
    (*Timer_##n.handler_)(Timer_##n.handlerContext_);                          \
    TIM##n->SR = ~TIM_SR_UIF;                                                  \
    /* TODO: Fix other cases like this. */                                     \
    FORCE_READ(TIM##n->SR);                                                    \
    NVIC_ClearPendingIRQ(TIM##n##_IRQn);                                       \
  }

DEFINE_TIMER_ISR(2);
DEFINE_TIMER_ISR(3);
DEFINE_TIMER_ISR(4);
DEFINE_TIMER_ISR(5);

uint32_t Timer::rccBit() {
  if (timer_ == TIM2) {
    return RCC_APB1ENR_TIM2EN;
  } else if (timer_ == TIM3) {
    return RCC_APB1ENR_TIM3EN;
  } else if (timer_ == TIM4) {
    return RCC_APB1ENR_TIM4EN;
  } else if (timer_ == TIM5) {
    return RCC_APB1ENR_TIM5EN;
  }

  return 0;
}

IRQn_Type Timer::irqN() {
  if (timer_ == TIM2) {
    NVIC_EnableIRQ(TIM2_IRQn);
  } else if (timer_ == TIM3) {
    NVIC_EnableIRQ(TIM3_IRQn);
  } else if (timer_ == TIM4) {
    NVIC_EnableIRQ(TIM4_IRQn);
  } else if (timer_ == TIM5) {
    NVIC_EnableIRQ(TIM5_IRQn);
  }

  return TIM2_IRQn;
}

uint32_t Timer::getPeripheralFrequency() {
  if (timer_ == TIM2 || timer_ == TIM3 || timer_ == TIM4 || timer_ == TIM5) {
    return Clock::getAPB1TimerFrequency();
  }

  return 0;
}

void Timer::enable(uint32_t prescaler, uint32_t overflow, Action action,
                   EventHandler handler, void* handlerContext) {
  handler_ = handler;
  handlerContext_ = handlerContext;

  BIT_SET(RCC->APB1ENR, rccBit());

  // Disable update interrupt while we're setting things up
  BIT_CLEAR(timer_->DIER, TIM_DIER_UIE);
  FORCE_READ(timer_->DIER);

  // Setting the prescaler, and applying the setting
  timer_->PSC = prescaler - 1;
  BIT_SET(timer_->EGR, TIM_EGR_UG);
  WAIT_UNTIL(BIT_IS_SET(timer_->SR, TIM_SR_UIF));
  timer_->SR = 0;

  // Settings the counts
  timer_->ARR = overflow;
  timer_->CNT = 0;

  switch (action) {
  case Action::ONE_SHOT: {
    BIT_SET(timer_->CR1, TIM_CR1_OPM);
    break;
  }

  case Action::PERIODIC: {
    BIT_CLEAR(timer_->CR1, TIM_CR1_OPM);
    break;
  }
  }

  // Re-enable interrupt
  BIT_SET(timer_->DIER, TIM_DIER_UIE);
  NVIC_EnableIRQ(irqN());

  BIT_SET(timer_->CR1, TIM_CR1_ARPE);
  BIT_SET(timer_->CR1, TIM_CR1_CEN);
}

void Timer::disable() {
  NVIC_DisableIRQ(irqN());
  BIT_CLEAR(timer_->CR1, TIM_CR1_CEN);
  BIT_CLEAR(RCC->APB1ENR, rccBit());
}

void Timer::setOverflow(uint32_t overflow) { timer_->ARR = overflow; }

Timer Timer_2(TIM2);
Timer Timer_3(TIM3);
Timer Timer_4(TIM4);
Timer Timer_5(TIM5);
