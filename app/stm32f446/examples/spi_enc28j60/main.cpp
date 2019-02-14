#include <Utils.h>

#include <Clock.h>
#include <DMA.h>
#include <Flash.h>
#include <GPIO.h>
#include <SPI.h>
#include <Timer.h>
#include <USART.h>

#include <RingBuffer.h>

#include <enc28j60/ENC28J60.h>

#define DUMP_PACKET_HEADERS 0
#define DUMP_STATS 1
#define PRINT_PACKET_INDICATOR 0

enum class Event {
  ETHERNET_RX_NEW_PACKET,
  ETHERNET_RX_OVERFLOW,
  ETHERNET_RX_CHIP_OVERFLOW,
  TIMER_INTERRUPT,
};

enum class State {
  IDLE,
};

enc28j60::ENC28J60 eth;

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

void handleEthernetEvent(enc28j60::Event event, void*) {
  switch (event) {
  case enc28j60::Event::RX_NEW_PACKET: {
    events.push(Event::ETHERNET_RX_NEW_PACKET);
    break;
  }

  case enc28j60::Event::RX_OVERFLOW: {
    events.push(Event::ETHERNET_RX_OVERFLOW);
    break;
  }

  case enc28j60::Event::RX_CHIP_OVERFLOW: {
    events.push(Event::ETHERNET_RX_CHIP_OVERFLOW);
    break;
  }
  }
}

void handleTimerInterrupt() { events.push(Event::TIMER_INTERRUPT); }

static void initializeEthernet() {
  eth.enable(&SPI_2, GPIO::Pin{&GPIO_B, 12}, GPIO::Pin{&GPIO_C, 9},
             DMA::Channel{&DMA_1, 4, 0}, DMA::Channel{&DMA_1, 3, 0},
             enc28j60::Mode::FULL_DUPLEX, handleEthernetEvent, nullptr);

  USART_1.write("Waiting for link");
  while (!eth.linkIsUp()) {
    USART_1.write(".");
    DELAY(1000000);
  }
  USART_1.write("\r\n");
  USART_1.write("Link is up!\r\n");

  eth.enableRx();

#if DUMP_STATS
  Timer_2.enable(6000, 16000, handleTimerInterrupt);
#endif
}

static void processEthernetRxPackets() {
  while (!eth.rxBuffer.empty()) {
    enc28j60::Packet* packet{};
    eth.rxBuffer.pop(packet);

#if PRINT_PACKET_INDICATOR
    USART_1.write(".");
#endif

#if DUMP_PACKET_HEADERS
    dumpPacketHeader(packet);
#endif

    eth.freePacket(packet);
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

    case Event::ETHERNET_RX_CHIP_OVERFLOW: {
#if PRINT_PACKET_INDICATOR
      USART_1.write("C");
#endif
      break;
    }

    case Event::TIMER_INTERRUPT: {
      USART_1.write("RxBytes = ");
      USART_1.write(DecString(eth.stats.rxBytes, 8));
      USART_1.write(", RxPackets = ");
      USART_1.write(DecString(eth.stats.rxPackets, 5));
      USART_1.write(", RxPacketsLostInDriver = ");
      USART_1.write(DecString(eth.stats.rxPacketsLostInDriver, 5));
      USART_1.write(", MaxPKTCNT = ");
      USART_1.write(DecString(eth.stats.maxPKTCNT, 3));
      USART_1.write(", RxKbps = ");
      USART_1.write(DecString((eth.stats.rxBytes * 8) >> 10, 2));
      USART_1.write("\r\n");

      eth.stats.reset();

      break;
    }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  Flash::setLatency(9);
  Clock::configureAPB1Prescaler(Clock::APBPrescaler::DIV_2);
  Clock::configurePLL(8, 96);
  Clock::switchSysclk(Clock::SysclkSource::PLL);

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
                        SPI::DataFrameFormat::BYTE, SPI::BaudRate::PCLK_OVER_4,
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
