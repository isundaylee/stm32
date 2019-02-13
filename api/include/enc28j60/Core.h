#pragma once

#include "enc28j60/Consts.h"

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>

namespace enc28j60 {

class Core {
private:
  // Peripheral dependencies
  SPI* spi_;
  GPIO::Pin pinCS_;
  GPIO::Pin pinInt_;

  ControlRegBank currentBank_ = ControlRegBank::BANK_DONT_CARE;

  void selectControlRegBank(ControlRegBank bank);

public:
  Core() {}

  void enable(SPI* spi, GPIO::Pin pinCS, GPIO::Pin pinInt);

  void setETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                         uint8_t bits);
  void clearETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                           uint8_t bits);

  uint8_t readETHReg(ControlRegBank bank, ControlRegAddress addr);
  uint8_t readMACMIIReg(ControlRegBank bank, ControlRegAddress addr);
  uint8_t writeControlReg(ControlRegBank bank, ControlRegAddress addr,
                          uint8_t value);

  uint16_t readPHYReg(PHYRegAddress addr);
  void writePHYReg(PHYRegAddress addr, uint16_t value);

  void readBufferMemory(uint16_t* data, size_t len);
  void readBufferMemoryStart();
  void readBufferMemoryEnd();
};

}; // namespace enc28j60
