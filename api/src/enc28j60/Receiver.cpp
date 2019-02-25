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

void Receiver::enable() {
  parent_.dmaTx_.dma->configureStream(
      parent_.dmaTx_.stream, parent_.dmaTx_.channel,
      DMA::Direction::MEM_TO_PERI, 0, DMA::FIFOThreshold::DIRECT, false,
      DMA::Priority::VERY_HIGH, nullptr, DMA::Size::BYTE, true,
      &parent_.spi_->getRaw()->DR, DMA::Size::BYTE, false,
      handleTxDMAEventWrapper, this);
  parent_.dmaRx_.dma->configureStream(
      parent_.dmaRx_.stream, parent_.dmaRx_.channel,
      DMA::Direction::PERI_TO_MEM, 0, DMA::FIFOThreshold::DIRECT, false,
      DMA::Priority::VERY_HIGH, &parent_.spi_->getRaw()->DR, DMA::Size::BYTE,
      false, nullptr, DMA::Size::BYTE, true, nullptr, nullptr);
}

void Receiver::requestTx() {
  if (txRequestPending_) {
    return;
  }

  txRequestPending_ = true;
  fsm_.pushEvent(FSMEvent::TX_REQUESTED);
}

void Receiver::handleTxDMAEvent(DMA::StreamEvent event) {
  if (fsm_.state == FSMState::RX_DMA_PENDING) {
    switch (event.type) {
    case DMA::StreamEventType::TRANSFER_COMPLETE:
      fsm_.pushEvent(Receiver::FSM::Event::RX_DMA_COMPLETE);
      break;
    }
  } else if (fsm_.state == FSMState::TX_DMA_PENDING) {
    switch (event.type) {
    case DMA::StreamEventType::TRANSFER_COMPLETE:
      fsm_.pushEvent(Receiver::FSM::Event::TX_DMA_COMPLETE);
      break;
    }
  } else {
    DEBUG_FAIL("DMA event received unexpectedly.");
  }
}

void handleTxDMAEventWrapper(DMA::StreamEvent event, void* context) {
  static_cast<Receiver*>(context)->handleTxDMAEvent(event);
}

////////////////////////////////////////////////////////////////////////////////
// FSM goes round and round...
////////////////////////////////////////////////////////////////////////////////

void Receiver::fsmActionActivate() {
  // We handle interrupt one at a time.
  // Inspiration taken from the Linux kernel driver.
  parent_.core_.clearETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                                    ControlRegAddress::EIE, EIE_INTIE);
}

void Receiver::fsmActionDispatch() {
  uint8_t eir = parent_.core_.readETHReg(ControlRegBank::BANK_DONT_CARE,
                                         ControlRegAddress::EIR);

  if (BIT_IS_SET(eir, EIR_RXERIF)) {
    parent_.postEvent(Event::RX_CHIP_OVERFLOW);
    parent_.core_.clearETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                                      ControlRegAddress::EIR, EIR_RXERIF);
  }

  if (BIT_IS_SET(eir, EIR_TXIF)) {
    DEBUG_ASSERT(!!currentTxPacket_, "TXIF set but we're not in Tx.");

    parent_.core_.clearETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                                      ControlRegAddress::EIR, EIR_TXIF);
    parent_.packetBuffer_.free(currentTxPacket_);
    currentTxPacket_ = nullptr;
    parent_.postEvent(Event::TX_DONE);
  }

  uint8_t packetCount = parent_.core_.readETHReg(ControlRegBank::BANK_1,
                                                 ControlRegAddress::EPKTCNT);
  if (packetCount > parent_.stats.maxPKTCNT) {
    parent_.stats.maxPKTCNT = packetCount;
  }

  if (!currentTxPacket_ && !parent_.txBuffer.empty()) {
    // We prioritize Tx over Rx.
    fsm_.pushEvent(FSMEvent::TX_STARTED);
  } else if (packetCount > 0) {
    fsm_.pushEvent(FSMEvent::RX_STARTED);
  } else {
    // We have nothing to do. If we're currently Tx-ing, we need to wait for its
    // completion. Otherwise we're off!

    if (!!currentTxPacket_) {
      fsm_.pushEvent(FSMEvent::SLACK_OFF);
    } else {
      txRequestPending_ = false;
      fsm_.pushEvent(FSMEvent::DEACTIVATE);
    }
  }
}

