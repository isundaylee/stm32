#include <Utils.h>

#include <DMA.h>
#include <GPIO.h>
#include <USART.h>

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  GPIO_A.enable();

  GPIO_A.setMode(9, GPIO::PinMode::ALTERNATE, 7);
  GPIO_A.setMode(10, GPIO::PinMode::ALTERNATE, 7);
  USART_1.enable(115200);
  USART_1.enableTxDMA();

  char hello[] = "Hello, USART via DMA (over and over)!\r\n";

  DMA_2.enable();
  DMA_2.configureStream(
      7, 4, DMA_DIR_MEM_TO_PERI, sizeof(hello) / sizeof(hello[0]) - 1,
      DMA_FIFO_THRES_DIRECT, true, DMA_PRIORITY_VERY_HIGH, hello,
      DMA_SIZE_8_BIT, true, &USART1->DR, DMA_SIZE_8_BIT, false);
  DMA_2.enableStream(7);

  while (true) {
  }
}
