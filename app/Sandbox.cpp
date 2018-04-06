#include "ADC.h"
#include "Clock.h"
#include "DMA.h"
#include "Flash.h"
#include "GPIO.h"
#include "USART.h"
#include "Timer.h"

void configureClock() {
  // Setting up clock-out pin
  Clock::configureMCO1(CLOCK_MCO_SOURCE_PLL, 5);
  GPIO_A.enable();
  GPIO_A.setMode(8, GPIO_MODE_ALTERNATE, 0);

  // Setting up PLL frequency
  Flash::setLatency(6);
  Clock::configurePLL(8, 90);
  Clock::switchSysclk(CLOCK_SYSCLK_PLL);
  Clock::configureAPB1Prescaler(CLOCK_APB_PRESCALER_2);
}

void togglePB12() {
  GPIO_B.toggle(12);
  for (int i = 0; i < 10; i++)
    asm volatile("nop");
  GPIO_B.toggle(12);
}

extern "C" void main() {
  configureClock();

  GPIO_B.enable();
  GPIO_B.setMode(6, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(7, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(12, GPIO_MODE_OUTPUT);

  Timer_5.enable(1, 1000, &togglePB12);

  WAIT_UNTIL(false);
}
