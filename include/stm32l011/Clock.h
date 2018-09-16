#pragma once

#include <DeviceHeader.h>

class Clock {
public:
  enum class Sysclk { HSI };

  Clock(Clock&&) = delete;
  Clock(Clock const&) = delete;
  Clock& operator=(Clock&&) = delete;
  Clock& operator=(Clock const&) = delete;

  static void enableHSI();
  static void enableLSI();
  static void switchSysclk(Sysclk sysclk);
};
