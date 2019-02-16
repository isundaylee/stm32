#pragma once

#include "enc28j60/Consts.h"

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>

namespace enc28j60 {

class ENC28J60;
class Receiver;

class Core {
private:
  ENC28J60& parent_;

  uint16_t currentReadPointer_;
  ControlRegBank currentBank_ = ControlRegBank::BANK_DONT_CARE;

  Core(ENC28J60& parent) : parent_(parent) {}

  void selectControlRegBank(ControlRegBank bank);

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
  void readBufferMemoryEnd(size_t bytesRead);
  
  void writeBufferMemoryStart();
  void writeBufferMemoryEnd();

  friend class ENC28J60;
  friend class Receiver;
};

}; // namespace enc28j60
