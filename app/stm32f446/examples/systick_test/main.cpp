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

  GPIO_A.enable();
  GPIO_A.setMode(9, GPIO::PinMode::ALTERNATE, 7);  // TX
  GPIO_A.setMode(10, GPIO::PinMode::ALTERNATE, 7); // RX
  USART_1.enable(115200);

  Tick::enable();

  while (true) {
    Tick::delay(1000);
    DEBUG_PRINT("Tick value: %d\r\n", Tick::value);
  }
}
