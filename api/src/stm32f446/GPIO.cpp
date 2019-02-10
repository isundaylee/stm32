#include <GPIO.h>

GPIO GPIO_A(GPIOA);
GPIO GPIO_B(GPIOB);
GPIO GPIO_C(GPIOC);
GPIO GPIO_D(GPIOF);
GPIO GPIO_E(GPIOE);
GPIO GPIO_F(GPIOF);
GPIO GPIO_G(GPIOG);
GPIO GPIO_H(GPIOH);

#define DEFINE_EXTI_ISR(name, low, high)                                       \
  extern "C" void isrEXTI##name() { GPIO::handleInterrupt(low, high); }

DEFINE_EXTI_ISR(0, 0, 0);
DEFINE_EXTI_ISR(1, 1, 1);
DEFINE_EXTI_ISR(2, 2, 2);
DEFINE_EXTI_ISR(3, 3, 3);
DEFINE_EXTI_ISR(4, 4, 4);
DEFINE_EXTI_ISR(9_5, 5, 9);
DEFINE_EXTI_ISR(15_10, 10, 15);

/* static */ void GPIO::handleInterrupt(size_t pinLow, size_t pinHigh) {
  for (size_t pin = pinLow; pin <= pinHigh; pin++) {
    if ((EXTI->PR & (1U << pin)) == 0) {
      continue;
    }

    int bitPos = 4 * (pin % 4);

    GPIO* gpio = nullptr;

    switch ((SYSCFG->EXTICR[pin / 4] & (0xFU << bitPos)) >> bitPos) {
    case 0b0000:
      gpio = &GPIO_A;
      break;
    case 0b0001:
      gpio = &GPIO_B;
      break;
    case 0b0010:
      gpio = &GPIO_C;
      break;
    case 0b0011:
      gpio = &GPIO_D;
      break;
    case 0b0100:
      gpio = &GPIO_E;
      break;
    case 0b0101:
      gpio = &GPIO_F;
      break;
    case 0b0110:
      gpio = &GPIO_G;
      break;
    case 0b0111:
      gpio = &GPIO_H;
      break;
    }

    if (gpio->interruptHandlers_[pin] != 0) {
      gpio->interruptHandlers_[pin]();
    }

    EXTI->PR |= (1U << pin);
  }
}

void GPIO::enable() {
  size_t unitOffset =
      reinterpret_cast<uintptr_t>(GPIOB) - reinterpret_cast<uintptr_t>(GPIOA);
  size_t offset =
      reinterpret_cast<uintptr_t>(gpio_) - reinterpret_cast<uintptr_t>(GPIOA);
  int index = offset / unitOffset;

  BIT_SET(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN << index);
}

void GPIO::setMode(int pin, PinMode mode, uint32_t alternate /* = 0 */) {
  gpio_->MODER &= ~(0b11U << (2 * pin));
  gpio_->MODER |= (static_cast<uint32_t>(mode) << (2 * pin));
  gpio_->AFR[pin / 8] &= ~(0b1111 << (4 * (pin % 8)));
  gpio_->AFR[pin / 8] |= (alternate << (4 * (pin % 8)));
}

void GPIO::enableExternalInterrupt(int pin, TriggerDirection direction,
                                   InterruptHandler handler) {
  BIT_SET(RCC->APB2ENR, RCC_APB2ENR_SYSCFGEN);

  interruptHandlers_[pin] = handler;

  size_t portNumber =
      (reinterpret_cast<uintptr_t>(gpio_) -
       reinterpret_cast<uintptr_t>(GPIOA)) /
      (reinterpret_cast<uintptr_t>(GPIOB) - reinterpret_cast<uintptr_t>(GPIOA));

  SYSCFG->EXTICR[pin / 4] &= ~(0xFU << (4 * (pin % 4)));
  SYSCFG->EXTICR[pin / 4] |= (portNumber << (4 * (pin % 4)));

  EXTI->IMR |= (1U << pin);
  EXTI->EMR &= ~(1U << pin);

  switch (direction) {
  case TriggerDirection::RISING_EDGE:
    EXTI->RTSR |= (1U << pin);
    EXTI->FTSR &= ~(1U << pin);
    break;
  case TriggerDirection::FALLING_EDGE:
    EXTI->RTSR &= ~(1U << pin);
    EXTI->FTSR |= (1U << pin);
    break;
  case TriggerDirection::BOTH:
    EXTI->RTSR |= (1U << pin);
    EXTI->FTSR |= (1U << pin);
    break;
  }

  if (pin == 0) {
    NVIC_EnableIRQ(EXTI0_IRQn);
  } else if (pin == 1) {
    NVIC_EnableIRQ(EXTI1_IRQn);
  } else if (pin == 2) {
    NVIC_EnableIRQ(EXTI2_IRQn);
  } else if (pin == 3) {
    NVIC_EnableIRQ(EXTI3_IRQn);
  } else if (pin == 4) {
    NVIC_EnableIRQ(EXTI4_IRQn);
  } else if (pin <= 9) {
    NVIC_EnableIRQ(EXTI9_5_IRQn);
  } else {
    NVIC_EnableIRQ(EXTI15_10_IRQn);
  }
}
