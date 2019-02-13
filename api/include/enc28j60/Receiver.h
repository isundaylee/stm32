#pragma once

#include <stddef.h>
#include <stdint.h>

#include <DMA.h>

#include <EmbeddedFSM.h>
#include <FreeListBuffer.h>

#include "enc28j60/Consts.h"

namespace enc28j60 {

void handleRxDMAEventWrapper(DMA::StreamEvent event, void* context);

class ENC28J60;

class Receiver {
private:
  ENC28J60& parent_;

  // Rx packet buffer
  FreeListBuffer<Packet, RX_PACKET_BUFFER_SIZE> rxPacketBuffer_;
  Packet* currentRxPacket_ = nullptr;
  uint8_t devNullFrame_;
  uint16_t devNullHeader_[PACKET_HEADER_SIZE];

  // FSM helper functions
  bool receivePacketHeader();

  void handleRxDMAEvent(DMA::StreamEvent event);
  friend void handleRxDMAEventWrapper(DMA::StreamEvent event, void* context);

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

  using FSM = EmbeddedFSM<Receiver, FSMState, FSMEvent>;
  FSM fsm_;

  void fsmActionCheckEIR(void);
  void fsmActionRxCleanup(void);
  void fsmActionEnableInt(void);

  static FSM::Transition fsmTransitions_[];

  Receiver(ENC28J60& parent)
      : parent_(parent), fsm_(FSM::State::IDLE, *this, fsmTransitions_) {}

  friend class ENC28J60;
};

} // namespace enc28j60
