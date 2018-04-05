#include <DMA.h>

void DMA::enable() {
  if (dma_ == DMA1) {
    BIT_SET(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN);
  } else if (dma_ == DMA2) {
    BIT_SET(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);
  }
}

DMA_Stream_TypeDef *DMA::getStream(int streamNumber) {
  if (dma_ == DMA1) {
    return reinterpret_cast<DMA_Stream_TypeDef *>(
        DMA1_Stream0_BASE +
        (DMA1_Stream1_BASE - DMA1_Stream0_BASE) * streamNumber);
  } else if (dma_ == DMA2) {
    return reinterpret_cast<DMA_Stream_TypeDef *>(
        DMA2_Stream0_BASE +
        (DMA2_Stream1_BASE - DMA2_Stream0_BASE) * streamNumber);
  }

  return nullptr;
}

void DMA::configureStream(int streamNumber, uint32_t channel, uint32_t dir,
                          uint32_t n, bool circular, uint32_t priority,
                          volatile void *peri, uint32_t periSize, bool periInc,
                          volatile void *mem, uint32_t memSize, bool memInc) {
  DMA_Stream_TypeDef *stream = getStream(streamNumber);

  stream->CR = 0;

  FIELD_SET(stream->CR, DMA_SxCR_DIR, dir);
  FIELD_SET(stream->CR, DMA_SxCR_CHSEL, channel);
  FIELD_SET(stream->CR, DMA_SxCR_PL, priority);
  FIELD_SET(stream->CR, DMA_SxCR_PSIZE, periSize);
  FIELD_SET(stream->CR, DMA_SxCR_MSIZE, memSize);

  if (circular) {
    BIT_SET(stream->CR, DMA_SxCR_CIRC);
  }

  if (periInc) {
    BIT_SET(stream->CR, DMA_SxCR_PINC);
  }

  if (memInc) {
    BIT_SET(stream->CR, DMA_SxCR_MINC);
  }

  stream->NDTR = n;
  stream->M0AR = reinterpret_cast<uintptr_t>(mem);
  stream->PAR = reinterpret_cast<uintptr_t>(peri);
}

void DMA::enableStream(int streamNumber){
  DMA_Stream_TypeDef *stream = getStream(streamNumber);

  BIT_SET(stream->CR, DMA_SxCR_EN);
}

DMA DMA_1(DMA1);
DMA DMA_2(DMA2);
