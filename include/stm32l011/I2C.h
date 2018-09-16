#pragma once

#include <DeviceHeader.h>
#include <Utils.h>

enum class I2C_SCL_Pin {
  I2C1_PA4,
  I2C1_PA9,
  I2C1_PB6,
  I2C1_PB8,
};

enum class I2C_SDA_Pin {
  I2C1_PA10,
  I2C1_PA13,
  I2C1_PB7,
};

class I2C {
private:
  I2C_TypeDef* i2c_;

public:
  I2C(I2C_TypeDef* i2c) { i2c_ = i2c; }

  I2C(I2C&& move) = delete;
  I2C(I2C const& copy) = delete;
  I2C& operator=(I2C&& move) = delete;
  I2C& operator=(I2C const& copy) = delete;

  void enable(I2C_SCL_Pin scl, I2C_SDA_Pin sda);

  bool write(uint8_t addr, size_t len, uint8_t* data);
};

extern I2C I2C_1;
