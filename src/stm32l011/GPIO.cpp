#include <GPIO.h>

GPIO GPIO_A(GPIOA);
GPIO GPIO_B(GPIOB);
GPIO GPIO_C(GPIOC);

void GPIO::initialize() {
  if (initialized_) {
    return;
  }

  initialized_ = true;

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

void GPIO::set(int pin) { gpio_->BSRR = (1UL << pin); }

void GPIO::clear(int pin) { gpio_->BSRR = (1UL << (pin + 16)); }

void GPIO::toggle(int pin) { gpio_->ODR ^= (1UL << pin); }

bool GPIO::get(int pin) { return (gpio_->IDR & (1UL << pin)) != 0; }
