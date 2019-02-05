#include <DMA.h>

void DMA::enable() {
  if (dma_ == DMA1) {
    BIT_SET(RCC->AHB1ENR, RCC_AHB1ENR_DMA1EN);
  } else if (dma_ == DMA2) {
    BIT_SET(RCC->AHB1ENR, RCC_AHB1ENR_DMA2EN);
  }
}

DMA_Stream_TypeDef* DMA::getStream(int streamNumber) {
  if (dma_ == DMA1) {
    return reinterpret_cast<DMA_Stream_TypeDef*>(
        DMA1_Stream0_BASE +
        (DMA1_Stream1_BASE - DMA1_Stream0_BASE) * streamNumber);
  } else if (dma_ == DMA2) {
    return reinterpret_cast<DMA_Stream_TypeDef*>(
        DMA2_Stream0_BASE +
        (DMA2_Stream1_BASE - DMA2_Stream0_BASE) * streamNumber);
  }

  return nullptr;
}

void DMA::configureStream(int streamNumber, uint32_t channel, Direction dir,
                          uint32_t n, FIFOThreshold fifoThres, bool circular,
                          Priority priority, volatile void* src, Size srcSize,
                          bool srcInc, volatile void* dst, Size dstSize,
                          bool dstInc) {
  DMA_Stream_TypeDef* stream = getStream(streamNumber);

  stream->CR = 0;
  stream->FCR = 0x21;

  stream->NDTR = n;

  FIELD_SET(stream->CR, DMA_SxCR_DIR, static_cast<uint32_t>(dir));
  FIELD_SET(stream->CR, DMA_SxCR_CHSEL, channel);
  FIELD_SET(stream->CR, DMA_SxCR_PL, static_cast<uint32_t>(priority));

  if (fifoThres != DMA::FIFOThreshold::DIRECT) {
    FIELD_SET(stream->FCR, DMA_SxFCR_FTH, static_cast<uint32_t>(fifoThres));
    BIT_SET(stream->FCR, DMA_SxFCR_DMDIS);
  }

  if (circular) {
    BIT_SET(stream->CR, DMA_SxCR_CIRC);
  }

  if ((dir == Direction::PERI_TO_MEM) | (dir == Direction::MEM_TO_MEM)) {
    stream->M0AR = reinterpret_cast<uintptr_t>(dst);
    stream->PAR = reinterpret_cast<uintptr_t>(src);

    FIELD_SET(stream->CR, DMA_SxCR_PSIZE, static_cast<uint32_t>(srcSize));
    FIELD_SET(stream->CR, DMA_SxCR_MSIZE, static_cast<uint32_t>(dstSize));

    if (srcInc) {
      BIT_SET(stream->CR, DMA_SxCR_PINC);
    }

    if (dstInc) {
      BIT_SET(stream->CR, DMA_SxCR_MINC);
    }
  } else {
    stream->M0AR = reinterpret_cast<uintptr_t>(src);
    stream->PAR = reinterpret_cast<uintptr_t>(dst);

    FIELD_SET(stream->CR, DMA_SxCR_PSIZE, static_cast<uint32_t>(dstSize));
    FIELD_SET(stream->CR, DMA_SxCR_MSIZE, static_cast<uint32_t>(srcSize));

    if (dstInc) {
      BIT_SET(stream->CR, DMA_SxCR_PINC);
    }

    if (srcInc) {
      BIT_SET(stream->CR, DMA_SxCR_MINC);
    }
  }
}

void DMA::enableStream(int streamNumber) {
  DMA_Stream_TypeDef* stream = getStream(streamNumber);

  BIT_SET(stream->CR, DMA_SxCR_EN);
}

DMA DMA_1(DMA1);
DMA DMA_2(DMA2);
