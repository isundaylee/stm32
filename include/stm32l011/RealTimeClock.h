#pragma once

#include <DeviceHeader.h>

class RealTimeClock {
private:
  static void disableWriteProtection();
  static void enableWriteProtection();

public:
  enum class RTCClock {
    LSI,
  };

  enum class WakeupTimerClock {
    CK_SPRE,
  };

  static void enable(RTCClock clock);

  static void setupWakeupTimer(size_t count, WakeupTimerClock clock,
                               void (*handler)(void));

  static void (*wakeupTimerHandler)(void);
};
