#include "enc28j60/ENC28J60.h"

#include <USART.h>

namespace enc28j60 {

////////////////////////////////////////////////////////////////////////////////
// Initialization routines
////////////////////////////////////////////////////////////////////////////////

static const uint16_t CONFIG_ERXST = 0x0000;
static const uint16_t CONFIG_ERXND = 0x0FFF;

static uint8_t lowByte(uint16_t num) { return (num & 0x00FF); }
static uint8_t highByte(uint16_t num) { return (num & 0xFF00) >> 8; }
static uint16_t mergeBytes(uint8_t low, uint8_t high) {
  return ((static_cast<uint16_t>(high) << 8) + low);
}

void ENC28J60::initializeMAC() {
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON1,
                        MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON3,
                        MACON3_PADCFG0 | MACON3_FRMLNEN | MACON3_FULDPX);
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON4,
                        MACON4_DEFER);

  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAMXFLL,
                        0x00);
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAMXFLH,
                        0x06);

  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MABBIPG,
                        0x15);
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAIPGL,
                        0x12);

  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR1,
                        0x11);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR2,
                        0x22);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR3,
                        0x33);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR4,
                        0x44);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR5,
                        0x55);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR6,
                        0x66);
}

void ENC28J60::initializeETH() {
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXSTL,
                        lowByte(CONFIG_ERXST));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXSTH,
                        highByte(CONFIG_ERXST));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXNDL,
                        lowByte(CONFIG_ERXND));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXNDH,
                        highByte(CONFIG_ERXND));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTL,
                        lowByte(CONFIG_ERXST));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTH,
                        highByte(CONFIG_ERXST));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTL,
                        lowByte(CONFIG_ERXND));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTH,
                        highByte(CONFIG_ERXND));

  core_.writeControlReg(ControlRegBank::BANK_1, ControlRegAddress::ERXFCON,
                        0x00);
  WAIT_UNTIL(BIT_IS_SET(
      core_.readETHReg(ControlRegBank::BANK_0, ControlRegAddress::ESTAT),
      0b00000001));
}

void ENC28J60::initializePHY() {
  core_.writePHYReg(PHYRegAddress::PHCON1, PHCON1_PDPXMD);
}

void ENC28J60::enable(SPI* spi, GPIO::Pin pinCS, GPIO::Pin pinInt,
                      DMA::Channel dmaTx, DMA::Channel dmaRx, Mode mode,
                      EventHandler eventHandler, void* eventHandlerContext) {
  spi_ = spi;
  pinCS_ = pinCS;
  pinInt_ = pinInt;
  dmaTx_ = dmaTx;
  dmaRx_ = dmaRx;

  mode_ = mode;
  eventHandler_ = eventHandler;
  eventHandlerContext_ = eventHandlerContext;

  core_.enable(spi, pinCS, pinInt);

  pinInt_.gpio->setupExternalInterrupt(pinInt_.pin,
                                       GPIO::TriggerDirection::FALLING_EDGE,
                                       handleInterruptWrapper, this);

  initializeETH();
  initializeMAC();
  initializePHY();
}

void ENC28J60::enableRx() {
  core_.setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::ECON1,
                          ECON1_RXEN);
  core_.setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::EIE,
                          EIE_INTIE | EIE_PKTIE | EIE_RXERIE);
  pinInt_.gpio->enableExternalInterrupt(pinInt_.pin);
}

////////////////////////////////////////////////////////////////////////////////
// Interrupt handlers...
////////////////////////////////////////////////////////////////////////////////

void ENC28J60::handleInterrupt() {
  // We handle interrupt one at a time.
  // Inspiration taken from the Linux kernel driver.
  pinInt_.gpio->disableExternalInterrupt(pinInt_.pin);

  fsm_.pushEvent(FSM::Event::INTERRUPT);
}

void ENC28J60::handleRxDMAEvent(DMA::StreamEvent event) {
  switch (event.type) {
  case DMA::StreamEventType::TRANSFER_COMPLETE:
    fsm_.pushEvent(FSM::Event::RX_DMA_COMPLETE);
    break;
  }
}

void handleInterruptWrapper(void* context) {
  static_cast<ENC28J60*>(context)->handleInterrupt();
}

