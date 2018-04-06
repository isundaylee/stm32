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
  Flash::setLatency(8);
  Clock::configurePLL(8, 160);
  Clock::switchSysclk(CLOCK_SYSCLK_PLL);
  Clock::configureAPB1Prescaler(CLOCK_APB_PRESCALER_4);
  Clock::configureAPB2Prescaler(CLOCK_APB_PRESCALER_2);

  GPIO_B.enable();
  GPIO_B.setMode(6, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(7, GPIO_MODE_ALTERNATE, 7);

  USART_1.enable(230400);
  USART_1.write("================================\n");

  USART_1.write("PLL frequency: ");
  USART_1.write(DecString(Clock::getPLLCLKFrequency() / 1000000));
  USART_1.write(" MHz\n");

  USART_1.write("SYSCLK frequency: ");
  USART_1.write(DecString(Clock::sysclkFrequency / 1000000));
  USART_1.write(" MHz\n");

  USART_1.write("AHB frequency: ");
  USART_1.write(DecString(Clock::getAHBFrequency() / 1000000));
  USART_1.write(" MHz\n");

  USART_1.write("APB1 frequency: ");
  USART_1.write(DecString(Clock::getAPB1Frequency() / 1000000));
  USART_1.write(" MHz\n");

  USART_1.write("APB1 Timer frequency: ");
  USART_1.write(DecString(Clock::getAPB1TimerFrequency() / 1000000));
  USART_1.write(" MHz\n");

  USART_1.write("APB2 frequency: ");
  USART_1.write(DecString(Clock::getAPB2Frequency() / 1000000));
  USART_1.write(" MHz\n");

  USART_1.write("APB2 Timer frequency: ");
  USART_1.write(DecString(Clock::getAPB2TimerFrequency() / 1000000));
  USART_1.write(" MHz\n");
}

void generateWave() {
  static uint16_t timestep = 0;
  static uint16_t pattern[16] = {2048, 2832, 3496, 3940, 4095, 3940,
                                 3496, 2832, 2048, 1264, 600,  156,
                                 0,    156,  600,  1264};

  GPIO_B.set(12);
  DAC_1.setChannelValue(1, pattern[(timestep++) % 16]);
  GPIO_B.clear(12);
}

uint16_t freqs[8] = {0, 262, 294, 330, 349, 392, 440, 494};

extern "C" void main() {
  configureClock();

  GPIO_B.enable();
  GPIO_B.setMode(12, GPIO_MODE_OUTPUT);

  GPIO_A.enable();
  GPIO_A.setMode(4, GPIO_MODE_ANALOG);
  GPIO_A.setMode(5, GPIO_MODE_ANALOG);

  DAC_1.enable();
  DAC_1.enableChannel(1);

  Timer_5.enable(1, Timer_5.getPeripheralFrequency() / 16 / freqs[1],
                 &generateWave);

  while (true) {
    int data = USART_1.read();

    if (data >= '1' && data <= '7') {
      Timer_5.setOverflow(Timer_5.getPeripheralFrequency() / 16 /
                          freqs[data - '1' + 1]);
    }
  }

  WAIT_UNTIL(false);
}
