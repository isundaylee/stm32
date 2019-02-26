#include <Utils.h>

#include <GPIO.h>
#include <SPI.h>
#include <USART.h>

const size_t TRANSACTION_SIZE = 16;

uint8_t poorMansRand() {
  static uint8_t seed = 1;

  uint8_t ans = seed;
  seed = seed * 93;

  return ans;
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  DEBUG_INIT();

  // Sets up SPI 2
  GPIO_B.enable();
  GPIO_B.setMode(12, GPIO::PinMode::OUTPUT); // NSS / LOAD
  GPIO_B.set(12);
  GPIO_B.setMode(13, GPIO::PinMode::ALTERNATE, 5); // SCK
  GPIO_B.setMode(14, GPIO::PinMode::ALTERNATE, 5); // MISO
  GPIO_B.setMode(15, GPIO::PinMode::ALTERNATE, 5); // MOSI
  SPI_2.configureMaster(SPI::ClockPolarity::IDLE_HIGH,
                        SPI::ClockPhase::SAMPLE_ON_SECOND_EDGE,
                        SPI::DataFrameFormat::BYTE, SPI::BaudRate::PCLK_OVER_8,
                        SPI::NSSMode::MANUAL);
  SPI_2.enable();

  USART_1.write("Hello, SPI loopback testing!\r\n");

  uint16_t answer[TRANSACTION_SIZE];
  uint16_t data[TRANSACTION_SIZE];

  while (true) {
    for (size_t i = 0; i < TRANSACTION_SIZE; i++) {
      answer[i] = poorMansRand();
      data[i] = answer[i];
    }

    GPIO_B.clear(12);
    SPI_2.transact(data, TRANSACTION_SIZE);
    GPIO_B.set(12);

    bool correct = true;

    for (size_t i = 0; i < TRANSACTION_SIZE; i++) {
      if (data[i] != answer[i]) {
        correct = false;
        break;
      }
    }

    if (correct) {
      USART_1.write(".");
    } else {
      USART_1.write("X");
      break;
    }
  }

  while (true) {
  }
}
