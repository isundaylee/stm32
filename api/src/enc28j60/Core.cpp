#include "enc28j60/Core.h"

namespace enc28j60 {

void Core::enable(SPI* spi, GPIO* gpioCS, int pinCS, GPIO* gpioInt,
                  int pinInt) {
  spi_ = spi;
  gpioCS_ = gpioCS;
  pinCS_ = pinCS;
  gpioInt_ = gpioInt;
  pinInt_ = pinInt;
}

////////////////////////////////////////////////////////////////////////////////
// Low-level interface
////////////////////////////////////////////////////////////////////////////////

static uint8_t generateHeaderByte(Opcode opcode, ControlRegAddress addr) {
  return (static_cast<uint8_t>(opcode) << 5) + static_cast<uint8_t>(addr);
}

void Core::selectControlRegBank(ControlRegBank bank) {
  // TODO: Fix me!
  static auto currentBank = ControlRegBank::BANK_0;

  if ((currentBank == bank) || (bank == ControlRegBank::BANK_DONT_CARE)) {
    return;
  } else {
    currentBank = bank;
  }

  uint16_t data[] = {
      generateHeaderByte(Opcode::BIT_FIELD_CLEAR, ControlRegAddress::ECON1),
      0b00000011};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);

  data[0] = generateHeaderByte(Opcode::BIT_FIELD_SET, ControlRegAddress::ECON1);
  data[1] = static_cast<uint8_t>(bank);

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);
}

void Core::setETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                             uint8_t bits) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::BIT_FIELD_SET, addr), bits};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);
}

void Core::clearETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                               uint8_t bits) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::BIT_FIELD_CLEAR, addr), bits};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);
}

uint8_t Core::readETHReg(ControlRegBank bank, ControlRegAddress addr) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::READ_CONTROL_REGISTER, addr),
                     0x00};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);

  return static_cast<uint8_t>(data[1]);
}

uint8_t Core::writeControlReg(ControlRegBank bank, ControlRegAddress addr,
                              uint8_t value) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::WRITE_CONTROL_REGISTER, addr),
                     value};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);

  return static_cast<uint8_t>(data[1]);
}

uint8_t Core::readMACMIIReg(ControlRegBank bank, ControlRegAddress addr) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::READ_CONTROL_REGISTER, addr),
                     0x00, 0x00};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);

  return static_cast<uint8_t>(data[2]);
}

uint16_t Core::readPHYReg(PHYRegAddress addr) {
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIREGADR,
                  static_cast<uint8_t>(addr));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MICMD, 0x01);
  WAIT_UNTIL(BIT_IS_CLEAR(
      readMACMIIReg(ControlRegBank::BANK_3, ControlRegAddress::MISTAT), 0x01));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MICMD, 0x00);

  uint8_t low = readMACMIIReg(ControlRegBank::BANK_2, ControlRegAddress::MIRDL);
  uint8_t high =
      readMACMIIReg(ControlRegBank::BANK_2, ControlRegAddress::MIRDH);

  return static_cast<uint16_t>(low) + (static_cast<uint16_t>(high) << 8);
}

void Core::writePHYReg(PHYRegAddress addr, uint16_t value) {
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIREGADR,
                  static_cast<uint8_t>(addr));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIWRL,
                  static_cast<uint8_t>(value & 0x00FF));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIWRH,
                  static_cast<uint8_t>(value >> 8));
  WAIT_UNTIL(BIT_IS_CLEAR(
      readMACMIIReg(ControlRegBank::BANK_3, ControlRegAddress::MISTAT), 0x01));
}

void Core::readBufferMemoryStart() {
  uint16_t header[] = {generateHeaderByte(
      Opcode::READ_BUFFER_MEMORY, ControlRegAddress::READ_BUFFER_MEMORY)};

  GPIO_B.clear(12);
  SPI_2.transact(header, sizeof(header) / sizeof(header[0]));
}

void Core::readBufferMemoryEnd() { GPIO_B.set(12); }

void Core::readBufferMemory(uint16_t* data, size_t len) {
  readBufferMemoryStart();
  SPI_2.transact(data, len);
  readBufferMemoryEnd();
}

}; // namespace enc28j60
