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

void handleTimerInterrupt(void*) { events.push(Event::TIMER_INTERRUPT); }

static void initializeEthernet() {
  eth.enable(&SPI_2, GPIO::Pin{&GPIO_B, 12}, GPIO::Pin{&GPIO_C, 9},
             DMA::Channel{&DMA_1, 4, 0}, DMA::Channel{&DMA_1, 3, 0},
             enc28j60::Mode::FULL_DUPLEX, MAC, handleEthernetEvent, nullptr);

  printf("Waiting for link");
  while (!eth.linkIsUp()) {
    printf(".");
    DELAY(1000000);
  }
  printf("\r\n");
  printf("Link is up!\r\n");

  eth.enableRx();

  Timer_2.enable(6000, Clock::getAPB1TimerFrequency() / 6000 / 30,
                 Timer::Action::PERIODIC, handleTimerInterrupt, nullptr);
}

uint16_t calculateChecksum(uint8_t const* data, size_t len) {
  uint32_t checksum = 0;

  while (len > 1) {
    uint32_t chunk =
        static_cast<uint32_t>(data[1]) + (static_cast<uint32_t>(data[0]) << 8);

    checksum += chunk;
    if ((checksum >> 16) != 0) {
      checksum = (checksum & 0xFFFF) + 1;
    }

    len -= 2;
    data += 2;
  }

  if (len == 1) {
    checksum += (static_cast<uint32_t>(data[0]) << 8);
  }

  while ((checksum >> 16) != 0) {
    checksum = (checksum & 0xFFFF) + (checksum >> 16);
  }

  return ~static_cast<uint16_t>(checksum);
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

    DEBUG_PIN_0_PULSE_LOW(1);

    uint8_t* f = packet->frame;

    // Check if the packet length is too short
    if (packet->frameLength < 42) {
      eth.freePacket(packet);
      continue;
    }

    // Check if the packet is for us
    if ((f[0] != MAC[0]) || (f[1] != MAC[1]) || (f[2] != MAC[2]) ||
        (f[3] != MAC[3]) || (f[4] != MAC[4]) || (f[5] != MAC[5])) {
      eth.freePacket(packet);
      continue;
    }

    // Check if the packet is an IPv4 packet
    if ((f[12] != 0x08) || (f[13] != 0x00)) {
      eth.freePacket(packet);
      continue;
    }

    // Check if the packet is an ICMP packet
    if (f[23] != 0x01) {
      eth.freePacket(packet);
      continue;
    }

    // Check if the packet is an ICMP ping request
    if (f[23] != 0x01) {
      eth.freePacket(packet);
      continue;
    }

    // Fake an ICMP echo request packet
    static uint8_t headerTemplate[] = {
        // Ethernet header
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // Destination MAC (needs change)
        MAC[0], MAC[1], MAC[2], MAC[3], MAC[4], MAC[5], // Source MAC
        0x08, 0x00,                                     // Ethertype
        // IP header
        0x45,                   // Version, Header Length
        0x00,                   // ECN
        0x00, 0x00,             // Total Length (needs change)
        0x00, 0x3A,             // IP ID
        0x00, 0x00,             // Frag Flags
        0x40,                   // TTL
        0x01,                   // IP Protocol
        0x00, 0x00,             // Header Checksum
        0x0A, 0x00, 0x00, 0xCE, // Source IP
        0x00, 0x00, 0x00, 0x00, // Destination IP (needs change)
        // ICMP header
        0x00,       // Type
        0x00,       // Code
        0x00, 0x00, // Checksum (needs change)
        0xCF, 0x95, // ID (needs change)
        0x00, 0x21  // Seq num (needs change)
    };

    // Destination MAC
    headerTemplate[0] = f[6];
    headerTemplate[1] = f[7];
    headerTemplate[2] = f[8];
    headerTemplate[3] = f[9];
    headerTemplate[4] = f[10];
    headerTemplate[5] = f[11];

    // Destination IP
    headerTemplate[30] = f[26];
    headerTemplate[31] = f[27];
    headerTemplate[32] = f[28];
    headerTemplate[33] = f[29];

    // ICMP Echo ID and Seq
    headerTemplate[38] = f[38];
    headerTemplate[39] = f[39];
    headerTemplate[40] = f[40];
    headerTemplate[41] = f[41];

    // Calculate IP total length
    packet->frameLength -= 4; // Remove CRC lengh
    uint16_t ipTotalLength = packet->frameLength - 14;
    headerTemplate[16] = static_cast<uint8_t>((ipTotalLength & 0xFF00) >> 8);
    headerTemplate[17] = static_cast<uint8_t>(ipTotalLength & 0x00FF);

    // No need to set frameLength: we're reusing the same buffer
    memcpy(f, headerTemplate, sizeof(headerTemplate));
    // No need to copy the data: we're reusing the same buffer

    // Calculate IP checksum
    uint16_t ipcheckSum = calculateChecksum(&f[14], 20);
    f[24] = static_cast<uint8_t>((ipcheckSum & 0xFF00) >> 8);
    f[25] = static_cast<uint8_t>(ipcheckSum & 0x00FF);

    // Calculate ICMP checksum
    uint16_t icmpChecksum = calculateChecksum(&f[34], packet->frameLength - 34);
    f[36] = static_cast<uint8_t>((icmpChecksum & 0xFF00) >> 8);
    f[37] = static_cast<uint8_t>(icmpChecksum & 0x00FF);

    eth.transmit(packet);
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

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  Flash::setLatency(8);
  Clock::configureAPB1Prescaler(Clock::APBPrescaler::DIV_4);
  Clock::configurePLL(8, 128);
  Clock::switchSysclk(Clock::SysclkSource::PLL);

  DEBUG_INIT();

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

  printf("Hello, SPI ENC28J60!\r\n");

  // Check VCC level
  ADC_1.enable();
  ADC_1.selectChannel(17);
  ADC::enableTempSensorAndVRefInt();
  float vcc = 1.21 / ADC_1.convert() * (1 << 12);
  printf("VCC: %f\r\n", vcc);

  if (vcc < 3.3) {
    DEBUG_FAIL("VCC too low!\r\n");
  }

  initializeEthernet();

  while (true) {
    eth.process();
    processEvents();
  }
}
