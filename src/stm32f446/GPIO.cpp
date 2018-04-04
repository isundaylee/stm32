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

  size_t unitOffset =
      reinterpret_cast<uintptr_t>(GPIOB) - reinterpret_cast<uintptr_t>(GPIOA);
  size_t offset =
      reinterpret_cast<uintptr_t>(gpio_) - reinterpret_cast<uintptr_t>(GPIOA);
  int index = offset / unitOffset;

  BIT_SET(RCC->AHB1ENR, RCC_AHB1ENR_GPIOAEN << index);
}

void GPIO::setMode(int pin, uint32_t mode, uint32_t alternate /* = 0 */) {
  gpio_->MODER &= ~(0b11U << (2 * pin));
  gpio_->MODER |= (mode << (2 * pin));
  gpio_->AFR[pin / 8] &= ~(0b1111 << (4 * (pin % 8)));
  gpio_->AFR[pin / 8] |= (alternate << (4 * (pin % 8)));
}
