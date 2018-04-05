#include "ADC.h"
#include "Clock.h"
#include "DMA.h"
#include "Flash.h"
#include "GPIO.h"
#include "USART.h"

void test() {
  USART_1.write("Hello, world!\n");

  uint32_t from[] = {0x11111111, 0x22222222, 0x33333333, 0x44444444};
  uint32_t to[] = {0x00, 0x00, 0x00, 0x00};

  DMA_2.enable();
  DMA_2.configureStream(0, 0, DMA_DIR_MEM_TO_MEM, 4, DMA_FIFO_THRES_FULL, false,
                        DMA_PRIORITY_VERY_HIGH, from, DMA_SIZE_32_BIT, true,
                        to, DMA_SIZE_32_BIT, true);
  DMA_2.enableStream(0);

  for (size_t i = 0; i < sizeof(to) / sizeof(to[0]); i++) {
    USART_1.write("to[");
    USART_1.write(static_cast<uint8_t>('0' + i));
    USART_1.write("] = ");
    USART_1.write(HexString(to[i]));
    USART_1.write("\n");
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
