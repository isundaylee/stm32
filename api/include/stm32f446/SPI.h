#pragma once

#include <DeviceHeader.h>

class SPI {
private:
  SPI_TypeDef* spi_;

public:
  enum class ClockPolarity {
    IDLE_LOW = 0b0,
    IDLE_HIGH = 0b1,
  };

  enum class ClockPhase {
    SAMPLE_ON_FIRST_EDGE = 0b0,
    SAMPLE_ON_SECOND_EDGE = 0b1,
  };

  enum class DataFrameFormat {
    BYTE = 0b0,
    HALFWORD = 0b1,
  };

  enum class BaudRate {
    PCLK_OVER_2 = 0b000,
    PCLK_OVER_4 = 0b001,
    PCLK_OVER_8 = 0b010,
    PCLK_OVER_16 = 0b011,
    PCLK_OVER_32 = 0b100,
    PCLK_OVER_64 = 0b101,
    PCLK_OVER_128 = 0b110,
    PCLK_OVER_256 = 0b111,
  };

  enum class NSSMode {
    AUTOMATIC,
    MANUAL,
  };

  SPI(SPI_TypeDef* spi) : spi_(spi) {}

  void enableMaster(ClockPolarity cpol, ClockPhase cpha, DataFrameFormat dff,
                    BaudRate baud, NSSMode nssMode);
  void disable();

  bool transmit(uint16_t const* data, size_t len);
};

extern SPI SPI_1;
extern SPI SPI_2;
extern SPI SPI_3;
extern SPI SPI_4;
