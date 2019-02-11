#pragma once

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>

#include <FreeListBuffer.h>
#include <RingBuffer.h>

void enc28j60HandleInterruptWrapper(void* context);
void enc28j60HandleRxDMAEventWrapper(DMA::StreamEvent event, void* context);

class ENC28J60 {
public:
  enum class Mode {
    FULL_DUPLEX,
  };

  enum class ControlRegBank {
    BANK_0,
    BANK_1,
    BANK_2,
    BANK_3,
  };

  enum class ControlRegAddress {
    ERDPTL = 0x00,
    ERDPTH,
    EWRPTL,
    EWRPTH,
    ETXSTL,
    ETXSTH,
    ETXNDL,
    ETXNDH,
    ERXSTL,
    ERXSTH,
    ERXNDL,
    ERXNDH,
    ERXRDPTL,
    ERXRDPTH,
    ERXWRPTL,
    ERXWRPTH,
    EDMASTL,
    EDMASTH,
    EDMANDL,
    EDMANDH,
    EDMADSTL,
    EDMADSTH,
    EDMACSL,
    EDMACSH,
    // --
    // --
    // Reserved
    EIE = 0x1B,
    EIR,
    ESTAT,
    ECON2,
    ECON1,

    EHT0 = 0x00,
    EHT1,
    EHT2,
    EHT3,
    EHT4,
    EHT5,
    EHT6,
    EHT7,
    EPMM0,
    EPMM1,
    EPMM2,
    EPMM3,
    EPMM4,
    EPMM5,
    EPMM6,
    EPMM7,
    EPMCSL,
    EPMCSH,
    // --
    // --
    EPMOL = 0x14,
    EPMOH,
    // Reserved
    // Reserved
    ERXFCON = 0x18,
    EPKTCNT,

    MACON1 = 0x00,
    // Reserved
    MACON3 = 0x02,
    MACON4,
    MABBIPG,
    // --
    MAIPGL = 0x06,
    MAIPGH,
    MACLCON1,
    MACLCON2,
    MAMXFLL,
    MAMXFLH,
    // Reserved
    // Reserved
    // Reserved
    // --
    // Reserved
    // Reserved
    MICMD = 0x12,
    // --
    MIREGADR = 0x14,
    // Reserved
    MIWRL = 0x16,
    MIWRH,
    MIRDL,
    MIRDH,

    MAADR5 = 0x00,
    MAADR6,
    MAADR3,
    MAADR4,
    MAADR1,
    MAADR2,
    EBSTSD,
    EBSTCON,
    EBSTCSL,
    EBSTCSH,
    MISTAT,
    // --
    // --
    // --
    // --
    // --
    // --
    // --
    EREVID = 0x12,
    // --
    // --
    ECOCON = 0x15,
    // Reserved
    EFLOCON = 0x17,
    EPAUSL,
    EPAUSH,

    READ_BUFFER_MEMORY = 0x1A,
  };

  enum class PHYRegAddress {
    PHCON1 = 0x00,
    PHSTAT1 = 0x01,
    PHID1 = 0x02,
    PHID2 = 0x03,
    PHCON2 = 0x10,
    PHSTAT2 = 0x11,
    PHIE = 0x12,
    PHIR = 0x13,
    PHLCON = 0x14,
  };

  enum class Opcode {
    READ_CONTROL_REGISTER = 0b000,
    READ_BUFFER_MEMORY = 0b001,
    WRITE_CONTROL_REGISTER = 0b010,
    WRITE_BUFFER_MEMORY = 0b011,
    BIT_FIELD_SET = 0b100,
    BIT_FIELD_CLEAR = 0b101,
    SYSTEM_RESET_COMMAND = 0b111,
  };

  static const uint8_t ECON1_TXRST = 0b10000000;
  static const uint8_t ECON1_RXRST = 0b01000000;
  static const uint8_t ECON1_DMAST = 0b00100000;
  static const uint8_t ECON1_CSUMEN = 0b00010000;
  static const uint8_t ECON1_TXRTS = 0b00001000;
  static const uint8_t ECON1_RXEN = 0b00000100;
  static const uint8_t ECON1_BSEL1 = 0b00000010;
  static const uint8_t ECON1_BSEL0 = 0b00000001;

  static const uint8_t ECON2_AUTOINC = 0b10000000;
  static const uint8_t ECON2_PKTDEC = 0b01000000;
  static const uint8_t ECON2_PWRSV = 0b00100000;
  static const uint8_t ECON2_VRPS = 0b00001000;

  static const uint8_t EIE_INTIE = 0b10000000;
  static const uint8_t EIE_PKTIE = 0b01000000;
  static const uint8_t EIE_DMAIE = 0b00100000;
  static const uint8_t EIE_LINKIE = 0b00010000;
  static const uint8_t EIE_TXIE = 0b00001000;
  static const uint8_t EIE_TXERIE = 0b00000010;
  static const uint8_t EIE_RXERIE = 0b00000001;

  static const uint8_t MACON1_TXPAUS = 0b00001000;
  static const uint8_t MACON1_RXPAUS = 0b00000100;
  static const uint8_t MACON1_PASSALL = 0b00000010;
  static const uint8_t MACON1_MARXEN = 0b00000001;

  static const uint8_t MACON3_PADCFG2 = 0b10000000;
  static const uint8_t MACON3_PADCFG1 = 0b01000000;
  static const uint8_t MACON3_PADCFG0 = 0b00100000;
  static const uint8_t MACON3_TXCRCEN = 0b00010000;
  static const uint8_t MACON3_PHDREN = 0b00001000;
  static const uint8_t MACON3_HFRMEN = 0b00000100;
  static const uint8_t MACON3_FRMLNEN = 0b00000010;
  static const uint8_t MACON3_FULDPX = 0b00000001;

  static const uint8_t MACON4_DEFER = 0b01000000;
  static const uint8_t MACON4_BPEN = 0b00100000;
  static const uint8_t MACON4_NOBKOFF = 0b00010000;

  static const uint16_t PHCON1_PRST = 0b1000000000000000;
  static const uint16_t PHCON1_PLOOPBK = 0b0100000000000000;
  static const uint16_t PHCON1_PPWRSV = 0b0000100000000000;
  static const uint16_t PHCON1_PDPXMD = 0b0000000100000000;

  static const uint16_t PHSTAT1_PFDPX = 0b0001000000000000;
  static const uint16_t PHSTAT1_PHDPX = 0b0000100000000000;
  static const uint16_t PHSTAT1_LLSTAT = 0b0000000000000100;
  static const uint16_t PHSTAT1_JBSTAT = 0b0000000000000010;

  static const size_t PACKET_HEADER_SIZE = 6;
  static const size_t PACKET_FRAME_SIZE = 1600;

  struct Packet {
    uint16_t header[PACKET_HEADER_SIZE] = {0};
    size_t frameLength = 0;
    uint8_t frame[PACKET_FRAME_SIZE] = {0};
  };

  enum Event {
    RX_NEW_PACKET,
    RX_OVERFLOW,
  };

  using EventHandler = void (*)(Event event, void* context);

private:
  enum class InternalEvent {
    INTERRUPT,
    RX_DMA_COMPLETE,
  };

  enum class State {
    IDLE,
    RX_HEADER,
    RX_FRAME_PENDING,
    RX_FRAME_DONE,
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
                      EIE_INTIE | ENC28J60::EIE_PKTIE);
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
