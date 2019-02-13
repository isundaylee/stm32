#pragma once

#include "enc28j60/Consts.h"
#include "enc28j60/Core.h"
#include "enc28j60/Receiver.h"

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>

#include <EmbeddedFSM.h>
#include <RingBuffer.h>

namespace enc28j60 {

void handleInterruptWrapper(void* context);

enum Event {
  RX_NEW_PACKET,
  RX_OVERFLOW,
  RX_CHIP_OVERFLOW,
};

using EventHandler = void (*)(Event event, void* context);

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
  DMA::Channel dmaTx_;
  DMA::Channel dmaRx_;

  // Configuration
  Mode mode_;

  EventHandler eventHandler_;
  void* eventHandlerContext_;

  Core core_;
  Receiver receiver_;

  // Initialization routines
  void initializeETH();
  void initializeMAC();
  void initializePHY();

  // Interrupt handlers
  void handleInterrupt();
  friend void handleInterruptWrapper(void* context);

  // Send the lovely event to our dear user's event handler
  void postEvent(Event event);

public:
  RingBuffer<Packet*, RX_PACKET_BUFFER_SIZE> rxBuffer;

  Stats stats;

  // High-level interface
  ENC28J60() : core_{}, receiver_(*this) {}

  void enable(SPI* spi, GPIO::Pin pinCS, GPIO::Pin pinInt, DMA::Channel dmaTx,
              DMA::Channel dmaRx, Mode mode, EventHandler eventHandler,
              void* eventHandlerContext);
  void enableRx();

  void process();

  bool linkIsUp();

  void freeRxPacket(Packet* packet);

  friend class Receiver;
};

} // namespace enc28j60