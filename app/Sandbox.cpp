#include "ADC.h"
#include "Clock.h"
#include "DAC.h"
#include "DMA.h"
#include "Flash.h"
#include "GPIO.h"
#include "Timer.h"
#include "USART.h"

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

void generateWave() {
  static uint16_t output = 0;
  static int direction = 1;

  GPIO_B.set(12);

  output += direction;
  if (output == 0xFFF || output == 0) {
    direction = -direction;
  }
  DAC_1.setChannelValue(1, output);

  GPIO_B.clear(12);
}

extern "C" void main() {
  configureClock();

  GPIO_B.enable();
  GPIO_B.setMode(6, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(7, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(12, GPIO_MODE_OUTPUT);

  GPIO_A.enable();
  GPIO_A.setMode(4, GPIO_MODE_ANALOG);
  GPIO_A.setMode(5, GPIO_MODE_ANALOG);

  DAC_1.enable();
  DAC_1.enableChannel(1);

  Timer_5.enable(1, 80, &generateWave);

  WAIT_UNTIL(false);
}
