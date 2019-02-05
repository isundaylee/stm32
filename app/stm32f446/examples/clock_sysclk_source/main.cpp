#include <Utils.h>

#include <Clock.h>
#include <GPIO.h>

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  GPIO_C.enable();
  GPIO_C.setMode(9, GPIO::PinMode::ALTERNATE, 0);

  Clock::configurePLL(16, 50);
  Clock::switchSysclk(Clock::SysclkSource::PLL);

  Clock::configureMCO2(Clock::MCO2Source::SYSCLK, 5);

  while (true) {
  }
}
