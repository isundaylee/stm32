#pragma once

#include <DeviceHeader.h>

extern "C" void isrTimer2();
extern "C" void isrTimer3();
extern "C" void isrTimer4();
extern "C" void isrTimer5();

class Timer {
private:
  TIM_TypeDef *timer_;

  void (*handler_)();

public:
  Timer(TIM_TypeDef *timer) : timer_(timer) {}

  void enable(uint32_t prescaler, uint32_t overflow, void (*handler)());

  friend void isrTimer2();
  friend void isrTimer3();
  friend void isrTimer4();
  friend void isrTimer5();
};

extern Timer Timer_2;
extern Timer Timer_3;
extern Timer Timer_4;
extern Timer Timer_5;