void handleRxDMAEventWrapper(DMA::StreamEvent event, void* context) {
  static_cast<ENC28J60*>(context)->handleRxDMAEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
// FSM helper functions
////////////////////////////////////////////////////////////////////////////////

bool ENC28J60::receivePacketHeader() {
  uint8_t packetCount =
      core_.readETHReg(ControlRegBank::BANK_1, ControlRegAddress::EPKTCNT);

  if (packetCount == 0) {
    return false;
  }

  if (packetCount > stats.maxPKTCNT) {
    stats.maxPKTCNT = packetCount;
  }

  // Allocate the new RX packet
  currentRxPacket_ = rxPacketBuffer_.allocate();

  // Reader packet header and calculate frame size
  uint16_t* packetHeader =
      (!!currentRxPacket_ ? currentRxPacket_->header : devNullHeader_);
  size_t frameLen;

  core_.readBufferMemory(packetHeader, PACKET_HEADER_SIZE);
  frameLen = static_cast<size_t>(packetHeader[2]) +
             (static_cast<size_t>(packetHeader[3]) << 8);

  if (!!currentRxPacket_) {
    currentRxPacket_->frameLength = frameLen;
  }

  // Starts DMA transactions to read packet frame
  size_t transactionSize = frameLen + (frameLen % 2);

  core_.readBufferMemoryStart();
  spi_->enableTxDMA();
  spi_->enableRxDMA();
  dmaTx_.dma->enable();
  dmaRx_.dma->enable();

  uint8_t* rxDst =
      (!!currentRxPacket_ ? currentRxPacket_->frame : &devNullFrame_);
  bool rxDstInc = (!!currentRxPacket_ ? true : false);

  dmaTx_.dma->configureStream(
      dmaTx_.stream, dmaTx_.channel, DMA::Direction::MEM_TO_PERI,
      transactionSize, DMA::FIFOThreshold::DIRECT, false,
      DMA::Priority::VERY_HIGH, rxDst, DMA::Size::BYTE, rxDstInc, &SPI2->DR,
      DMA::Size::BYTE, false, nullptr, nullptr);
  dmaRx_.dma->configureStream(
      dmaRx_.stream, dmaRx_.channel, DMA::Direction::PERI_TO_MEM,
      transactionSize, DMA::FIFOThreshold::DIRECT, false,
      DMA::Priority::VERY_HIGH, &SPI2->DR, DMA::Size::BYTE, false, rxDst,
      DMA::Size::BYTE, rxDstInc, handleRxDMAEventWrapper, this);

  dmaTx_.dma->enableStream(dmaTx_.stream);
  dmaRx_.dma->enableStream(dmaRx_.stream);

  return true;
}

void ENC28J60::postEvent(Event event) {
  if (!!eventHandler_) {
    eventHandler_(event, eventHandlerContext_);
  }
}

////////////////////////////////////////////////////////////////////////////////
// FSM goes round and round...
////////////////////////////////////////////////////////////////////////////////

void ENC28J60::fsmActionRxCleanup() {
  core_.readBufferMemoryEnd();
  SPI_2.disableRxDMA();
  SPI_2.disableTxDMA();

  uint16_t* packetHeader =
      (!!currentRxPacket_ ? currentRxPacket_->header : devNullHeader_);

  uint16_t newERXRDPT = mergeBytes(packetHeader[0], packetHeader[1]);
  // ENC28J60 errata issue 14
  newERXRDPT = (newERXRDPT == CONFIG_ERXST ? CONFIG_ERXND : newERXRDPT - 1);

  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTL,
                        lowByte(newERXRDPT));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTH,
                        highByte(newERXRDPT));
  core_.setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::ECON2,
                          ECON2_PKTDEC);

  if (!!currentRxPacket_) {
    rxBuffer.push(currentRxPacket_);

    stats.rxBytes += currentRxPacket_->frameLength;
    stats.rxPackets++;
  } else {
    stats.rxPacketsLostInDriver++;
  }

  postEvent((!!currentRxPacket_) ? Event::RX_NEW_PACKET : Event::RX_OVERFLOW);

  currentRxPacket_ = nullptr;

  fsm_.pushEvent(FSMEvent::INTERRUPT);
}

void ENC28J60::fsmActionCheckEIR() {
  uint8_t eir =
      core_.readETHReg(ControlRegBank::BANK_DONT_CARE, ControlRegAddress::EIR);

  if (BIT_IS_SET(eir, EIR_RXERIF)) {
    postEvent(Event::RX_CHIP_OVERFLOW);
    core_.clearETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                              ControlRegAddress::EIR, EIR_RXERIF);
  }

  // We should always try to do an Rx even regardless of the value of
  // EIR_PKTIF. See ENC28J60 errata issue 6.
  if (receivePacketHeader()) {
    fsm_.pushEvent(FSMEvent::RX_HEADER_READ);
  } else {
    fsm_.pushEvent(FSMEvent::RX_ALL_DONE);
  }
}

void ENC28J60::fsmActionEnableInt() {
  pinInt_.gpio->enableExternalInterrupt(pinInt_.pin);
}

/* static */ ENC28J60::FSM::Transition ENC28J60::fsmTransitions_[] = {
    // clang-format off
    {FSMState::IDLE,        FSMEvent::INTERRUPT,        &ENC28J60::fsmActionCheckEIR,   FSMState::EIR_CHECKED},
    {FSMState::EIR_CHECKED, FSMEvent::RX_HEADER_READ,   nullptr,                        FSMState::RX_PENDING},
    {FSMState::EIR_CHECKED, FSMEvent::RX_ALL_DONE,      &ENC28J60::fsmActionEnableInt,  FSMState::IDLE},
    {FSMState::RX_PENDING,  FSMEvent::RX_DMA_COMPLETE,  &ENC28J60::fsmActionRxCleanup,  FSMState::IDLE},
    FSM::TransitionTerminator,
    // clang-format on
};

////////////////////////////////////////////////////////////////////////////////
// API (except initialization)
////////////////////////////////////////////////////////////////////////////////

void ENC28J60::process() { fsm_.processOneEvent(); }

bool ENC28J60::linkIsUp() {
  return BIT_IS_SET(core_.readPHYReg(PHYRegAddress::PHSTAT1), PHSTAT1_LLSTAT);
}

void ENC28J60::freeRxPacket(Packet* packet) { rxPacketBuffer_.free(packet); }

}; // namespace enc28j60
