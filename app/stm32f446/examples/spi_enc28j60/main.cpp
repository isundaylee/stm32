#include <Utils.h>

#include <GPIO.h>
#include <SPI.h>
#include <USART.h>

#include "ENC28J60.h"

#define DUMP_PACKET_HEADERS 0

uint8_t generateHeaderByte(Opcode opcode, ControlRegAddress addr) {
  return (static_cast<uint8_t>(opcode) << 5) + static_cast<uint8_t>(addr);
}

void selectControlRegBank(ControlRegBank bank) {
  static auto currentBank = ControlRegBank::BANK_0;

  if (currentBank == bank) {
    return;
  } else {
    currentBank = bank;
  }

  uint16_t data[] = {
      generateHeaderByte(Opcode::BIT_FIELD_CLEAR, ControlRegAddress::ECON1),
      0b00000011};

  GPIO_B.clear(12);
  SPI_2.transact(data, sizeof(data) / sizeof(data[0]));
  GPIO_B.set(12);

  data[0] = generateHeaderByte(Opcode::BIT_FIELD_SET, ControlRegAddress::ECON1);
  data[1] = static_cast<uint8_t>(bank);

  GPIO_B.clear(12);
  SPI_2.transact(data, sizeof(data) / sizeof(data[0]));
  GPIO_B.set(12);
}

void setETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                       uint8_t bits) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::BIT_FIELD_SET, addr), bits};

  GPIO_B.clear(12);
  SPI_2.transact(data, sizeof(data) / sizeof(data[0]));
  GPIO_B.set(12);
}

void clearETHRegBitField(ControlRegBank bank, ControlRegAddress addr,
                         uint8_t bits) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::BIT_FIELD_CLEAR, addr), bits};

  GPIO_B.clear(12);
  SPI_2.transact(data, sizeof(data) / sizeof(data[0]));
  GPIO_B.set(12);
}

uint8_t readETHReg(ControlRegBank bank, ControlRegAddress addr) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::READ_CONTROL_REGISTER, addr),
                     0x00};

  GPIO_B.clear(12);
  SPI_2.transact(data, sizeof(data) / sizeof(data[0]));
  GPIO_B.set(12);

  return static_cast<uint8_t>(data[1]);
}

uint8_t writeControlReg(ControlRegBank bank, ControlRegAddress addr,
                        uint8_t value) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::WRITE_CONTROL_REGISTER, addr),
                     value};

  GPIO_B.clear(12);
  SPI_2.transact(data, sizeof(data) / sizeof(data[0]));
  GPIO_B.set(12);

  return static_cast<uint8_t>(data[1]);
}

uint8_t readMACMIIReg(ControlRegBank bank, ControlRegAddress addr) {
  selectControlRegBank(bank);

  uint16_t data[] = {generateHeaderByte(Opcode::READ_CONTROL_REGISTER, addr),
                     0x00, 0x00};

  GPIO_B.clear(12);
  SPI_2.transact(data, sizeof(data) / sizeof(data[0]));
  GPIO_B.set(12);

  return static_cast<uint8_t>(data[2]);
}

static void initializeETH() {
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXSTL, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXSTH, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXNDL, 0xFF);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXNDH, 0x0F);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTL, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERDPTH, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTL, 0x00);
  writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTH, 0x00);

  writeControlReg(ControlRegBank::BANK_1, ControlRegAddress::ERXFCON, 0x00);

  WAIT_UNTIL(
      BIT_IS_SET(readETHReg(ControlRegBank::BANK_0, ControlRegAddress::ESTAT),
                 0b00000001));
}

static void initializeMAC() {
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON1,
                  MACON1_MARXEN | MACON1_TXPAUS | MACON1_RXPAUS);
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON3,
                  MACON3_PADCFG0 | MACON3_FRMLNEN | MACON3_FULDPX);
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MACON4,
                  MACON4_DEFER);

  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAMXFLL, 0x00);
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAMXFLH, 0x06);

  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MABBIPG, 0x15);
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MAIPGL, 0x12);

  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR1, 0x11);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR2, 0x22);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR3, 0x33);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR4, 0x44);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR5, 0x55);
  writeControlReg(ControlRegBank::BANK_3, ControlRegAddress::MAADR6, 0x66);
}

uint16_t readPHYReg(PHYRegAddress addr) {
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIREGADR,
                  static_cast<uint8_t>(addr));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MICMD, 0x01);
  WAIT_UNTIL(BIT_IS_CLEAR(
      readMACMIIReg(ControlRegBank::BANK_3, ControlRegAddress::MISTAT), 0x01));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MICMD, 0x00);

  uint8_t low = readMACMIIReg(ControlRegBank::BANK_2, ControlRegAddress::MIRDL);
  uint8_t high =
      readMACMIIReg(ControlRegBank::BANK_2, ControlRegAddress::MIRDH);

  return static_cast<uint16_t>(low) + (static_cast<uint16_t>(high) << 8);
}

