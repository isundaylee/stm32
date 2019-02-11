#include "ENC28J60.h"

#include <USART.h>

uint8_t ENC28J60::generateHeaderByte(Opcode opcode, ControlRegAddress addr) {
  return (static_cast<uint8_t>(opcode) << 5) + static_cast<uint8_t>(addr);
}

void ENC28J60::selectControlRegBank(ControlRegBank bank) {
  static auto currentBank = ControlRegBank::BANK_0;

  if (currentBank == bank) {
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

void ENC28J60::setETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                                 uint8_t bits) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::BIT_FIELD_SET, addr), bits};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);
}

void ENC28J60::clearETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                                   uint8_t bits) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::BIT_FIELD_CLEAR, addr), bits};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);
}

uint8_t ENC28J60::readETHReg(ControlRegBank bank, ControlRegAddress addr) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::READ_CONTROL_REGISTER, addr),
                     0x00};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);

  return static_cast<uint8_t>(data[1]);
}

uint8_t ENC28J60::writeControlReg(ControlRegBank bank, ControlRegAddress addr,
                                  uint8_t value) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::WRITE_CONTROL_REGISTER, addr),
                     value};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);

  return static_cast<uint8_t>(data[1]);
}

uint8_t ENC28J60::readMACMIIReg(ControlRegBank bank, ControlRegAddress addr) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::READ_CONTROL_REGISTER, addr),
                     0x00, 0x00};

  gpioCS_->clear(pinCS_);
  spi_->transact(data, sizeof(data) / sizeof(data[0]));
  gpioCS_->set(pinCS_);

  return static_cast<uint8_t>(data[2]);
}

uint16_t ENC28J60::readPHYReg(PHYRegAddress addr) {
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

void ENC28J60::writePHYReg(PHYRegAddress addr, uint16_t value) {
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIREGADR,
                  static_cast<uint8_t>(addr));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIWRL,
                  static_cast<uint8_t>(value & 0x00FF));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIWRH,
                  static_cast<uint8_t>(value >> 8));
  WAIT_UNTIL(BIT_IS_CLEAR(
      readMACMIIReg(ControlRegBank::BANK_3, ControlRegAddress::MISTAT), 0x01));
}

void ENC28J60::readBufferMemoryStart() {
  uint16_t header[] = {generateHeaderByte(
      Opcode::READ_BUFFER_MEMORY, ControlRegAddress::READ_BUFFER_MEMORY)};

  GPIO_B.clear(12);
  SPI_2.transact(header, sizeof(header) / sizeof(header[0]));
}

void ENC28J60::readBufferMemoryEnd() { GPIO_B.set(12); }

void ENC28J60::readBufferMemory(uint16_t* data, size_t len) {
  readBufferMemoryStart();
  SPI_2.transact(data, len);
  readBufferMemoryEnd();
}

void ENC28J60::initializeMAC() {
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON1,
                  MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON3,
                  MACON3_PADCFG0 | MACON3_FRMLNEN | MACON3_FULDPX);
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON4,
                  MACON4_DEFER);

  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAMXFLL, 0x00);
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAMXFLH, 0x06);

  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MABBIPG, 0x15);
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAIPGL, 0x12);

  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR1, 0x11);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR2, 0x22);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR3, 0x33);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR4, 0x44);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR5, 0x55);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR6, 0x66);
}

void ENC28J60::initializeETH() {
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXSTL, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXSTH, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXNDL, 0xFF);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXNDH, 0x0F);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTL, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTH, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTL, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTH, 0x00);

  writeControlReg(ControlRegBank::BANK_1, ControlRegAddress::ERXFCON, 0x00);

  WAIT_UNTIL(
      BIT_IS_SET(readETHReg(ControlRegBank::BANK_0, ControlRegAddress::ESTAT),
                 0b00000001));
}

void ENC28J60::initializePHY() {
  writePHYReg(PHYRegAddress::PHCON1, PHCON1_PDPXMD);
}

void enc28j60HandleInterruptWrapper(void* context) {
  static_cast<ENC28J60*>(context)->handleInterrupt();
}

void enc28j60HandleRxDMAEventWrapper(DMA::StreamEvent event, void* context) {
  static_cast<ENC28J60*>(context)->handleRxDMAEvent(event);
}

