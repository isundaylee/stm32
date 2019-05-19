#pragma once

#include <DeviceHeader.h>

#include "UtilsL011.h"

class Timer {
private:
  TIM_TypeDef* timer_;

public:
  InterruptHandler updateHandler = 0;

  Timer(TIM_TypeDef* timer) : timer_(timer) {}

  void enable();

  void startPeriodic(uint32_t prescaler, uint32_t period,
                     InterruptHandler handler);

  void stop();
};

extern Timer Timer_2;