void Receiver::fsmActionRxStartDMA() {
  // Allocate the new RX packet
  currentRxPacket_ = parent_.packetBuffer_.allocate();

  // Reader packet header and calculate frame size
  uint16_t* packetHeader =
      (!!currentRxPacket_ ? currentRxPacket_->header : devNullHeader_);

  parent_.core_.readBufferMemory(packetHeader, PACKET_HEADER_SIZE);

  size_t frameLen = mergeBytes(packetHeader[2], packetHeader[3]);
  uint16_t expectedNext =
      (parent_.core_.currentReadPointer_ + (frameLen + (frameLen % 2))) %
      (CONFIG_ERXND + 1);
  uint16_t actualNext = mergeBytes(packetHeader[0], packetHeader[1]);

  if (expectedNext != actualNext) {
    fsm_.pushEvent(FSMEvent::RX_BAD_HEADER);
    return;
  }

  if (!!currentRxPacket_) {
    currentRxPacket_->frameLength = frameLen;
  }

  // Starts DMA transactions to read packet frame
  currentRxDMATransactionSize_ = frameLen + (frameLen % 2);

  parent_.core_.readBufferMemoryStart();
  parent_.spi_->enableRxDMA();

  uint8_t* rxDst =
      (!!currentRxPacket_ ? currentRxPacket_->frame : &devNullFrame_);
  bool rxDstInc = (!!currentRxPacket_ ? true : false);

  parent_.dmaTx_.dma->reconfigureMemory(parent_.dmaTx_.stream,
                                        currentRxDMATransactionSize_, rxDst,
                                        DMA::Size::BYTE, rxDstInc);
  parent_.dmaRx_.dma->reconfigureMemory(parent_.dmaRx_.stream,
                                        currentRxDMATransactionSize_, rxDst,
                                        DMA::Size::BYTE, rxDstInc);

  parent_.dmaTx_.dma->enableStream(parent_.dmaTx_.stream);
  parent_.dmaRx_.dma->enableStream(parent_.dmaRx_.stream);

  parent_.spi_->enableTxDMA();
}

void Receiver::fsmActionRxReset() {
  if (!!currentRxPacket_) {
    parent_.packetBuffer_.free(currentRxPacket_);
  }

  parent_.resetRx();

  parent_.stats.rxResets++;
}

void Receiver::fsmActionRxCleanup() {
  parent_.core_.readBufferMemoryEnd(currentRxDMATransactionSize_);

  parent_.spi_->waitUntilNotBusy();
  auto numberOfData =
      parent_.dmaRx_.dma->getNumberOfData(parent_.dmaRx_.stream);
  auto success = true;
  if (numberOfData != 0) {
    DEBUG_ASSERT(BIT_IS_SET(parent_.spi_->getRaw()->SR, SPI_SR_OVR),
                 "Rx DMA incomplete without SPI overrun.");

    success = false;

    parent_.dmaRx_.dma->disableStream(parent_.dmaRx_.stream);
    FORCE_READ(parent_.spi_->getRaw()->DR);
    FORCE_READ(parent_.spi_->getRaw()->SR);
  }

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
    if (success) {
      parent_.rxBuffer.push(currentRxPacket_);

      parent_.stats.rxBytes += currentRxPacket_->frameLength;
      parent_.stats.rxPackets++;
    } else {
      parent_.stats.rxPacketsFailed++;
    }
  } else {
    parent_.stats.rxPacketsLostInDriver++;
  }

  parent_.postEvent((!!currentRxPacket_) ? Event::RX_NEW_PACKET
                                         : Event::RX_OVERFLOW);

  currentRxPacket_ = nullptr;
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

  parent_.spi_->enableRxDMA();

  parent_.dmaTx_.dma->reconfigureMemory(
      parent_.dmaTx_.stream, currentTxPacket_->frameLength,
      currentTxPacket_->frame, DMA::Size::BYTE, true);
  parent_.dmaRx_.dma->reconfigureMemory(
      parent_.dmaRx_.stream, currentTxPacket_->frameLength,
      currentTxPacket_->frame, DMA::Size::BYTE, true);

  parent_.dmaTx_.dma->enableStream(parent_.dmaTx_.stream);
  parent_.dmaRx_.dma->enableStream(parent_.dmaRx_.stream);

  parent_.spi_->enableTxDMA();
}

