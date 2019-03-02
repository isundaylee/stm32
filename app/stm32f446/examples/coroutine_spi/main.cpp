#include <Utils.h>

#include <CoroSPI.h>
#include <GPIO.h>

CoroSPI spi;

Task<> transactionA() {
  uint8_t data[] = {0x1E, 0x00};

  bool success = co_await spi.transact({&GPIO_B, 12}, data, sizeof(data));

  DEBUG_PRINT("Transaction A %s with reply 0x%02x 0x%02x\r\n",
              (success ? "succeeded" : "failed"), data[0], data[1]);
}

Task<> transactionB() {
  uint8_t data[] = {0x1E, 0x00};

  bool success = co_await spi.transact({&GPIO_B, 12}, data, sizeof(data));

  DEBUG_PRINT("Transaction B %s with reply 0x%02x 0x%02x\r\n",
              (success ? "succeeded" : "failed"), data[0], data[1]);
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  DEBUG_INIT();

  DEBUG_PRINT("Hello, CoroSPI!\r\n");

  // Sets up SPI 2
  DMA_1.enable();

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
  spi.enable(&SPI_2, DMA::Channel{&DMA_1, 4, 0}, DMA::Channel{&DMA_1, 3, 0});

  FixedSizeScheduler<16, 16> sched;
  spi.bindToScheduler(&sched);
  Task<> tasks[] = {transactionA(), transactionB()};
  sched.run();

  DEBUG_PRINT("Bye, CoroSPI!\r\n");

  WAIT_UNTIL(false);
}
