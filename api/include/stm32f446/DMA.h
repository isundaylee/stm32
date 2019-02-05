#pragma once

#include <DeviceHeader.h>

class DMA {
private:
  DMA_TypeDef* dma_;

  DMA_Stream_TypeDef* getStream(int streamNumber);

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

  DMA(DMA_TypeDef* dma) : dma_(dma) {}

  void enable();

  void configureStream(int streamNumber, uint32_t channel, Direction dir,
                       uint32_t n, FIFOThreshold fifoThres, bool circular,
                       Priority priority, volatile void* src, Size srcSize,
                       bool srcInc, volatile void* dst, Size dstSize,
                       bool dstInc);
  void enableStream(int streamNumber);
};

extern DMA DMA_1;
extern DMA DMA_2;
