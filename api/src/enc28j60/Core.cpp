#include "enc28j60/Core.h"

#include "enc28j60/ENC28J60.h"

namespace enc28j60 {

// TODO: Fix dup!
static uint8_t lowByte(uint16_t num) { return (num & 0x00FF); }
static uint8_t highByte(uint16_t num) { return (num & 0xFF00) >> 8; }

////////////////////////////////////////////////////////////////////////////////
// Low-level interface
////////////////////////////////////////////////////////////////////////////////

static uint8_t generateHeaderByte(Opcode opcode, ControlRegAddress addr) {
  return (static_cast<uint8_t>(opcode) << 5) + static_cast<uint8_t>(addr);
}

void Core::selectControlRegBank(ControlRegBank bank) {
  if ((currentBank_ == bank) || (bank == ControlRegBank::BANK_DONT_CARE)) {
    return;
  } else {
    currentBank_ = bank;
  }

  uint16_t data[] = {
      generateHeaderByte(Opcode::BIT_FIELD_CLEAR, ControlRegAddress::ECON1),
      0b00000011};

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(data, sizeof(data) / sizeof(data[0]));
  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);

  data[0] = generateHeaderByte(Opcode::BIT_FIELD_SET, ControlRegAddress::ECON1);
  data[1] = static_cast<uint8_t>(bank);

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(data, sizeof(data) / sizeof(data[0]));
  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);
}

void Core::setETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                             uint8_t bits) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::BIT_FIELD_SET, addr), bits};

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(data, sizeof(data) / sizeof(data[0]));
  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);
}

void Core::clearETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                               uint8_t bits) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::BIT_FIELD_CLEAR, addr), bits};

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(data, sizeof(data) / sizeof(data[0]));
  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);
}

uint8_t Core::readETHReg(ControlRegBank bank, ControlRegAddress addr) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::READ_CONTROL_REGISTER, addr),
                     0x00};

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(data, sizeof(data) / sizeof(data[0]));
  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);

  if (addr == ControlRegAddress::EIR) {
    DEBUG_ASSERT((data[1] & 0b10110100) == 0, "Unexpected EIR value.");
  }

  return static_cast<uint8_t>(data[1]);
}

uint8_t Core::writeControlReg(ControlRegBank bank, ControlRegAddress addr,
                              uint8_t value) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::WRITE_CONTROL_REGISTER, addr),
                     value};

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(data, sizeof(data) / sizeof(data[0]));
  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);

  return static_cast<uint8_t>(data[1]);
}

uint8_t Core::readMACMIIReg(ControlRegBank bank, ControlRegAddress addr) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::READ_CONTROL_REGISTER, addr),
                     0x00, 0x00};

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(data, sizeof(data) / sizeof(data[0]));
  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);

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

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(header, sizeof(header) / sizeof(header[0]));
}

void Core::readBufferMemoryEnd(size_t bytesRead) {
  currentReadPointer_ += bytesRead;
  currentReadPointer_ %= (CONFIG_ERXND + 1);

  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);
}

void Core::readBufferMemory(uint16_t* data, size_t len) {
  readBufferMemoryStart();
  parent_.spi_->transact(data, len);
  readBufferMemoryEnd(len);
}

void Core::fakeReadBufferMemory(size_t len) {
  currentReadPointer_ += len;
  currentReadPointer_ %= (CONFIG_ERXND + 1);

  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTL,
                  lowByte(currentReadPointer_));
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTH,
                  highByte(currentReadPointer_));
}

void Core::writeBufferMemoryStart() {
  uint16_t header[] = {generateHeaderByte(
      Opcode::WRITE_BUFFER_MEMORY, ControlRegAddress::READ_BUFFER_MEMORY)};

  parent_.pinCS_.gpio->clear(parent_.pinCS_.pin);
  parent_.spi_->transact(header, sizeof(header) / sizeof(header[0]));
}

void Core::writeBufferMemoryEnd() {
  parent_.pinCS_.gpio->set(parent_.pinCS_.pin);
}

}; // namespace enc28j60
