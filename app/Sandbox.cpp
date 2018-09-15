#include "GPIO.h"
#include "I2C.h"
#include "Clock.h"

#include "OLED.h"

OLED<0x3C, 64, 128> oled(I2C_1);

extern "C" void main() {
  Clock::enableHSI();
  Clock::switchSysclk(Clock::Sysclk::HSI);

  GPIO_A.enable();
  GPIO_A.setMode(1, GPIO_MODE_OUTPUT, 1);

  I2C_1.enable(I2C_SCL_Pin::I2C1_PA4, I2C_SDA_Pin::I2C1_PA10);

  oled.turnDisplayOff();
  oled.enableChargePump();
  oled.disableEntireDisplay();
  oled.turnDisplayOn();

  oled.clearScreen();
  oled.rectangle(0, 15, 0, 15);

  if (!oled.render()) {
    GPIO_A.set(1);
  }

  while (true) {
  }
}
