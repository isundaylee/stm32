#pragma once

#include <DeviceHeader.h>

extern "C" void isrDMA1Stream0();
extern "C" void isrDMA1Stream1();
extern "C" void isrDMA1Stream2();
extern "C" void isrDMA1Stream3();
extern "C" void isrDMA1Stream4();
extern "C" void isrDMA1Stream5();
extern "C" void isrDMA1Stream6();
extern "C" void isrDMA1Stream7();
extern "C" void isrDMA2Stream0();
extern "C" void isrDMA2Stream1();
extern "C" void isrDMA2Stream2();
extern "C" void isrDMA2Stream3();
extern "C" void isrDMA2Stream4();
extern "C" void isrDMA2Stream5();
extern "C" void isrDMA2Stream6();
extern "C" void isrDMA2Stream7();

class DMA {
public:
  enum class StreamEventType {
    TRANSFER_COMPLETE,
  };

  struct StreamEvent {
    StreamEventType type;
  };

  using StreamEventHandler = void (*)(StreamEvent event, void* context);

private:
  DMA_TypeDef* dma_;

  StreamEventHandler streamEventHandlers_[8];
  void* streamEventHandlerContexts_[8];

  DMA_Stream_TypeDef* getStream(int streamNumber);

  void handleInterrupt(int streamNumber);

public:
  enum class Direction {
    PERI_TO_MEM = 0b00,
    MEM_TO_PERI = 0b01,
    MEM_TO_MEM = 0b10,
  };

  enum class Priority {
    LOW = 0b00,
    MEDIUM = 0b01,
    HIGH = 0b10,
    VERY_HIGH = 0b11,
  };

  enum class Size {
    BYTE = 0b00,
    HALFWORD = 0b01,
    WORD = 0b10,
  };

  enum class FIFOThreshold {
    DIRECT = 0b100,
    QUARTER = 0b00,
    HALF = 0b01,
    THREE_QUARTERS = 0b10,
    FULL = 0b11,
  };

  struct Channel {
    DMA* dma;
    int stream;
    uint32_t channel;
  };

  DMA(DMA_TypeDef* dma) : dma_(dma) {}

  void enable();

  void configureStream(int streamNumber, uint32_t channel, Direction dir,
                       uint32_t n, FIFOThreshold fifoThres, bool circular,
                       Priority priority, volatile void* src, Size srcSize,
                       bool srcInc, volatile void* dst, Size dstSize,
                       bool dstInc, StreamEventHandler eventHandler,
                       void* eventHandlerContext);
  void reconfigureMemory(int streamNumber, uint32_t n, volatile void* addr,
                         Size size, bool inc);
  void reconfigurePeripheral(int streamNumber, uint32_t n, volatile void* addr,
                             Size size, bool inc);
  void enableStream(int streamNumber);

  size_t getNumberOfData(int streamNumber);

  friend void isrDMA1Stream0();
  friend void isrDMA1Stream1();
  friend void isrDMA1Stream2();
  friend void isrDMA1Stream3();
  friend void isrDMA1Stream4();
  friend void isrDMA1Stream5();
  friend void isrDMA1Stream6();
  friend void isrDMA1Stream7();
  friend void isrDMA2Stream0();
  friend void isrDMA2Stream1();
  friend void isrDMA2Stream2();
  friend void isrDMA2Stream3();
  friend void isrDMA2Stream4();
  friend void isrDMA2Stream5();
  friend void isrDMA2Stream6();
  friend void isrDMA2Stream7();
};

extern DMA DMA_1;
extern DMA DMA_2;
