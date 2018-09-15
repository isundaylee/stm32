#pragma once

#include "I2C.h"

template <uint8_t Addr, size_t H, size_t W>
class OLED {
private:
  I2C& i2c_;
  uint8_t buffer[H / 8][W];

  bool sendCommand(uint8_t cmd) {
    uint8_t data[] = {0x00, cmd};
    return i2c_.write(Addr, sizeof(data) / sizeof(data[0]), data);
  }

  bool sendCommand(uint8_t cmd, uint8_t val) {
    uint8_t data[] = {0x00, cmd, val};
    return i2c_.write(Addr, sizeof(data) / sizeof(data[0]), data);
  }

  bool sendCommand(uint8_t cmd, uint8_t val1, uint8_t val2) {
    uint8_t data[] = {0x00, cmd, val1, val2};
    return i2c_.write(Addr, sizeof(data) / sizeof(data[0]), data);
  }

public:
  enum class MemoryAddressingMode {
    HORIZONTAL,
    VERTICAL,
    PAGE,
  };

  OLED(I2C& i2c) : i2c_(i2c) {}

  bool turnDisplayOn() { return sendCommand(0xAF); }
  bool turnDisplayOff() { return sendCommand(0xAE); }

  bool enableChargePump() { return sendCommand(0x8D, 0x14); }

  bool enableEntireDisplay() { return sendCommand(0xA5); }
  bool disableEntireDisplay() { return sendCommand(0xA4); }

  bool deactivateScroll() { return sendCommand(0x2E); }

  bool setMemoryAddressingMode(MemoryAddressingMode mode) {
    switch (mode) {
    case MemoryAddressingMode::HORIZONTAL:
      return sendCommand(0x20, 0x00);
    case MemoryAddressingMode::VERTICAL:
      return sendCommand(0x20, 0x01);
    case MemoryAddressingMode::PAGE:
      return sendCommand(0x20, 0x02);
    }
  }

  bool setColumnAddress(uint8_t low, uint8_t high) {
    return sendCommand(0x21, low, high);
  }

  bool setRowAddress(uint8_t low, uint8_t high) {
    return sendCommand(0x22, low, high);
  }

  //////////////////////////////////////////////////////////////////////////////

  void clearScreen() {
    for (size_t r=0; r<H/8; r++) {
      for (size_t c=0; c<W; c++) {
        buffer[r][c] = 0x00;
      }
    }
  }

  void setPixel(size_t r, size_t c) {
    buffer[r / 8][c] |= (1 << (r % 8));
  }

  void rectangle(size_t left, size_t right, size_t top, size_t bottom) {
    for (size_t r=top; r<=bottom; r++) {
      for (size_t c=left; c<=right; c++) {
        setPixel(r, c);
      }
    }
  }

  bool render() {
    if (!setMemoryAddressingMode(MemoryAddressingMode::HORIZONTAL)) {
      return false;
    }

    if (!setColumnAddress(0, W - 1)) {
      return false;
    }

    if (!setRowAddress(0, H / 8 - 1)) {
      return false;
    }

    for (size_t r=0; r<H/8; r++) {
      for (size_t c=0; c<W; c+=16) {
        static uint8_t data[1 + 16];

        data[0] = 0x40;
        for (size_t i=0; i<16; i++) {
          data[i + 1] = buffer[r][c + i];
        }

        if (!i2c_.write(Addr, sizeof(data) / sizeof(data[0]), data)) {
          return false;
        }
      }
    }

    return true;
  }
};
