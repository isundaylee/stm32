#include <Utils.h>

#include <printf.h>

#include <ADC.h>
#include <Clock.h>
#include <DMA.h>
#include <Flash.h>
#include <GPIO.h>
#include <RingBuffer.h>
#include <SPI.h>
#include <Timer.h>
#include <USART.h>

extern "C" {
#include <pico_device.h>
#include <pico_icmp4.h>
#include <pico_ipv4.h>
#include <pico_stack.h>
}

#include <enc28j60/ENC28J60.h>

#define DUMP_PACKET_HEADERS 0
#define DUMP_STATS 1
#define PRINT_PACKET_INDICATOR 0

static const uint8_t MAC[] = {0xB0, 0xD5, 0x08, 0xA5, 0x38, 0x42};

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

static volatile RingBuffer<Event, 16> events;
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

static void initializeEthernet(uint8_t const* macAddress) {
  eth.enable(&SPI_2, GPIO::Pin{&GPIO_B, 12}, GPIO::Pin{&GPIO_C, 9},
             DMA::Channel{&DMA_1, 4, 0}, DMA::Channel{&DMA_1, 3, 0},
             enc28j60::Mode::FULL_DUPLEX, macAddress, handleEthernetEvent,
             nullptr);

  printf("Waiting for link");
  while (!eth.linkIsUp()) {
    printf(".");
    DELAY(1000000);
  }
  printf("\r\n");
  printf("Link is up!\r\n");

  eth.enableRx();

  Timer_2.enable(6000, Clock::getAPB1TimerFrequency() / 6000,
                 handleTimerInterrupt);
}

static void processEvents() {
  FORCE_READ(state);

  while (!events.empty()) {
    Event event{};
    events.pop(event);

    switch (event) {
    case Event::ETHERNET_RX_NEW_PACKET: {
      // Incoming packets are handled in picoPoll.
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
      static size_t totalRxResets = 0;
      static size_t totalRxPacketsFailed = 0;
      static size_t totalTxPacketsFailed = 0;

#if DUMP_STATS
      totalRxResets += eth.stats.rxResets;
      totalRxPacketsFailed += eth.stats.rxPacketsFailed;
      totalTxPacketsFailed += eth.stats.txPacketsFailed;

      printf("Rx = %5d / %7d B / %5d Kbps / TF %2d / L %5d, Tx = TF %2d, "
             "MaxPKTCNT = %3d, "
             "RxResets = %3d\r\n",
             eth.stats.rxPackets, eth.stats.rxBytes,
             (eth.stats.rxBytes * 8) >> 10, totalRxPacketsFailed,
             eth.stats.rxPacketsLostInDriver, totalTxPacketsFailed,
             eth.stats.maxPKTCNT, totalRxResets);
#endif

      eth.stats.reset();
      break;
    }
    }
  }
}

static int picoSend(struct pico_device* dev, void* buf, int len) {
  FORCE_READ(dev);

  auto packet = eth.allocatePacket();
  if (!packet) {
    return 0;
  }

  memcpy(packet->frame, buf, len);
  packet->frameLength = len;
  eth.transmit(packet);

  return len;
}

static int picoPoll(struct pico_device* dev, int loop_score) {
  while (loop_score > 0) {
    enc28j60::Packet* packet;
    if (!eth.rxBuffer.pop(packet)) {
      break;
    }

    // "- 4" to get rid of the CRC.
    pico_stack_recv(dev, packet->frame, packet->frameLength - 4);
    eth.freePacket(packet);
    loop_score--;
  }

  return loop_score;
}

struct pico_device* picoCreateDevice(char const* name,
                                     uint8_t const* macAddress) {
  struct pico_device* dev = reinterpret_cast<struct pico_device*>(
      PICO_ZALLOC(sizeof(struct pico_device)));
  if (!dev) {
    DEBUG_PRINT("Failed to allocate pico_device.");
    return NULL;
  }

  initializeEthernet(macAddress);

  dev->send = picoSend;
  dev->poll = picoPoll;

  if (pico_device_init(dev, name, macAddress) != 0) {
    DEBUG_PRINT("pico_device_init() failed.");
    PICO_FREE(dev);
    return NULL;
  }

  return dev;
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  Flash::setLatency(8);
  Clock::configureAPB1Prescaler(Clock::APBPrescaler::DIV_4);
  Clock::configurePLL(8, 128);
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
                        SPI::DataFrameFormat::BYTE, SPI::BaudRate::PCLK_OVER_2,
                        SPI::NSSMode::MANUAL);
  SPI_2.enable();

  DMA_1.enable();

  GPIO_C.enable();
  GPIO_C.setMode(7, GPIO::PinMode::OUTPUT);
  GPIO_C.set(7);
  GPIO_C.setMode(8, GPIO::PinMode::OUTPUT);
  GPIO_C.set(8);
  GPIO_C.setMode(9, GPIO::PinMode::INPUT);

  printf("Hello, picotcp!\r\n");

  if (pico_stack_init() != 0) {
    DEBUG_FAIL("pico_stack_init() failed.");
  }

  auto dev = picoCreateDevice("eth0", MAC);

  struct pico_ip4 ip, netmask;
  pico_string_to_ipv4("10.0.88.206", &ip.addr);
  pico_string_to_ipv4("255.255.255.0", &netmask.addr);
  pico_ipv4_link_add(dev, ip, netmask);

  while (true) {
    eth.process();
    processEvents();
    pico_stack_tick();
  }
}
