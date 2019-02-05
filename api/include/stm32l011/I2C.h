#pragma once

#include <DeviceHeader.h>
#include <Utils.h>

class I2C {
private:
  I2C_TypeDef* i2c_;

public:
  enum class SCLPin {
    I2C1_PA4,
    I2C1_PA9,
    I2C1_PB6,
    I2C1_PB8,
  };

  enum class SDAPin {
    I2C1_PA10,
    I2C1_PA13,
    I2C1_PB7,
  };
  
  I2C(I2C_TypeDef* i2c) { i2c_ = i2c; }

  I2C(I2C&& move) = delete;
  I2C(I2C const& copy) = delete;
  I2C& operator=(I2C&& move) = delete;
  I2C& operator=(I2C const& copy) = delete;

  void enable(SCLPin scl, SDAPin sda);

  bool write(uint8_t addr, size_t len, uint8_t* data);
};

extern I2C I2C_1;
