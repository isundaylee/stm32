#include <GPIO.h>

GPIO GPIO_A(GPIOA);
GPIO GPIO_B(GPIOB);
GPIO GPIO_C(GPIOC);
GPIO GPIO_D(GPIOF);
GPIO GPIO_E(GPIOE);
GPIO GPIO_F(GPIOF);
GPIO GPIO_G(GPIOG);
GPIO GPIO_H(GPIOH);

void GPIO::initialize() {
  if (initialized_) {
    return;
  }

  initialized_ = true;

  if (gpio_ == GPIOA) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  } else if (gpio_ == GPIOB) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOBEN;
  } else if (gpio_ == GPIOC) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  } else if (gpio_ == GPIOD) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
  } else if (gpio_ == GPIOE) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN;
  } else if (gpio_ == GPIOF) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOFEN;
  } else if (gpio_ == GPIOG) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
  } else if (gpio_ == GPIOH) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOHEN;
  }
}

void GPIO::setMode(int pin, uint32_t mode, uint32_t alternate /* = 0 */) {
  gpio_->MODER &= ~(0b11U << (2 * pin));
  gpio_->MODER |= (mode << (2 * pin));
  gpio_->AFR[pin / 8] &= ~(0b1111 << (4 * (pin % 8)));
  gpio_->AFR[pin / 8] |= (alternate << (4 * (pin % 8)));
}
