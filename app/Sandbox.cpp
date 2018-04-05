#include "ADC.h"
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

  ADC_1.selectChannel(10);
  ADC_1.startContinuousConversion();
  while (true) {
    USART_1.write("Conversion result: ");
    USART_1.write(HexString(ADC_1.getContinuousConversion()));
    USART_1.write("\n");

    int data = USART_1.read();
    if (data == '1') {
      ADC_1.startContinuousConversion();
    } else if (data == '0') {
      ADC_1.stopContinuousConversion();
    }
  }
}