void ENC28J60::enable(SPI* spi, GPIO* gpioCS, int pinCS, GPIO* gpioInt,
                      int pinInt, DMA* dmaTx, int dmaStreamTx, int dmaChannelTx,
                      DMA* dmaRx, int dmaStreamRx, int dmaChannelRx,
                      Mode mode) {
  spi_ = spi;
  gpioCS_ = gpioCS;
  pinCS_ = pinCS;
  gpioInt_ = gpioInt;
  pinInt_ = pinInt;
  dmaTx_ = dmaTx;
  dmaStreamTx_ = dmaStreamTx;
  dmaChannelTx_ = dmaChannelTx;
  dmaRx_ = dmaRx;
  dmaStreamRx_ = dmaStreamRx;
  dmaChannelRx_ = dmaChannelRx;
  mode_ = mode;

  GPIO_C.enableExternalInterrupt(9, GPIO::TriggerDirection::FALLING_EDGE,
                                 enc28j60HandleInterruptWrapper, this);

  initializeETH();
  initializeMAC();
  initializePHY();
}

bool ENC28J60::receivePacketHeader() {
  if (readETHReg(ControlRegBank::BANK_1, ControlRegAddress::EPKTCNT) == 0) {
    return false;
  }

  readBufferMemory(packetHeader_, 6);

  size_t frameLen = static_cast<size_t>(packetHeader_[2]) +
                    (static_cast<size_t>(packetHeader_[3]) << 8);

  size_t transactionSize = frameLen + (frameLen % 2);

  readBufferMemoryStart();
  spi_->enableTxDMA();
  spi_->enableRxDMA();
  dmaTx_->enable();
  dmaRx_->enable();
  dmaTx_->configureStream(
      dmaStreamTx_, dmaChannelTx_, DMA::Direction::MEM_TO_PERI, transactionSize,
      DMA::FIFOThreshold::DIRECT, false, DMA::Priority::VERY_HIGH, frameData_,
      DMA::Size::BYTE, true, &SPI2->DR, DMA::Size::BYTE, false, nullptr,
      nullptr);
  dmaRx_->configureStream(
      dmaStreamRx_, dmaChannelRx_, DMA::Direction::PERI_TO_MEM, transactionSize,
      DMA::FIFOThreshold::DIRECT, false, DMA::Priority::VERY_HIGH, &SPI2->DR,
      DMA::Size::BYTE, false, frameData_, DMA::Size::BYTE, true,
      enc28j60HandleRxDMAEventWrapper, this);
  dmaTx_->enableStream(dmaStreamTx_);
  dmaRx_->enableStream(dmaStreamRx_);

  return true;
}

void ENC28J60::receivePacketCleanup() {
  readBufferMemoryEnd();
  SPI_2.disableRxDMA();
  SPI_2.disableTxDMA();

  writeControlReg(ENC28J60::ControlRegBank::BANK_0,
                  ENC28J60::ControlRegAddress::ERXRDPTL,
                  static_cast<uint8_t>(packetHeader_[0]));
  writeControlReg(ENC28J60::ControlRegBank::BANK_0,
                  ENC28J60::ControlRegAddress::ERXRDPTH,
                  static_cast<uint8_t>(packetHeader_[1]));
  setETHRegBitField(ENC28J60::ControlRegBank::BANK_0,
                    ENC28J60::ControlRegAddress::ECON2, ENC28J60::ECON2_PKTDEC);
}

void ENC28J60::process() {
  while (!events_.empty()) {
    Event event{};
    events_.pop(event);

    switch (event) {
    case Event::INTERRUPT: {
      if (state_ == State::IDLE) {
        state_ = State::RX_HEADER;
      }
      break;
    }

    case Event::RX_DMA_COMPLETE: {
      if (state_ == State::RX_FRAME_PENDING) {
        state_ = State::RX_FRAME_DONE;
      } else {
        // TODO: Error handling
      }

      break;
    }
    }

    switch (state_) {
    case State::IDLE:
    case State::RX_FRAME_PENDING: {
      // Nothing to do
      break;
    }

    case State::RX_HEADER: {
      if (!receivePacketHeader()) {
        state_ = State::IDLE;
      } else {
        state_ = State::RX_FRAME_PENDING;
      }

      break;
    }

    case State::RX_FRAME_DONE: {
      receivePacketCleanup();
      state_ = State::RX_HEADER;

      break;
    }
    }
  }
}