#include <Utils.h>

#include <Clock.h>
#include <Flash.h>
#include <GPIO.h>
#include <Tick.h>
#include <USART.h>

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  Flash::setLatency(8);
  Clock::configureAPB1Prescaler(Clock::APBPrescaler::DIV_4);
  Clock::configurePLL(8, 128);
  Clock::switchSysclk(Clock::SysclkSource::PLL);

  DEBUG_INIT();

  Tick::enable();

  while (true) {
    Tick::delay(1000);
    DEBUG_PRINT("Tick value: %d\r\n", Tick::value);
  }
}
