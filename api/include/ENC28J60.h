#pragma once

#include "ENC28J60Consts.h"

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>

#include <FreeListBuffer.h>
#include <RingBuffer.h>

void enc28j60HandleInterruptWrapper(void* context);
void enc28j60HandleRxDMAEventWrapper(DMA::StreamEvent event, void* context);

namespace enc28j60 {

enum Event {
  RX_NEW_PACKET,
  RX_OVERFLOW,
  RX_CHIP_OVERFLOW,
};

using EventHandler = void (*)(Event event, void* context);

struct Packet {
  uint16_t header[PACKET_HEADER_SIZE] = {0};
  size_t frameLength = 0;
  uint8_t frame[PACKET_FRAME_SIZE] = {0};
};

class ENC28J60 {
private:
  enum class InternalEvent {
    INTERRUPT,
    RX_DMA_COMPLETE,
  };

  enum class State {
    IDLE,
    RX_FRAME_PENDING,
  };

  SPI* spi_;

  GPIO* gpioCS_;
  int pinCS_;

  GPIO* gpioInt_;
  int pinInt_;

  DMA* dmaTx_;
  int dmaStreamTx_;
  int dmaChannelTx_;

  DMA* dmaRx_;
  int dmaStreamRx_;
  int dmaChannelRx_;

  Mode mode_;

  EventHandler eventHandler_;
  void* eventHandlerContext_;

  // State and event processing
  RingBuffer<InternalEvent, 16> events_;
  State state_ = State::IDLE;

  static const size_t RX_PACKET_BUFFER_SIZE = 8;

  FreeListBuffer<Packet, RX_PACKET_BUFFER_SIZE> rxPacketBuffer_;
  Packet* currentRxPacket_ = nullptr;
  uint8_t devNullFrame_;
  uint16_t devNullHeader_[PACKET_HEADER_SIZE];

  uint8_t generateHeaderByte(Opcode opcode, ControlRegAddress addr);

  void selectControlRegBank(ControlRegBank bank);

  void initializeETH();
  void initializeMAC();
  void initializePHY();

  void handleInterrupt() { events_.push(InternalEvent::INTERRUPT); }
  void handleRxDMAEvent(DMA::StreamEvent event) {
    switch (event.type) {
    case DMA::StreamEventType::TRANSFER_COMPLETE:
      events_.push(InternalEvent::RX_DMA_COMPLETE);
      break;
    }
  }

  bool receivePacketHeader();
  void receivePacketCleanup();

  void postEvent(Event event);

  void transitionToIdle();
  void handleInterruptEvent();

public:
  RingBuffer<Packet*, RX_PACKET_BUFFER_SIZE> rxBuffer;

  ENC28J60() {}

  void enable(SPI* spi, GPIO* gpioCS, int pinCS, GPIO* gpioInt, int pinInt,
              DMA* dmaTx, int dmaStreamTx, int dmaChannelTx, DMA* dmaRx,
              int dmaStreamRx, int dmaChannelRx, Mode mode,
              EventHandler eventHandler, void* eventHandlerContext);

  void process();

  bool linkIsUp() {
    return BIT_IS_SET(readPHYReg(PHYRegAddress::PHSTAT1), PHSTAT1_LLSTAT);
  }

  void enableRx() {
    setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::ECON1,
                      ECON1_RXEN);
    setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::EIE,
                      EIE_INTIE | EIE_PKTIE | EIE_RXERIE);
  }

  void freeRxPacket(Packet* packet) { rxPacketBuffer_.free(packet); }

  // Low-level interface
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

  friend void enc28j60HandleInterruptWrapper(void* context);
  friend void enc28j60HandleRxDMAEventWrapper(DMA::StreamEvent event,
                                              void* context);
};

} // namespace enc28j60