void writePHYReg(PHYRegAddress addr, uint16_t value) {
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIREGADR,
                  static_cast<uint8_t>(addr));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIWRL,
                  static_cast<uint8_t>(value & 0x00FF));
  writeControlReg(ControlRegBank::BANK_2, ControlRegAddress::MIWRH,
                  static_cast<uint8_t>(value >> 8));
  WAIT_UNTIL(BIT_IS_CLEAR(
      readMACMIIReg(ControlRegBank::BANK_3, ControlRegAddress::MISTAT), 0x01));
}

void printETHReg(ControlRegBank bank, ControlRegAddress addr,
                 char const* name) {
  USART_1.write(name);
  USART_1.write(": ");
  USART_1.write(HexString(readETHReg(bank, addr)));
  USART_1.write("\r\n");
}

void printMACMIIReg(ControlRegBank bank, ControlRegAddress addr,
                    char const* name) {
  USART_1.write(name);
  USART_1.write(": ");
  USART_1.write(HexString(readMACMIIReg(bank, addr)));
  USART_1.write("\r\n");
}

void printPHYReg(PHYRegAddress addr, char const* name) {
  USART_1.write(name);
  USART_1.write(": ");
  USART_1.write(HexString(readPHYReg(addr)));
  USART_1.write("\r\n");
}

void readBufferMemory(uint16_t* data, size_t len) {
  uint16_t header[] = {generateHeaderByte(
      Opcode::READ_BUFFER_MEMORY, ControlRegAddress::READ_BUFFER_MEMORY)};

  GPIO_B.clear(12);
  SPI_2.transact(header, sizeof(header) / sizeof(header[0]));
  SPI_2.transact(data, len);
  GPIO_B.set(12);
}

static void initializePHY() {
  writePHYReg(PHYRegAddress::PHCON1, PHCON1_PDPXMD);
}

static void initialize() {
  initializeETH();
  initializeMAC();
  initializePHY();

  USART_1.write("Waiting for link");
  while (BIT_IS_CLEAR(readPHYReg(PHYRegAddress::PHSTAT1), PHSTAT1_LLSTAT)) {
    USART_1.write(".");
    DELAY(1000000);
  }
  USART_1.write("\r\n");
  USART_1.write("Link is up!\r\n");

  setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::ECON1,
                    ECON1_RXEN);
  setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::EIE,
                    EIE_INTIE | EIE_PKTIE);

  while (true) {
    if (readETHReg(ControlRegBank::BANK_1, ControlRegAddress::EPKTCNT) != 0) {
      GPIO_C.clear(7);

      uint16_t header[6];
      readBufferMemory(header, 6);

      uint8_t nextPacketPointerLow = static_cast<uint8_t>(header[0]);
      uint8_t nextPacketPointerHigh = static_cast<uint8_t>(header[1]);
      size_t frameLen = header[2] + (header[3] << 8);

      static uint16_t data[1536];
      readBufferMemory(data, frameLen + (frameLen % 2));

#if DUMP_PACKET_HEADERS
      USART_1.write("[");
      for (size_t i = 0; i < 6; i++) {
        USART_1.write(HexString(header[i], 2));
        if (i != 5) {
          USART_1.write(" ");
        }
      }
      USART_1.write("]");

      for (size_t i = 0; i < 12; i++) {
        USART_1.write(" ");
        USART_1.write(HexString(data[i], 2));
      }
      USART_1.write("\r\n");
#endif

      writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTL,
                      nextPacketPointerLow);
      writeControlReg(ControlRegBank::BANK_0, ControlRegAddress::ERXRDPTH,
                      nextPacketPointerHigh);
      setETHRegBitField(ControlRegBank::BANK_0, ControlRegAddress::ECON2,
                        ECON2_PKTDEC);

      GPIO_C.set(7);
    }

    DELAY(1000);
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
                        SPI::DataFrameFormat::BYTE, SPI::BaudRate::PCLK_OVER_4,
                        SPI::NSSMode::MANUAL);
  SPI_2.enable();

  GPIO_C.enable();
  GPIO_C.setMode(7, GPIO::PinMode::OUTPUT);
  GPIO_C.set(7);

  USART_1.write("Hello, SPI ENC28J60!\r\n");

  initialize();

  while (true) {
  }
}
