#pragma once

#include <DeviceHeader.h>

const uint32_t DMA_DIR_PERI_TO_MEM = 0b00;

const uint32_t DMA_PRIORITY_LOW = 0b00;
const uint32_t DMA_PRIORITY_MEDIUM = 0b01;
const uint32_t DMA_PRIORITY_HIGH = 0b10;
const uint32_t DMA_PRIORITY_VERY_HIGH = 0b11;

const uint32_t DMA_SIZE_8_BIT = 0b00;
const uint32_t DMA_SIZE_16_BIT = 0b01;
const uint32_t DMA_SIZE_32_BIT = 0b10;

class DMA {
private:
  DMA_TypeDef *dma_;

  DMA_Stream_TypeDef *getStream(int streamNumber);

public:
  DMA(DMA_TypeDef *dma) : dma_(dma) {}

  void enable();

  void configureStream(int streamNumber, uint32_t channel, uint32_t dir,
                       uint32_t n, bool circular, uint32_t priority,
                       volatile void *peri, uint32_t periSize, bool periInc,
                       volatile void *mem, uint32_t memSize, bool memInc);
  void enableStream(int streamNumber);
};

extern DMA DMA_1;
extern DMA DMA_2;