void Receiver::fsmActionTxCleanup(void) {
  parent_.core_.writeBufferMemoryEnd();

  parent_.spi_->waitUntilNotBusy();
  auto numberOfData =
      parent_.dmaRx_.dma->getNumberOfData(parent_.dmaRx_.stream);
  if (numberOfData != 0) {
    parent_.stats.txPacketsFailed++;

    parent_.dmaRx_.dma->disableStream(parent_.dmaRx_.stream);

    // If current DMA is unsuccessful, we need to clear out set flags.
    FORCE_READ(parent_.spi_->getRaw()->DR);
  }

  parent_.spi_->disableRxDMA();
  parent_.spi_->disableTxDMA();

  uint16_t ETXND = CONFIG_ETXST + currentTxPacket_->frameLength;

  parent_.core_.writeControlReg(ControlRegBank::BANK_0,
                                ControlRegAddress::ETXNDL, lowByte(ETXND));
  parent_.core_.writeControlReg(ControlRegBank::BANK_0,
                                ControlRegAddress::ETXNDH, highByte(ETXND));

  parent_.core_.setETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                                  ControlRegAddress::ECON1, ECON1_TXRTS);
}

void Receiver::fsmActionDeactivate() {
  parent_.core_.setETHRegBitField(ControlRegBank::BANK_DONT_CARE,
                                  ControlRegAddress::EIE, EIE_INTIE);
}

/* static */ Receiver::FSM::Transition Receiver::fsmTransitions_[] = {
    // clang-format off

    // Activation
    {FSMState::IDLE,            FSMEvent::INTERRUPT,        &Receiver::fsmActionActivate,     FSMState::ACTIVE},
    {FSMState::IDLE,            FSMEvent::TX_REQUESTED,     &Receiver::fsmActionActivate,     FSMState::ACTIVE},

    // Rx path
    {FSMState::ACTIVE,          FSMEvent::RX_STARTED,       &Receiver::fsmActionRxStartDMA,   FSMState::RX_DMA_PENDING},
    {FSMState::RX_DMA_PENDING,  FSMEvent::RX_BAD_HEADER,    &Receiver::fsmActionRxReset,      FSMState::ACTIVE},
    {FSMState::RX_DMA_PENDING,  FSMEvent::RX_DMA_COMPLETE,  &Receiver::fsmActionRxCleanup,    FSMState::ACTIVE},

    // Tx path
    {FSMState::ACTIVE,          FSMEvent::TX_STARTED,       &Receiver::fsmActionTxStartDMA,   FSMState::TX_DMA_PENDING},
    {FSMState::TX_DMA_PENDING,  FSMEvent::TX_DMA_COMPLETE,  &Receiver::fsmActionTxCleanup,    FSMState::ACTIVE},

    // Slack off path
    {FSMState::ACTIVE,          FSMEvent::SLACK_OFF,        nullptr,                          FSMState::ACTIVE},

    // Deactivation
    {FSMState::ACTIVE,          FSMEvent::DEACTIVATE,       &Receiver::fsmActionDeactivate,   FSMState::IDLE},

    FSM::TransitionTerminator,
    // clang-format on
};

/* static */ Receiver::FSM::StateAction Receiver::fsmStateActions_[] = {
    // clang-format off
    {FSMState::ACTIVE, &Receiver::fsmActionDispatch},

    FSM::StateActionTerminator,
    // clang-format on
};

}; // namespace enc28j60
