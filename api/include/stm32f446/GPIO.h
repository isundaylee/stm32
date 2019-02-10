#pragma once

#include <DeviceHeader.h>

extern "C" void isrEXTI0();
extern "C" void isrEXTI1();
extern "C" void isrEXTI2();
extern "C" void isrEXTI3();
extern "C" void isrEXTI4();
extern "C" void isrEXTI9_5();
extern "C" void isrEXTI15_10();

class GPIO {
private:
  GPIO_TypeDef* gpio_;

  InterruptHandler interruptHandlers_[16] = {0};

  static void handleInterrupt(size_t pinLow, size_t pinHigh);

public:
  enum class PinMode {
    INPUT = 0b00,
    OUTPUT = 0b01,
    ALTERNATE = 0b10,
    ANALOG = 0b11,
  };

  enum class TriggerDirection {
    RISING_EDGE,
    FALLING_EDGE,
    BOTH,
  };

  GPIO(GPIO_TypeDef* gpio) { gpio_ = gpio; }

  GPIO(GPIO&& move) = delete;
  GPIO(GPIO const& copy) = delete;
  GPIO& operator=(GPIO&& move) = delete;
  GPIO& operator=(GPIO const& copy) = delete;

  void enable();

  void setMode(int pin, PinMode mode, uint32_t alternate = 0);
  void enableExternalInterrupt(int pin, TriggerDirection direction,
                               InterruptHandler handler);

  void set(int pin) { gpio_->BSRR = (1UL << pin); }
  void clear(int pin) { gpio_->BSRR = (1UL << (pin + 16)); }
  void toggle(int pin) { gpio_->ODR ^= (1UL << pin); }
  bool get(int pin) { return (gpio_->IDR & (1UL << pin)) != 0; }

  friend void isrEXTI0();
  friend void isrEXTI1();
  friend void isrEXTI2();
  friend void isrEXTI3();
  friend void isrEXTI4();
  friend void isrEXTI9_5();
  friend void isrEXTI15_10();
};

extern GPIO GPIO_A;
extern GPIO GPIO_B;
extern GPIO GPIO_C;
extern GPIO GPIO_D;
extern GPIO GPIO_E;
extern GPIO GPIO_F;
extern GPIO GPIO_G;
extern GPIO GPIO_H;
