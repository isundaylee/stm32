#include <Utils.h>

#include <printf.h>

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
#define DUMP_STATS 0
#define PRINT_PACKET_INDICATOR 1

enum class Event {
  ETHERNET_RX_NEW_PACKET,
  ETHERNET_RX_OVERFLOW,
  ETHERNET_RX_CHIP_OVERFLOW,
  ETHERNET_TX_DONE,
  ETHERNET_TX_ERROR,
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
static void dumpPacketHeader(enc28j60::Packet* packet) {
  printf("[");
  for (size_t i = 0; i < 6; i++) {
    printf("0x%02x", packet->header[i]);
    if (i != 5) {
      printf(" ");
    }
  }
  printf("]");

  for (size_t i = 0; i < min<size_t>(packet->frameLength, 12); i++) {
    printf(" 0x%02x", packet->frame[i]);
  }
  printf("\r\n");
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

  case enc28j60::Event::TX_DONE: {
    events.push(Event::ETHERNET_TX_DONE);
    break;
  }

  case enc28j60::Event::TX_ERROR: {
    events.push(Event::ETHERNET_TX_ERROR);
    break;
  }
  }
}

void handleTimerInterrupt() { events.push(Event::TIMER_INTERRUPT); }

static void initializeEthernet() {
  eth.enable(&SPI_2, GPIO::Pin{&GPIO_B, 12}, GPIO::Pin{&GPIO_C, 9},
             DMA::Channel{&DMA_1, 4, 0}, DMA::Channel{&DMA_1, 3, 0},
             enc28j60::Mode::FULL_DUPLEX, handleEthernetEvent, nullptr);

  printf("Waiting for link");
  while (!eth.linkIsUp()) {
    printf(".");
    DELAY(1000000);
  }
  printf("\r\n");
  printf("Link is up!\r\n");

  eth.enableRx();

  Timer_2.enable(6000, 16000, handleTimerInterrupt);
}

static void processEthernetRxPackets() {
  while (!eth.rxBuffer.empty()) {
    enc28j60::Packet* packet{};
    eth.rxBuffer.pop(packet);

#if PRINT_PACKET_INDICATOR
    printf(".");
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
      printf("O");
#endif
      break;
    }

    case Event::ETHERNET_RX_CHIP_OVERFLOW: {
#if PRINT_PACKET_INDICATOR
      printf("C");
#endif
      break;
    }

    case Event::ETHERNET_TX_DONE: {
#if PRINT_PACKET_INDICATOR
      printf("D");
#endif
      break;
    }

    case Event::ETHERNET_TX_ERROR: {
#if PRINT_PACKET_INDICATOR
      printf("E");
#endif
      break;
    }

    case Event::TIMER_INTERRUPT: {
#if DUMP_STATS
      printf("RxBytes = %8d, RxPackets = %5d, RxPacketsLostInDriver = %5d, "
             "MaxPKTCNT = %3d, RxKbps = %2d\r\n",
             eth.stats.rxBytes, eth.stats.rxPackets,
             eth.stats.rxPacketsLostInDriver, eth.stats.maxPKTCNT,
             (eth.stats.rxBytes * 8) >> 10);
#endif

      eth.stats.reset();

      // Fake an ICMP echo request packet
      static uint8_t data[] = {
          0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x11, 0x22, 0x33, 0x44, 0x55,
          0x66, 0x08, 0x00, 0x45, 0x00, 0x00, 0x1D, 0xAE, 0x3A, 0x00, 0x00,
          0x40, 0x01, 0x00, 0x00, 0x0A, 0x00, 0x00, 0xCE, 0x0A, 0x00, 0x00,
          0xFF, 0x08, 0x00, 0x28, 0x49, 0xCF, 0x95, 0x00, 0x21, 0x00,
      };

      for (size_t i = 0; i < 4; i++) {
        auto packet = eth.allocatePacket();
        packet->frameLength = sizeof(data);
        memcpy(packet->frame, data, sizeof(data));
        eth.transmit(packet);
      }

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

  printf("Hello, SPI ENC28J60!\r\n");

  initializeEthernet();

  while (true) {
    eth.process();
    processEvents();
  }
}
