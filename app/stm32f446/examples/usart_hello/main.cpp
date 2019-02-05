#include <Utils.h>

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
  USART_1.write("Hello, USART!\r\n");

  while (true) {
  }
}
