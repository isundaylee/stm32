#include "ADC.h"
#include "Clock.h"
#include "DMA.h"
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
  // Clock::configurePLL(8, 150);
  // Clock::switchSysclk(CLOCK_SYSCLK_PLL);

  GPIO_B.enable();
  GPIO_B.setMode(6, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(7, GPIO_MODE_ALTERNATE, 7);
  GPIO_B.setMode(12, GPIO_MODE_OUTPUT);

  GPIO_C.enable();
  GPIO_C.setMode(0, GPIO_MODE_ANALOG);

  char hello[] = "Hello, world!\n";

  USART_1.enable();
  USART_1.enableTxDMA();

  DMA_2.enable();
  DMA_2.configureStream(7, 4, DMA_DIR_MEM_TO_PERI,
                        sizeof(hello) / sizeof(hello[0]) - 1, DMA_FIFO_THRES_DIRECT,
                        true, DMA_PRIORITY_VERY_HIGH, hello, DMA_SIZE_8_BIT,
                        true, &USART1->DR, DMA_SIZE_8_BIT, false);
  DMA_2.enableStream(7);
}
