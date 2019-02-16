#include "enc28j60/ENC28J60.h"

#include <USART.h>

namespace enc28j60 {

////////////////////////////////////////////////////////////////////////////////
// Initialization routines
////////////////////////////////////////////////////////////////////////////////

static uint8_t lowByte(uint16_t num) { return (num & 0x00FF); }
static uint8_t highByte(uint16_t num) { return (num & 0xFF00) >> 8; }

void ENC28J60::initializeMAC() {
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON1,
                        MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON3,
                        MACON3_PADCFG0 | MACON3_TXCRCEN | MACON3_FULDPX);
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON4,
                        MACON4_DEFER);

  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAMXFLL,
                        0x00);
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAMXFLH,
                        0x06);

  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MABBIPG,
                        0x15);
  core_.writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAIPGL,
                        0x12);

  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR1,
                        0x11);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR2,
                        0x22);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR3,
                        0x33);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR4,
                        0x44);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR5,
                        0x55);
  core_.writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR6,
                        0x66);
}

void ENC28J60::initializeETH() {
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXSTL,
                        lowByte(CONFIG_ERXST));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXSTH,
                        highByte(CONFIG_ERXST));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXNDL,
                        lowByte(CONFIG_ERXND));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXNDH,
                        highByte(CONFIG_ERXND));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTL,
                        lowByte(CONFIG_ERXST));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTH,
                        highByte(CONFIG_ERXST));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTL,
                        lowByte(CONFIG_ERXND));
  core_.writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTH,
                        highByte(CONFIG_ERXND));

  core_.writeControlReg(ControlRegBank::BANK_1, ControlRegAddress::ERXFCON,
                        0x00);
  WAIT_UNTIL(BIT_IS_SET(
      core_.readETHReg(ControlRegBank::BANK_0, ControlRegAddress::ESTAT),
      0b00000001));

  // ENC28J60 errata issue 4
  DELAY(50000);
}

void ENC28J60::initializePHY() {
  core_.writePHYReg(PHYRegAddress::PHCON1, PHCON1_PDPXMD);
}

void ENC28J60::enable(SPI* spi, GPIO::Pin pinCS, GPIO::Pin pinInt,
                      DMA::Channel dmaTx, DMA::Channel dmaRx, Mode mode,
                      EventHandler eventHandler, void* eventHandlerContext) {
  spi_ = spi;
  pinCS_ = pinCS;
  pinInt_ = pinInt;
  dmaTx_ = dmaTx;
  dmaRx_ = dmaRx;

  mode_ = mode;
  eventHandler_ = eventHandler;
  eventHandlerContext_ = eventHandlerContext;

  pinInt_.gpio->setupExternalInterrupt(pinInt_.pin,
                                       GPIO::TriggerDirection::FALLING_EDGE,
                                       handleInterruptWrapper, this);

  initializeETH();
  initializeMAC();
  initializePHY();
}

void ENC28J60::enableRx() {
  core_.setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::ECON1,
                          ECON1_RXEN);
  core_.setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::EIE,
                          EIE_INTIE | EIE_PKTIE | EIE_RXERIE);
  pinInt_.gpio->enableExternalInterrupt(pinInt_.pin);
}

////////////////////////////////////////////////////////////////////////////////
// Interrupt handlers...
////////////////////////////////////////////////////////////////////////////////

void ENC28J60::handleInterrupt() {
  receiver_.fsm_.pushEvent(Receiver::FSM::Event::INTERRUPT);
}

void handleInterruptWrapper(void* context) {
  static_cast<ENC28J60*>(context)->handleInterrupt();
}

void ENC28J60::postEvent(Event event) {
  if (!!eventHandler_) {
    eventHandler_(event, eventHandlerContext_);
  }
}

////////////////////////////////////////////////////////////////////////////////
// API (except initialization)
////////////////////////////////////////////////////////////////////////////////

void ENC28J60::process() { receiver_.fsm_.processOneEvent(); }

bool ENC28J60::linkIsUp() {
  return BIT_IS_SET(core_.readPHYReg(PHYRegAddress::PHSTAT1), PHSTAT1_LLSTAT);
}

Packet* ENC28J60::allocatePacket() { return packetBuffer_.allocate(); }

void ENC28J60::freePacket(Packet* packet) {
  // TODO: Over-reach?
  packetBuffer_.free(packet);
}

void ENC28J60::transmit(Packet* packet) {
  if (txBuffer.empty() && receiver_.fsm_.state == Receiver::FSMState::IDLE) {
    receiver_.fsm_.pushEvent(Receiver::FSMEvent::TX_REQUESTED);
  }

  // TODO: Error handling?
  txBuffer.push(packet);
}

}; // namespace enc28j60
