#pragma once

#include <DeviceHeader.h>

class GPIO {
private:
  GPIO_TypeDef* gpio_;

public:
  enum class PinMode {
    INPUT = 0b00,
    OUTPUT = 0b01,
    ALTERNATE = 0b10,
    ANALOG = 0b11,
  };

  GPIO(GPIO_TypeDef* gpio) { gpio_ = gpio; }

  GPIO(GPIO&& move) = delete;
  GPIO(GPIO const& copy) = delete;
  GPIO& operator=(GPIO&& move) = delete;
  GPIO& operator=(GPIO const& copy) = delete;

  void enable();

  void setMode(int pin, PinMode mode, uint32_t alternate = 0);

  void set(int pin) { gpio_->BSRR = (1UL << pin); }
  void clear(int pin) { gpio_->BSRR = (1UL << (pin + 16)); }
  void toggle(int pin) { gpio_->ODR ^= (1UL << pin); }
  bool get(int pin) { return (gpio_->IDR & (1UL << pin)) != 0; }
};

extern GPIO GPIO_A;
extern GPIO GPIO_B;
extern GPIO GPIO_C;
extern GPIO GPIO_D;
extern GPIO GPIO_E;
extern GPIO GPIO_F;
extern GPIO GPIO_G;
extern GPIO GPIO_H;