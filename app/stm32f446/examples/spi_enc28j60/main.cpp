#include <Utils.h>

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>
#include <USART.h>

#include <RingBuffer.h>

#include "ENC28J60.h"

#define DUMP_PACKET_HEADERS 0
#define PRINT_PACKET_INDICATOR 1

enum class Event {
  ETHERNET_RX_NEW_PACKET,
  ETHERNET_RX_OVERFLOW,
};

enum class State {
  IDLE,
};

ENC28J60 eth;

static RingBuffer<Event, 16> events;
static auto state = State::IDLE;

template <typename T> T min(T a, T b) { return (a < b ? a : b); }

#if DUMP_PACKET_HEADERS
static void dumpPacketHeader(ENC28J60::Packet* packet) {
  USART_1.write("[");
  for (size_t i = 0; i < 6; i++) {
    USART_1.write(HexString(packet->header[i], 2));
    if (i != 5) {
      USART_1.write(" ");
    }
  }
  USART_1.write("]");

  for (size_t i = 0; i < min<size_t>(packet->frameLength, 12); i++) {
    USART_1.write(" ");
    USART_1.write(HexString(packet->frame[i], 2));
  }
  USART_1.write("\r\n");
}
#endif

void handleEthernetEvent(ENC28J60::Event event, void*) {
  switch (event) {
  case ENC28J60::Event::RX_NEW_PACKET: {
    events.push(Event::ETHERNET_RX_NEW_PACKET);
    break;
  }

  case ENC28J60::Event::RX_OVERFLOW: {
    events.push(Event::ETHERNET_RX_OVERFLOW);
    break;
  }
  }
}

static void initializeEthernet() {
  eth.enable(&SPI_2, &GPIO_B, 12, &GPIO_C, 9, &DMA_1, 4, 0, &DMA_1, 3, 0,
             ENC28J60::Mode::FULL_DUPLEX, handleEthernetEvent, nullptr);

  USART_1.write("Waiting for link");
  while (!eth.linkIsUp()) {
    USART_1.write(".");
    DELAY(1000000);
  }
  USART_1.write("\r\n");
  USART_1.write("Link is up!\r\n");

  eth.enableRx();
}

static void processEthernetRxPackets() {
  while (!eth.rxBuffer.empty()) {
    ENC28J60::Packet* packet{};
    eth.rxBuffer.pop(packet);

#if PRINT_PACKET_INDICATOR
    USART_1.write(".");
#endif

#if DUMP_PACKET_HEADERS
    dumpPacketHeader(packet);
#endif

    eth.freeRxPacket(packet);
  }
}

static void processEvents() {
  FORCE_READ(state);

  while (!events.empty()) {
    Event event{};
    events.pop(event);

    switch (event) {
    case Event::ETHERNET_RX_NEW_PACKET: {
      processEthernetRxPackets();
      break;
    }

    case Event::ETHERNET_RX_OVERFLOW: {
#if PRINT_PACKET_INDICATOR
      USART_1.write("O");
#endif
      break;
    }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  // Sets up USART 1
  GPIO_A.enable();
  GPIO_A.setMode(9, GPIO::PinMode::ALTERNATE, 7);  // TX
  GPIO_A.setMode(10, GPIO::PinMode::ALTERNATE, 7); // RX
  USART_1.enable(115200);

  // Sets up SPI 2
  GPIO_B.enable();
  GPIO_B.setMode(12, GPIO::PinMode::OUTPUT); // NSS / LOAD
  GPIO_B.set(12);
  GPIO_B.setMode(13, GPIO::PinMode::ALTERNATE, 5); // SCK
  GPIO_B.setMode(14, GPIO::PinMode::ALTERNATE, 5); // MISO
  GPIO_B.setMode(15, GPIO::PinMode::ALTERNATE, 5); // MOSI
  SPI_2.configureMaster(SPI::ClockPolarity::IDLE_LOW,
                        SPI::ClockPhase::SAMPLE_ON_FIRST_EDGE,
                        SPI::DataFrameFormat::BYTE, SPI::BaudRate::PCLK_OVER_2,
                        SPI::NSSMode::MANUAL);
  SPI_2.enable();

  GPIO_C.enable();
  GPIO_C.setMode(7, GPIO::PinMode::OUTPUT);
  GPIO_C.set(7);
  GPIO_C.setMode(8, GPIO::PinMode::OUTPUT);
  GPIO_C.set(8);
  GPIO_C.setMode(9, GPIO::PinMode::INPUT);

  USART_1.write("Hello, SPI ENC28J60!\r\n");

  initializeEthernet();

  while (true) {
    eth.process();
    processEvents();
  }
}
