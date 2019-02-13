#pragma once

#include "enc28j60/Consts.h"
#include "enc28j60/Core.h"

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>

#include <EmbeddedFSM.h>
#include <FreeListBuffer.h>
#include <RingBuffer.h>

namespace enc28j60 {

void handleInterruptWrapper(void* context);
void handleRxDMAEventWrapper(DMA::StreamEvent event, void* context);

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

struct Stats {
  size_t rxPackets = 0;
  size_t rxBytes = 0;
  size_t rxPacketsLostInDriver = 0;

  uint8_t maxPKTCNT = 0;

  void reset() {
    rxPackets = 0;
    rxBytes = 0;
    rxPacketsLostInDriver = 0;
    maxPKTCNT = 0;
  }
};

class ENC28J60 {
private:
  // Peripheral dependencies
  SPI* spi_;
  GPIO::Pin pinCS_;
  GPIO::Pin pinInt_;

  DMA* dmaTx_;
  int dmaStreamTx_;
  int dmaChannelTx_;

  DMA* dmaRx_;
  int dmaStreamRx_;
  int dmaChannelRx_;

  // Configuration
  Mode mode_;

  EventHandler eventHandler_;
  void* eventHandlerContext_;

  Core core_;

  // Rx packet buffer
  static const size_t RX_PACKET_BUFFER_SIZE = 8;

  FreeListBuffer<Packet, RX_PACKET_BUFFER_SIZE> rxPacketBuffer_;
  Packet* currentRxPacket_ = nullptr;
  uint8_t devNullFrame_;
  uint16_t devNullHeader_[PACKET_HEADER_SIZE];

  // Initialization routines
  void initializeETH();
  void initializeMAC();
  void initializePHY();

  // Interrupt handlers
  void handleInterrupt();
  void handleRxDMAEvent(DMA::StreamEvent event);
  friend void handleInterruptWrapper(void* context);
  friend void handleRxDMAEventWrapper(DMA::StreamEvent event, void* context);

  // FSM helper functions
  bool receivePacketHeader();
  void postEvent(Event event);

private:
  // State machine
  enum class FSMEvent {
    INTERRUPT,
    RX_HEADER_READ,
    RX_DMA_COMPLETE,
    RX_ALL_DONE,
  };

  enum class FSMState {
    IDLE,
    EIR_CHECKED,
    RX_PENDING,
  };

  using FSM = EmbeddedFSM<ENC28J60, FSMState, FSMEvent>;
  FSM fsm_;

  void fsmActionCheckEIR(void);
  void fsmActionRxCleanup(void);
  void fsmActionEnableInt(void);

  static FSM::Transition fsmTransitions_[];

public:
  RingBuffer<Packet*, RX_PACKET_BUFFER_SIZE> rxBuffer;

  Stats stats;

  // High-level interface
  ENC28J60() : core_{}, fsm_(FSM::State::IDLE, *this, fsmTransitions_) {}

  void enable(SPI* spi, GPIO::Pin pinCS, GPIO::Pin pinInt, DMA* dmaTx,
              int dmaStreamTx, int dmaChannelTx, DMA* dmaRx, int dmaStreamRx,
              int dmaChannelRx, Mode mode, EventHandler eventHandler,
              void* eventHandlerContext);
  void enableRx();

  void process();

  bool linkIsUp();

  void freeRxPacket(Packet* packet);
};

} // namespace enc28j60
