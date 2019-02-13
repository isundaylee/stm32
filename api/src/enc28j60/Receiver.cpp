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
// FSM helper functions
////////////////////////////////////////////////////////////////////////////////

bool Receiver::receivePacketHeader() {
  uint8_t packetCount = parent_.core_.readETHReg(ControlRegBank::BANK_1,
                                                 ControlRegAddress::EPKTCNT);

  if (packetCount == 0) {
    return false;
  }

  if (packetCount > parent_.stats.maxPKTCNT) {
    parent_.stats.maxPKTCNT = packetCount;
  }

  // Allocate the new RX packet
  currentRxPacket_ = rxPacketBuffer_.allocate();

  // Reader packet header and calculate frame size
  uint16_t* packetHeader =
      (!!currentRxPacket_ ? currentRxPacket_->header : devNullHeader_);
  size_t frameLen;

  parent_.core_.readBufferMemory(packetHeader, PACKET_HEADER_SIZE);
  frameLen = static_cast<size_t>(packetHeader[2]) +
             (static_cast<size_t>(packetHeader[3]) << 8);

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
      false, DMA::Priority::VERY_HIGH, rxDst, DMA::Size::BYTE, rxDstInc,
      &SPI2->DR, DMA::Size::BYTE, false, nullptr, nullptr);
  parent_.dmaRx_.dma->configureStream(
      parent_.dmaRx_.stream, parent_.dmaRx_.channel,
      DMA::Direction::PERI_TO_MEM, transactionSize, DMA::FIFOThreshold::DIRECT,
      false, DMA::Priority::VERY_HIGH, &SPI2->DR, DMA::Size::BYTE, false, rxDst,
      DMA::Size::BYTE, rxDstInc, handleRxDMAEventWrapper, this);

  parent_.dmaTx_.dma->enableStream(parent_.dmaTx_.stream);
  parent_.dmaRx_.dma->enableStream(parent_.dmaRx_.stream);

  return true;
}

////////////////////////////////////////////////////////////////////////////////
// FSM goes round and round...
////////////////////////////////////////////////////////////////////////////////

void Receiver::fsmActionRxCleanup() {
  parent_.core_.readBufferMemoryEnd();
  SPI_2.disableRxDMA();
  SPI_2.disableTxDMA();

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

  fsm_.pushEvent(FSMEvent::INTERRUPT);
}

void Receiver::fsmActionCheckEIR() {
  uint8_t eir = parent_.core_.readETHReg(ControlRegBank::BANK_DONT_CARE,
                                         ControlRegAddress::EIR);

  if (BIT_IS_SET(eir, EIR_RXERIF)) {
    parent_.postEvent(Event::RX_CHIP_OVERFLOW);
    parent_.core_.clearETHRegBitField(ControlRegBank::BANK_DONT_CARE,
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

void Receiver::fsmActionEnableInt() {
  parent_.pinInt_.gpio->enableExternalInterrupt(parent_.pinInt_.pin);
}

/* static */ Receiver::FSM::Transition Receiver::fsmTransitions_[] = {
    // clang-format off
    {FSMState::IDLE,        FSMEvent::INTERRUPT,        &Receiver::fsmActionCheckEIR,   FSMState::EIR_CHECKED},
    {FSMState::EIR_CHECKED, FSMEvent::RX_HEADER_READ,   nullptr,                        FSMState::RX_PENDING},
    {FSMState::EIR_CHECKED, FSMEvent::RX_ALL_DONE,      &Receiver::fsmActionEnableInt,  FSMState::IDLE},
    {FSMState::RX_PENDING,  FSMEvent::RX_DMA_COMPLETE,  &Receiver::fsmActionRxCleanup,  FSMState::IDLE},
    FSM::TransitionTerminator,
    // clang-format on
};

}; // namespace enc28j60
