#pragma once

#include <DeviceHeader.h>

const uint32_t GPIO_MODE_INPUT = 0b00;
const uint32_t GPIO_MODE_OUTPUT = 0b01;
const uint32_t GPIO_MODE_ALTERNATE = 0b10;
const uint32_t GPIO_MODE_ANALOG = 0b11;

class GPIO {
private:
  bool initialized_ = false;
  GPIO_TypeDef *gpio_;

public:
  GPIO(GPIO_TypeDef *gpio) { gpio_ = gpio; }

  GPIO(GPIO &&move) = delete;
  GPIO(GPIO const &copy) = delete;
  GPIO &operator=(GPIO &&move) = delete;
  GPIO &operator=(GPIO const &copy) = delete;

  void initialize();

  void setMode(int pin, uint32_t mode, uint32_t alternate = 0);

  void set(int pin) { gpio_->BSRR = (1UL << pin); }
  void clear(int pin) { gpio_->BSRR = (1UL << (pin + 16)); }
  void toggle(int pin) { gpio_->ODR ^= (1UL << pin); }
  bool get(int pin) { return (gpio_->IDR & (1UL << pin)) != 0; }
};

extern GPIO GPIO_A;
extern GPIO GPIO_B;
extern GPIO GPIO_C;
