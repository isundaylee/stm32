#include "ADC.h"
#include "Clock.h"
#include "DMA.h"
#include "Flash.h"
#include "GPIO.h"
#include "USART.h"

void test() {
  USART_1.write("Hello, world!\n");

  static uint16_t data = 0;

  DMA_2.enable();

  BIT_SET(DMA2_Stream0->CR, DMA_SxCR_EN);

  USART_1.write("S0NDTR = ");
  USART_1.write(HexString(DMA2_Stream0->NDTR));
  USART_1.write("\n");

  DMA_2.enable();
  DMA_2.configureStream(0, 0, DMA_DIR_PERI_TO_MEM, 0xFFFF, true,
                        DMA_PRIORITY_VERY_HIGH, &ADC1->DR, DMA_SIZE_16_BIT,
                        false, &data, DMA_SIZE_16_BIT, false);
  DMA_2.enableStream(0);

  ADC_1.selectChannel(10);
  ADC_1.enableDMA();
  ADC_1.startContinuousConversion();

  while (true) {
    USART_1.write("DATA = ");
    USART_1.write(HexString(data));
    USART_1.write("\n");

    int command = USART_1.read();
    if (command == '1') {
      ADC_1.startContinuousConversion();
    } else if (command == '0') {
      ADC_1.stopContinuousConversion();
    }
  }
}

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

  USART_1.enable();
  ADC_1.enable();

  test();

  //   ADC_1.selectChannel(10);
  //   ADC_1.startContinuousConversion();
  //   while (true) {
  //     USART_1.write("Conversion result: ");
  //     USART_1.write(HexString(ADC_1.getContinuousConversion()));
  //     USART_1.write("\n");
  //
  //     int data = USART_1.read();
  //     if (data == '1') {
  //       ADC_1.startContinuousConversion();
  //     } else if (data == '0') {
  //       ADC_1.stopContinuousConversion();
  //     }
  //   }
}
