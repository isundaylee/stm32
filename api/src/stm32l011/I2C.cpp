#include "I2C.h"
#include "GPIO.h"

I2C I2C_1(I2C1);

void I2C::enable(SCLPin scl, SDAPin sda) {
  // Sets up SCL
  switch (scl) {
  case SCLPin::I2C1_PA4:
    GPIO_A.setMode(4, GPIO::PinMode::ALTERNATE, 3);
    GPIO_A.setOutputMode(4, GPIO::OutputMode::OPEN_DRAIN);
    break;
  case SCLPin::I2C1_PA9:
    GPIO_A.setMode(9, GPIO::PinMode::ALTERNATE, 1);
    GPIO_A.setOutputMode(9, GPIO::OutputMode::OPEN_DRAIN);
    break;
  case SCLPin::I2C1_PB6:
    GPIO_B.setMode(6, GPIO::PinMode::ALTERNATE, 1);
    GPIO_B.setOutputMode(6, GPIO::OutputMode::OPEN_DRAIN);
    break;
  case SCLPin::I2C1_PB8:
    GPIO_B.setMode(6, GPIO::PinMode::ALTERNATE, 4);
    GPIO_B.setOutputMode(6, GPIO::OutputMode::OPEN_DRAIN);
    break;
  }

  // Sets up SDA
  switch (sda) {
  case SDAPin::I2C1_PA10:
    GPIO_A.setMode(10, GPIO::PinMode::ALTERNATE, 1);
    GPIO_A.setOutputMode(10, GPIO::OutputMode::OPEN_DRAIN);
    break;
  case SDAPin::I2C1_PA13:
    GPIO_A.setMode(13, GPIO::PinMode::ALTERNATE, 3);
    GPIO_A.setOutputMode(13, GPIO::OutputMode::OPEN_DRAIN);
    break;
  case SDAPin::I2C1_PB7:
    GPIO_B.setMode(7, GPIO::PinMode::ALTERNATE, 1);
    GPIO_B.setOutputMode(7, GPIO::OutputMode::OPEN_DRAIN);
    break;
  }

  if (i2c_ == I2C1) {
    BIT_SET(RCC->APB1ENR, RCC_APB1ENR_I2C1EN);

    BIT_CLEAR(i2c_->CR1, I2C_CR1_PE);

    uint32_t timingr = 0;
    timingr |= (3 << I2C_TIMINGR_PRESC_Pos);
    timingr |= (19 << I2C_TIMINGR_SCLL_Pos);
    timingr |= (15 << I2C_TIMINGR_SCLH_Pos);
    timingr |= (2 << I2C_TIMINGR_SDADEL_Pos);
    timingr |= (4 << I2C_TIMINGR_SCLDEL_Pos);

    i2c_->TIMINGR = timingr;

    i2c_->CR1 = I2C_CR1_PE;
  }
}

bool I2C::write(uint8_t addr, size_t len, uint8_t* data) {
  WAIT_UNTIL((i2c_->ISR & I2C_ISR_BUSY) == 0);

  i2c_->CR2 = (len << I2C_CR2_NBYTES_Pos) | (addr << 1);
  i2c_->CR2 |= I2C_CR2_START;

  for (size_t i = 0; i < len; i++) {
    while (true) {
      if ((i2c_->ISR & I2C_ISR_TXIS) != 0) {
        break;
      }

      if ((i2c_->ISR & I2C_ISR_NACKF) != 0) {
        i2c_->ICR = I2C_ICR_NACKCF;
        return false;
      }
    }

    i2c_->TXDR = data[i];
  }

  while (true) {
    if ((i2c_->ISR & I2C_ISR_TC) != 0) {
      i2c_->CR2 |= I2C_CR2_STOP;
      return true;
    }

    if ((i2c_->ISR & I2C_ISR_NACKF) != 0) {
      i2c_->ICR = I2C_ICR_NACKCF;
      return false;
    }
  }
}
