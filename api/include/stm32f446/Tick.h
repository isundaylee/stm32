#pragma once

#include <DeviceHeader.h>

class Tick {
public:
  static volatile size_t value;

  static void enable();
};
