#include "enc28j60/Receiver.h"

#include "enc28j60/ENC28J60.h"

#include <USART.h>

namespace enc28j60 {

// TODO: Fix dup!
static uint8_t lowByte(uint16_t num) { return (num & 0x00FF); }
static uint8_t highByte(uint16_t num) { return (num & 0xFF00) >> 8; }
static uint16_t mergeBytes(uint8_t low, uint8_t high) {
  return ((static_cast<uint16_t>(high) << 8) + low);
}

void Receiver::handleTxDMAEvent(DMA::StreamEvent event) {
  switch (event.type) {
  case DMA::StreamEventType::TRANSFER_COMPLETE:
    fsm_.pushEvent(Receiver::FSM::Event::TX_DMA_COMPLETE);
    break;
  }
}

void handleTxDMAEventWrapper(DMA::StreamEvent event, void* context) {
  static_cast<Receiver*>(context)->handleTxDMAEvent(event);
}

void Receiver::handleRxDMAEvent(DMA::StreamEvent event) {
  switch (event.type) {
  case DMA::StreamEventType::TRANSFER_COMPLETE:
    fsm_.pushEvent(Receiver::FSM::Event::RX_DMA_COMPLETE);
    break;
  }
}

void handleRxDMAEventWrapper(DMA::StreamEvent event, void* context) {
  static_cast<Receiver*>(context)->handleRxDMAEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
// FSM goes round and round...
////////////////////////////////////////////////////////////////////////////////

void Receiver::fsmActionActivate() {
  // We handle interrupt one at a time.
  // Inspiration taken from the Linux kernel driver.
  parent_.pinInt_.gpio->disableExternalInterrupt(parent_.pinInt_.pin);

  fsm_.pushEvent(FSMEvent::NOW_ACTIVE);
}

void Receiver::fsmActionCheckEIR() {
  uint8_t eir = parent_.core_.readETHReg(ControlRegBank::BANK_DONT_CARE,
                                         ControlRegAddress::EIR);

  if (BIT_IS_SET(eir, EIR_RXERIF)) {
    parent_.postEvent(Event::RX_CHIP_OVERFLOW);
    parent_.core_.clearETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                                      ControlRegAddress::EIR, EIR_RXERIF);
  }

  if (!parent_.txBuffer.empty()) {
    // We prioritize Tx over Rx.
    fsm_.pushEvent(FSMEvent::TX_STARTED);
  } else {
    fsm_.pushEvent(FSMEvent::RX_STARTED);
  }
}

void Receiver::fsmActionRxStartDMA() {
  uint8_t packetCount = parent_.core_.readETHReg(ControlRegBank::BANK_1,
                                                 ControlRegAddress::EPKTCNT);

  if (packetCount == 0) {
    fsm_.pushEvent(FSMEvent::RX_ALL_DONE);
    return;
  }

  if (packetCount > parent_.stats.maxPKTCNT) {
    parent_.stats.maxPKTCNT = packetCount;
  }

  // Allocate the new RX packet
  currentRxPacket_ = parent_.packetBuffer_.allocate();

  // Reader packet header and calculate frame size
  uint16_t* packetHeader =
      (!!currentRxPacket_ ? currentRxPacket_->header : devNullHeader_);

  uint8_t ERDPTL = parent_.core_.readETHReg(ControlRegBank::BANK_0,
                                            ControlRegAddress::ERDPTL);
  uint8_t ERDPTH = parent_.core_.readETHReg(ControlRegBank::BANK_0,
                                            ControlRegAddress::ERDPTH);
  uint16_t ERDPT = mergeBytes(ERDPTL, ERDPTH);

  printf("\r\n0x%04x vs 0x%04x\r\n", ERDPT, parent_.core_.currentReadPointer_);

  parent_.core_.readBufferMemory(packetHeader, PACKET_HEADER_SIZE);

  size_t frameLen = mergeBytes(packetHeader[2], packetHeader[3]);
  uint16_t expectedNext =
      (parent_.core_.currentReadPointer_ + (frameLen + (frameLen % 2))) %
      (CONFIG_ERXND + 1);
  uint16_t actualNext = mergeBytes(packetHeader[0], packetHeader[1]);

  if (expectedNext != actualNext) {
    DEBUG_ASSERT(false, "Corrupt Rx packet header.");
  }

  if (!!currentRxPacket_) {
    currentRxPacket_->frameLength = frameLen;
  }

  // Starts DMA transactions to read packet frame
  size_t transactionSize = frameLen + (frameLen % 2);

  parent_.core_.readBufferMemoryStart();
  parent_.spi_->enableTxDMA();
  parent_.spi_->enableRxDMA();
  parent_.dmaTx_.dma->enable();
  parent_.dmaRx_.dma->enable();

  uint8_t* rxDst =
      (!!currentRxPacket_ ? currentRxPacket_->frame : &devNullFrame_);
  bool rxDstInc = (!!currentRxPacket_ ? true : false);

  parent_.dmaTx_.dma->configureStream(
      parent_.dmaTx_.stream, parent_.dmaTx_.channel,
      DMA::Direction::MEM_TO_PERI, transactionSize, DMA::FIFOThreshold::DIRECT,
      false, DMA::Priority::HIGH, rxDst, DMA::Size::BYTE, rxDstInc,
      &parent_.spi_->getRaw()->DR, DMA::Size::BYTE, false, nullptr, nullptr);
  parent_.dmaRx_.dma->configureStream(
      parent_.dmaRx_.stream, parent_.dmaRx_.channel,
      DMA::Direction::PERI_TO_MEM, transactionSize, DMA::FIFOThreshold::DIRECT,
      false, DMA::Priority::VERY_HIGH, &parent_.spi_->getRaw()->DR,
      DMA::Size::BYTE, false, rxDst, DMA::Size::BYTE, rxDstInc,
      handleRxDMAEventWrapper, this);

  parent_.dmaTx_.dma->enableStream(parent_.dmaTx_.stream);
  parent_.dmaRx_.dma->enableStream(parent_.dmaRx_.stream);
}

void Receiver::fsmActionRxCleanup() {
  size_t bytesRead =
      currentRxPacket_->frameLength + (currentRxPacket_->frameLength % 2);
  parent_.core_.readBufferMemoryEnd(bytesRead);
  parent_.spi_->disableRxDMA();
  parent_.spi_->disableTxDMA();

  uint16_t* packetHeader =
      (!!currentRxPacket_ ? currentRxPacket_->header : devNullHeader_);

  uint16_t newERXRDPT = mergeBytes(packetHeader[0], packetHeader[1]);
  // ENC28J60 errata issue 14
  newERXRDPT = (newERXRDPT == CONFIG_ERXST ? CONFIG_ERXND : newERXRDPT - 1);

  parent_.core_.writeControlReg(
      ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTL, lowByte(newERXRDPT));
  parent_.core_.writeControlReg(ControlRegBank::BANK_0,
                                ControlRegAddress::ERXRDPTH,
                                highByte(newERXRDPT));
  parent_.core_.setETHRegBitField(ControlRegBank::BANK_0,
                                  ControlRegAddress::ECON2, ECON2_PKTDEC);

  if (!!currentRxPacket_) {
    parent_.rxBuffer.push(currentRxPacket_);

    parent_.stats.rxBytes += currentRxPacket_->frameLength;
    parent_.stats.rxPackets++;
  } else {
    parent_.stats.rxPacketsLostInDriver++;
  }

  parent_.postEvent((!!currentRxPacket_) ? Event::RX_NEW_PACKET
                                         : Event::RX_OVERFLOW);

  currentRxPacket_ = nullptr;

  fsm_.pushEvent(FSMEvent::NOW_ACTIVE);
}

void Receiver::fsmActionTxStartDMA(void) {
  parent_.txBuffer.pop(currentTxPacket_);

  parent_.core_.writeControlReg(
      ControlRegBank::BANK_0, ControlRegAddress::ETXSTL, lowByte(CONFIG_ETXST));
  parent_.core_.writeControlReg(ControlRegBank::BANK_0,
                                ControlRegAddress::ETXSTH,
                                highByte(CONFIG_ETXST));

  parent_.core_.writeControlReg(
      ControlRegBank::BANK_0, ControlRegAddress::EWRPTL, lowByte(CONFIG_ETXST));
  parent_.core_.writeControlReg(ControlRegBank::BANK_0,
                                ControlRegAddress::EWRPTH,
                                highByte(CONFIG_ETXST));

  uint16_t controlByte = 0b00000000;

  parent_.core_.writeBufferMemoryStart();
  parent_.spi_->transact(&controlByte, 1);

  parent_.spi_->enableTxDMA();
  parent_.spi_->enableRxDMA();
  parent_.dmaTx_.dma->enable();
  parent_.dmaRx_.dma->enable();

  parent_.dmaTx_.dma->configureStream(
      parent_.dmaTx_.stream, parent_.dmaTx_.channel,
      DMA::Direction::MEM_TO_PERI, currentTxPacket_->frameLength,
      DMA::FIFOThreshold::DIRECT, false, DMA::Priority::VERY_HIGH,
      currentTxPacket_->frame, DMA::Size::BYTE, true,
      &parent_.spi_->getRaw()->DR, DMA::Size::BYTE, false,
      handleTxDMAEventWrapper, this);
  parent_.dmaRx_.dma->configureStream(
      parent_.dmaRx_.stream, parent_.dmaRx_.channel,
      DMA::Direction::PERI_TO_MEM, currentTxPacket_->frameLength,
      DMA::FIFOThreshold::DIRECT, false, DMA::Priority::VERY_HIGH,
      &parent_.spi_->getRaw()->DR, DMA::Size::BYTE, false,
      currentTxPacket_->frame, DMA::Size::BYTE, true, nullptr, nullptr);

  parent_.dmaTx_.dma->enableStream(parent_.dmaTx_.stream);
  parent_.dmaRx_.dma->enableStream(parent_.dmaRx_.stream);
}

void Receiver::fsmActionTxCleanup(void) {
  parent_.core_.writeBufferMemoryEnd();

  uint16_t ETXND = CONFIG_ETXST + currentTxPacket_->frameLength;

  parent_.core_.writeControlReg(ControlRegBank::BANK_0,
                                ControlRegAddress::ETXNDL, lowByte(ETXND));
  parent_.core_.writeControlReg(ControlRegBank::BANK_0,
                                ControlRegAddress::ETXNDH, highByte(ETXND));

  parent_.core_.setETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                                  ControlRegAddress::ECON1, ECON1_TXRTS);

