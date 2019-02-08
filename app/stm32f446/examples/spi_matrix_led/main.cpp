#include <Utils.h>

#include <GPIO.h>
#include <SPI.h>

static const int SCREEN_TEST_DURATION_NOPS = 1000000;

static const size_t NUM_LED_MATRICES = 4;

void sendCommand(uint8_t addr, uint8_t value) {
  static uint16_t data[NUM_LED_MATRICES];

  for (size_t i = 0; i < NUM_LED_MATRICES; i++) {
    data[i] = (static_cast<uint16_t>(addr) << 8) + value;
  }

  GPIO_B.clear(12);
  SPI_2.transmit(data, sizeof(data) / sizeof(data[0]));
  GPIO_B.set(12);
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  GPIO_B.enable();

  GPIO_B.setMode(13, GPIO::PinMode::ALTERNATE, 5); // SCK
  GPIO_B.setMode(15, GPIO::PinMode::ALTERNATE, 5); // MOSI

  GPIO_B.setMode(12, GPIO::PinMode::OUTPUT); // NSS / LOAD
  GPIO_B.set(12);

  SPI_2.configureMaster(SPI::ClockPolarity::IDLE_HIGH,
                        SPI::ClockPhase::SAMPLE_ON_SECOND_EDGE,
                        SPI::DataFrameFormat::HALFWORD,
                        SPI::BaudRate::PCLK_OVER_32, SPI::NSSMode::MANUAL);
  SPI_2.enable();

  sendCommand(0x0C, 0x01); // Shutdown mode off
  sendCommand(0x0A, 0x00); // Intensity
  sendCommand(0x0B, 0x07); // Scan limit 0x07
  sendCommand(0x0F, 0x01); // Test mode on

  // Wait a bit
  DELAY(SCREEN_TEST_DURATION_NOPS);

  sendCommand(0x0F, 0x00); // Test mode off

  // Do a checkerboard pattern
  for (int row = 1; row <= 8; row++) {
    sendCommand(row, (row % 2 == 1) ? 0xAA : 0x55);
  }

  while (true) {
  }
}
