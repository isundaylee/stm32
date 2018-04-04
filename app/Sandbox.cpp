#include "Clock.h"
#include "Flash.h"
#include "GPIO.h"
#include "USART.h"

extern "C" void main() {
  // Setting up clock-out pin
  Clock::configureMCO1(CLOCK_MCO_SOURCE_PLL, 5);
  GPIO_A.enable();
  GPIO_A.setMode(8, GPIO_MODE_ALTERNATE, 0);

  // Setting up PLL frequency
  // Flash::setLatency(6);
  // Clock::configurePLL(8, 50);
  // Clock::switchSysclk(CLOCK_SYSCLK_PLL);

  GPIO_B.enable();
  GPIO_B.setMode(6, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(7, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(12, GPIO_MODE_OUTPUT);

  USART_1.enable();

  while (true) {
    int data = USART_1.read();

    if (data != -1) {
      USART_1.write(data);
    }
  }
}
