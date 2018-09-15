#include "GPIO.h"
#include "I2C.h"
#include "Clock.h"

void sendCommand(I2C& i2c, uint8_t cmd) {
  uint8_t data[] = {0x00, cmd};
  i2c.write(0x3C, sizeof(data) / sizeof(data[0]), data);
}

void sendCommand(I2C& i2c, uint8_t cmd, uint8_t val) {
  uint8_t data[] = {0x00, cmd, val};
  i2c.write(0x3C, sizeof(data) / sizeof(data[0]), data);
}

void sendCommand(I2C& i2c, uint8_t cmd, uint8_t val1, uint8_t val2) {
  uint8_t data[] = {0x00, cmd, val1, val2};
  i2c.write(0x3C, sizeof(data) / sizeof(data[0]), data);
}

extern "C" void main() {
  Clock::enableHSI();
  Clock::switchSysclk(Clock::Sysclk::HSI);

  GPIO_A.enable();
  GPIO_A.setMode(1, GPIO_MODE_OUTPUT, 1);

  I2C_1.enable(I2C_SCL_Pin::I2C1_PA4, I2C_SDA_Pin::I2C1_PA10);

  sendCommand(I2C_1, 0xAE);
  sendCommand(I2C_1, 0x8D, 0x14);
  sendCommand(I2C_1, 0xA4);
  sendCommand(I2C_1, 0xAF);
  sendCommand(I2C_1, 0x20, 0x00);
  sendCommand(I2C_1, 0x21, 0x00, 0x7F);
  sendCommand(I2C_1, 0x22, 0x00, 0x07);

  for (int k=0; k<64; k++) {
    uint8_t data[17];

    data[0] = 0x40;

    for (int i=0; i<16; i++) {
      if ((i < 8) || (k / 8) % 2 == 1) {
        data[i + 1] = 0xFF;
      } else {
        data[i + 1] = 0x00;
      }
    }

    I2C_1.write(0x3C, 17, data);
  }

  while (true) {
  }
}
