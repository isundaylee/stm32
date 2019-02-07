#include <Utils.h>

#include <DMA.h>
#include <GPIO.h>
#include <USART.h>

char hello[] = "Hello, USART via DMA (over and over)!\r\n";

void sayHelloOnce() { DMA_2.enableStream(7); }

void handleTxDMAEvent(DMA::StreamEvent event) {
  switch (event.type) {
  case DMA::StreamEventType::TRANSFER_COMPLETE:
    sayHelloOnce();
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  GPIO_A.enable();

  GPIO_A.setMode(9, GPIO::PinMode::ALTERNATE, 7);
  GPIO_A.setMode(10, GPIO::PinMode::ALTERNATE, 7);
  USART_1.enable(115200);
  USART_1.enableTxDMA();

  GPIO_C.enable();
  GPIO_C.setMode(7, GPIO::PinMode::OUTPUT);

  DMA_2.enable();
  DMA_2.configureStream(7, 4, DMA::Direction::MEM_TO_PERI,
                        sizeof(hello) / sizeof(hello[0]) - 1,
                        DMA::FIFOThreshold::DIRECT, false,
                        DMA::Priority::VERY_HIGH, hello, DMA::Size::BYTE, true,
                        &USART1->DR, DMA::Size::BYTE, false, handleTxDMAEvent);

  sayHelloOnce();

  while (true) {
  }
}
