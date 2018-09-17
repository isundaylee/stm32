#pragma once

#include <DeviceHeader.h>

class GPIO {
private:
  GPIO_TypeDef* gpio_;

public:
  enum class PinMode {
    INPUT,
    OUTPUT,
    ALTERNATE,
    ANALOG,
  };

  enum class OutputMode {
    PUSH_PULL,
    OPEN_DRAIN,
  };

  enum class PullDirection {
    NONE,
    PULL_UP,
    PULL_DOWN,
  };

  enum class TriggerDirection { RISING_EDGE, FALLING_EDGE };

  InterruptHandler interruptHandlers[16] = {0};

  GPIO(GPIO_TypeDef* gpio) { gpio_ = gpio; }

  GPIO(GPIO&& move) = delete;
  GPIO(GPIO const& copy) = delete;
  GPIO& operator=(GPIO&& move) = delete;
  GPIO& operator=(GPIO const& copy) = delete;

  void enable();

  void setMode(int pin, PinMode mode, uint32_t alternate = 0);
  void setOutputMode(int pin, OutputMode mode);
  void setPullDirection(int pin, PullDirection direction);
  void enableExternalInterrupt(int pin, TriggerDirection direction,
                               InterruptHandler handler);

  void set(int pin) { gpio_->BSRR = (1UL << pin); }
  void clear(int pin) { gpio_->BSRR = (1UL << (pin + 16)); }
  void toggle(int pin) { gpio_->ODR ^= (1UL << pin); }
  bool get(int pin) { return (gpio_->IDR & (1UL << pin)) != 0; }
};

extern GPIO GPIO_A;
extern GPIO GPIO_B;
extern GPIO GPIO_C;
