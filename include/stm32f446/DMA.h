#pragma once

#include <DeviceHeader.h>

const uint32_t DMA_DIR_PERI_TO_MEM = 0b00;
const uint32_t DMA_DIR_MEM_TO_PERI = 0b01;
const uint32_t DMA_DIR_MEM_TO_MEM = 0b10;

const uint32_t DMA_PRIORITY_LOW = 0b00;
const uint32_t DMA_PRIORITY_MEDIUM = 0b01;
const uint32_t DMA_PRIORITY_HIGH = 0b10;
const uint32_t DMA_PRIORITY_VERY_HIGH = 0b11;

const uint32_t DMA_SIZE_8_BIT = 0b00;
const uint32_t DMA_SIZE_16_BIT = 0b01;
const uint32_t DMA_SIZE_32_BIT = 0b10;

const uint32_t DMA_FIFO_THRES_DIRECT = 0b100;
const uint32_t DMA_FIFO_THRES_QUARTER = 0b00;
const uint32_t DMA_FIFO_THRES_HALF = 0b01;
const uint32_t DMA_FIFO_THRES_THREE_QUARTERS = 0b10;
const uint32_t DMA_FIFO_THRES_FULL = 0b11;

class DMA {
private:
  DMA_TypeDef *dma_;

  DMA_Stream_TypeDef *getStream(int streamNumber);

public:
  DMA(DMA_TypeDef *dma) : dma_(dma) {}

  void enable();

  void configureStream(int streamNumber, uint32_t channel, uint32_t dir,
                       uint32_t n, uint32_t fifoThres, bool circular,
                       uint32_t priority, volatile void *src, uint32_t srcSize,
                       bool srcInc, volatile void *dst, uint32_t dstSize,
                       bool dstInc);
  void enableStream(int streamNumber);
};

extern DMA DMA_1;
extern DMA DMA_2;
