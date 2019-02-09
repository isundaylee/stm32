#include <Utils.h>

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>
#include <USART.h>

#include <RingBuffer.h>

const size_t TRANSACTION_SIZE = 16;
const size_t EVENT_BUFFER_SIZE = 16;

enum class EventType {
  TX_DMA_DONE,
};

struct Event {
  EventType type;

  void operator=(Event const volatile& copy) volatile { type = copy.type; }
};

volatile RingBuffer<Event, EVENT_BUFFER_SIZE> events;

uint8_t txData[TRANSACTION_SIZE];
uint8_t rxData[TRANSACTION_SIZE];

uint8_t poorMansRand() {
  static uint8_t seed = 1;

  uint8_t ans = seed;
  seed = seed * 93;

  return ans;
}

void handleTxDMAEvent(DMA::StreamEvent event) {
  switch (event.type) {
  case DMA::StreamEventType::TRANSFER_COMPLETE:
    events.push(Event{EventType::TX_DMA_DONE});
    return;
  }
}

static void initialize() {
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
  SPI_2.configureMaster(SPI::ClockPolarity::IDLE_HIGH,
                        SPI::ClockPhase::SAMPLE_ON_SECOND_EDGE,
                        SPI::DataFrameFormat::BYTE, SPI::BaudRate::PCLK_OVER_2,
                        SPI::NSSMode::MANUAL);

  // Sets up DMA
  SPI_2.enableRxDMA();
  SPI_2.enableTxDMA();
  DMA_1.enable();
  DMA_1.configureStream(4, 0, DMA::Direction::MEM_TO_PERI, TRANSACTION_SIZE,
                        DMA::FIFOThreshold::DIRECT, false,
                        DMA::Priority::VERY_HIGH, txData, DMA::Size::BYTE, true,
                        &SPI2->DR, DMA::Size::BYTE, false, handleTxDMAEvent);
  DMA_1.configureStream(3, 0, DMA::Direction::PERI_TO_MEM, TRANSACTION_SIZE,
                        DMA::FIFOThreshold::DIRECT, false,
                        DMA::Priority::VERY_HIGH, &SPI2->DR, DMA::Size::BYTE,
                        false, rxData, DMA::Size::BYTE, true, nullptr);
  SPI_2.enable();
}

void startSPITransaction() {
  // Generates data
  for (size_t i = 0; i < TRANSACTION_SIZE; i++) {
    txData[i] = poorMansRand();
  }

  // Start SPI transaction
  GPIO_B.clear(12);
  DMA_1.enableStream(3);
  DMA_1.enableStream(4);
}

bool checkSPITransactionResult() {
  // Check loopback correctness
  bool correct = true;

  for (size_t i = 0; i < TRANSACTION_SIZE; i++) {
    if (txData[i] != rxData[i]) {
      USART_1.write(HexString(i));
      USART_1.write(" ");
      USART_1.write(HexString(txData[i]));
      USART_1.write(" ");
      USART_1.write(HexString(rxData[i]));
      USART_1.write(" ");
      correct = false;
      break;
    }
  }

  if (correct) {
    USART_1.write(".");
  } else {
    USART_1.write("X");
    return false;
  }

  // Check OVR flag
  if (BIT_IS_CLEAR(SPI2->SR, SPI_SR_OVR)) {
    USART_1.write(".");
  } else {
    USART_1.write("O");
    return false;
  }

  return true;
}

void processEvents() {
  while (!events.empty()) {
    Event event;
    events.pop(event);

    switch (event.type) {
    case EventType::TX_DMA_DONE: {
      SPI_2.waitUntilNotBusy();
      GPIO_B.set(12);

      if (checkSPITransactionResult()) {
        startSPITransaction();
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
  initialize();

  USART_1.write("Hello, SPI loopback DMA testing!\r\n");

  startSPITransaction();

  while (true) {
    processEvents();
  }
}
