#pragma once

#include <stddef.h>
#include <stdint.h>

#include <DMA.h>

#include <EmbeddedFSM.h>
#include <FreeListBuffer.h>

#include "enc28j60/Consts.h"

namespace enc28j60 {

void handleTxDMAEventWrapper(DMA::StreamEvent event, void* context);
void handleRxDMAEventWrapper(DMA::StreamEvent event, void* context);

class ENC28J60;

class Receiver {
private:
  ENC28J60& parent_;

  // Rx packet buffer
  Packet* currentTxPacket_ = nullptr;
  Packet* currentRxPacket_ = nullptr;
  size_t currentRxDMATransactionSize_;
  bool currentRxDMASuccess_;
  uint8_t devNullFrame_;
  uint16_t devNullHeader_[PACKET_HEADER_SIZE];

  void handleTxDMAEvent(DMA::StreamEvent event);
  friend void handleTxDMAEventWrapper(DMA::StreamEvent event, void* context);

  // State machine
  enum class FSMEvent {
    INTERRUPT,
    TX_REQUESTED,

    NOW_ACTIVE,

    RX_STARTED,
    RX_BAD_HEADER,
    RX_DMA_COMPLETE,
    RX_ALL_DONE,

    TX_STARTED,
    TX_DMA_COMPLETE,
  };

  enum class FSMState {
    IDLE,
    ACTIVE,

    RX_DMA_PENDING,

    TX_DMA_PENDING,
  };

  using FSM = EmbeddedFSM<Receiver, FSMState, FSMEvent, false>;
  FSM fsm_;

  void fsmActionActivate(void);

  void fsmActionCheckEIR(void);

  void fsmActionRxStartDMA(void);
  void fsmActionRxReset(void);
  void fsmActionRxCleanup(void);

  void fsmActionTxStartDMA(void);
  void fsmActionTxCleanup(void);

  void fsmActionDeactivate(void);

  static FSM::Transition fsmTransitions_[];

  Receiver(ENC28J60& parent)
      : parent_(parent), fsm_(FSM::State::IDLE, *this, fsmTransitions_) {}

  void enable();

  friend class ENC28J60;
};

} // namespace enc28j60
