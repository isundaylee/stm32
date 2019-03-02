#pragma once

#include <DeviceHeader.h>

extern "C" void isrTimer2();
extern "C" void isrTimer3();
extern "C" void isrTimer4();
extern "C" void isrTimer5();

class Timer {
private:
  using EventHandler = void (*)(void*);

  TIM_TypeDef* timer_;

  EventHandler handler_;
  void* handlerContext_;

  uint32_t rccBit();
  IRQn_Type irqN();

public:
  enum class Action {
    PERIODIC,
    ONE_SHOT,
  };

  Timer(TIM_TypeDef* timer) : timer_(timer) {}

  uint32_t getPeripheralFrequency();

  void enable(uint32_t prescaler, uint32_t overflow, Action action,
              EventHandler handler, void* handlerContext);
  void disable();

  void setOverflow(uint32_t overflow);

  friend void isrTimer2();
  friend void isrTimer3();
  friend void isrTimer4();
  friend void isrTimer5();
};

extern Timer Timer_2;
extern Timer Timer_3;
extern Timer Timer_4;
extern Timer Timer_5;
