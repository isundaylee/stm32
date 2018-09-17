#include <GPIO.h>

GPIO GPIO_A(GPIOA);
GPIO GPIO_B(GPIOB);
GPIO GPIO_C(GPIOC);

static void triggerInterrupt(int pin) {
  if ((EXTI->PR & (1U << pin)) == 0) {
    return;
  }

  int bitPos = 4 * (pin % 4);

  switch ((SYSCFG->EXTICR[pin / 4] & (0xFU << bitPos)) >> bitPos) {
  case 0b0000:
    if (GPIO_A.interruptHandlers[pin] != 0) {
      GPIO_A.interruptHandlers[pin]();
    }
    break;
  case 0b0001:
    if (GPIO_B.interruptHandlers[pin] != 0) {
      GPIO_B.interruptHandlers[pin]();
    }
    break;
  case 0b0010:
    if (GPIO_C.interruptHandlers[pin] != 0) {
      GPIO_C.interruptHandlers[pin]();
    }
    break;
  }

  EXTI->PR |= (1U << pin);
}

extern "C" void isrEXTI01() {
  for (int i = 0; i <= 1; i++) {
    triggerInterrupt(i);
  }
}

extern "C" void isrEXTI23() {
  for (int i = 2; i <= 3; i++) {
    triggerInterrupt(i);
  }
}

extern "C" void isrEXTI415() {
  for (int i = 4; i <= 15; i++) {
    triggerInterrupt(i);
  }
}

void GPIO::enable() {
  if (gpio_ == GPIOA) {
    RCC->IOPENR |= RCC_IOPENR_IOPAEN;
  } else if (gpio_ == GPIOB) {
    RCC->IOPENR |= RCC_IOPENR_IOPBEN;
  } else {
    RCC->IOPENR |= RCC_IOPENR_IOPCEN;
  }
}

void GPIO::setMode(int pin, uint32_t mode, uint32_t alternate /* = 0 */) {
  gpio_->MODER &= ~(0b11U << (2 * pin));
  gpio_->MODER |= (mode << (2 * pin));
  gpio_->AFR[pin / 8] &= ~(0b1111 << (4 * (pin % 8)));
  gpio_->AFR[pin / 8] |= (alternate << (4 * (pin % 8)));
}

void GPIO::setPullDirection(int pin, PullDirection direction) {
  switch (direction) {
  case PullDirection::NONE:
    gpio_->PUPDR &= ~(0b11U << (2 * pin));
    gpio_->PUPDR |= (0b00U << (2 * pin));
    break;
  case PullDirection::PULL_UP:
    gpio_->PUPDR &= ~(0b11U << (2 * pin));
    gpio_->PUPDR |= (0b01U << (2 * pin));
    break;
  case PullDirection::PULL_DOWN:
    gpio_->PUPDR &= ~(0b11U << (2 * pin));
    gpio_->PUPDR |= (0b10U << (2 * pin));
    break;
  }
}

void GPIO::enableExternalInterrupt(int pin, TriggerDirection direction,
                                   InterruptHandler handler) {
  interruptHandlers[pin] = handler;

  if (gpio_ == GPIOA) {
    SYSCFG->EXTICR[pin / 4] &= ~(0xFU << (4 * (pin % 4)));
    SYSCFG->EXTICR[pin / 4] |= (0b0000 << (4 * (pin % 4)));
  } else if (gpio_ == GPIOB) {
    SYSCFG->EXTICR[pin / 4] &= ~(0xFU << (4 * (pin % 4)));
    SYSCFG->EXTICR[pin / 4] |= (0b0001 << (4 * (pin % 4)));
  } else if (gpio_ == GPIOC) {
    SYSCFG->EXTICR[pin / 4] &= ~(0xFU << (4 * (pin % 4)));
    SYSCFG->EXTICR[pin / 4] |= (0b0010 << (4 * (pin % 4)));
  }

  EXTI->IMR |= (1U << pin);
  EXTI->EMR &= ~(1U << pin);

  switch (direction) {
  case TriggerDirection::RISING_EDGE:
    EXTI->RTSR |= (1U << pin);
    break;
  case TriggerDirection::FALLING_EDGE:
    EXTI->FTSR |= (1U << pin);
    break;
  }

  if (pin <= 1) {
    NVIC_EnableIRQ(EXTI0_1_IRQn);
  } else if (pin <= 3) {
    NVIC_EnableIRQ(EXTI2_3_IRQn);
  } else {
    NVIC_EnableIRQ(EXTI4_15_IRQn);
  }
}

void GPIO::setOutputMode(int pin, uint32_t mode) {
  gpio_->OTYPER &= ~(0b1 << pin);
  gpio_->OTYPER |= (mode << pin);
}