  parent_.packetBuffer_.free(currentTxPacket_);

  fsm_.pushEvent(FSMEvent::TX_NOT_DONE_YET);
}

void Receiver::fsmActionTxWait(void) {
  uint8_t EIR = parent_.core_.readETHReg(ControlRegBank::BANK_DONT_CARE,
                                         ControlRegAddress::EIR);

  if (BIT_IS_SET(EIR, EIR_TXIF)) {
    parent_.core_.clearETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                                      ControlRegAddress::EIR, EIR_TXIF);

    parent_.postEvent(Event::TX_DONE);
    fsm_.pushEvent(FSMEvent::TX_DONE);
    fsm_.pushEvent(FSMEvent::NOW_ACTIVE);
  } else {
    fsm_.pushEvent(FSMEvent::TX_NOT_DONE_YET);
  }
}

void Receiver::fsmActionDeactivate() {
  parent_.pinInt_.gpio->enableExternalInterrupt(parent_.pinInt_.pin);
}

/* static */ Receiver::FSM::Transition Receiver::fsmTransitions_[] = {
    // clang-format off
    
    // Activation
    {FSMState::IDLE,            FSMEvent::INTERRUPT,        &Receiver::fsmActionActivate,     FSMState::ACTIVE},
    {FSMState::IDLE,            FSMEvent::TX_REQUESTED,     &Receiver::fsmActionActivate,     FSMState::ACTIVE},
    
    {FSMState::ACTIVE,          FSMEvent::NOW_ACTIVE,       &Receiver::fsmActionCheckEIR,     FSMState::ACTIVE},
    
    // Rx path
    {FSMState::ACTIVE,          FSMEvent::RX_STARTED,       &Receiver::fsmActionRxStartDMA,   FSMState::RX_DMA_PENDING},
    {FSMState::RX_DMA_PENDING,  FSMEvent::RX_DMA_COMPLETE,  &Receiver::fsmActionRxCleanup,    FSMState::ACTIVE},
    
    // Tx path
    {FSMState::ACTIVE,          FSMEvent::TX_STARTED,       &Receiver::fsmActionTxStartDMA,   FSMState::TX_DMA_PENDING},
    {FSMState::TX_DMA_PENDING,  FSMEvent::TX_DMA_COMPLETE,  &Receiver::fsmActionTxCleanup,    FSMState::TX_WAITING},
    {FSMState::TX_WAITING,      FSMEvent::TX_NOT_DONE_YET,  &Receiver::fsmActionTxWait,       FSMState::TX_WAITING},
    {FSMState::TX_WAITING,      FSMEvent::TX_DONE,          nullptr,                          FSMState::ACTIVE},
    
    // Deactivation
    {FSMState::RX_DMA_PENDING,  FSMEvent::RX_ALL_DONE,      &Receiver::fsmActionDeactivate,   FSMState::IDLE},
    
    FSM::TransitionTerminator,
    // clang-format on
};
}; // namespace enc28j60
