#include <DMA.h>

#define DEFINE_DMA_ISR(n, s)                                                   \
  extern "C" void isrDMA##n##Stream##s() { DMA_##n.handleInterrupt(s); }

DEFINE_DMA_ISR(1, 0);
DEFINE_DMA_ISR(1, 1);
DEFINE_DMA_ISR(1, 2);
DEFINE_DMA_ISR(1, 3);
DEFINE_DMA_ISR(1, 4);
DEFINE_DMA_ISR(1, 5);
DEFINE_DMA_ISR(1, 6);
DEFINE_DMA_ISR(1, 7);
DEFINE_DMA_ISR(2, 0);
DEFINE_DMA_ISR(2, 1);
DEFINE_DMA_ISR(2, 2);
DEFINE_DMA_ISR(2, 3);
DEFINE_DMA_ISR(2, 4);
DEFINE_DMA_ISR(2, 5);
DEFINE_DMA_ISR(2, 6);
DEFINE_DMA_ISR(2, 7);

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

static void enableDMAInterrupt(DMA_TypeDef* dma, int streamNumber) {
  if (dma == DMA1) {
    IRQn_Type irqs[] = {
        DMA1_Stream0_IRQn, DMA1_Stream1_IRQn, DMA1_Stream2_IRQn,
        DMA1_Stream3_IRQn, DMA1_Stream4_IRQn, DMA1_Stream5_IRQn,
        DMA1_Stream6_IRQn, DMA1_Stream7_IRQn,
    };

    NVIC_EnableIRQ(irqs[streamNumber]);
  }

  if (dma == DMA2) {
    IRQn_Type irqs[] = {
        DMA2_Stream0_IRQn, DMA2_Stream1_IRQn, DMA2_Stream2_IRQn,
        DMA2_Stream3_IRQn, DMA2_Stream4_IRQn, DMA2_Stream5_IRQn,
        DMA2_Stream6_IRQn, DMA2_Stream7_IRQn,
    };

    NVIC_EnableIRQ(irqs[streamNumber]);
  }

  // TODO: Error handling
}

void DMA::configureStream(int streamNumber, uint32_t channel, Direction dir,
                          uint32_t n, FIFOThreshold fifoThres, bool circular,
                          Priority priority, volatile void* src, Size srcSize,
                          bool srcInc, volatile void* dst, Size dstSize,
                          bool dstInc, StreamEventHandler eventHandler,
                          void* eventHandlerContext) {
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

  streamEventHandlers_[streamNumber] = eventHandler;
  streamEventHandlerContexts_[streamNumber] = eventHandlerContext;

  BIT_SET(stream->CR, DMA_SxCR_TCIE);

  enableDMAInterrupt(dma_, streamNumber);
}

void DMA::enableStream(int streamNumber) {
  DMA_Stream_TypeDef* stream = getStream(streamNumber);

  BIT_SET(stream->CR, DMA_SxCR_EN);
}

static bool isTCIFSet(DMA_TypeDef* dma, int streamNumber) {
  uint32_t offsets[] = {0, 6, 16, 22};

  if (streamNumber < 4) {
    return BIT_IS_SET(dma->LISR, DMA_LISR_TCIF0 << offsets[streamNumber - 0]);
  } else {
    return BIT_IS_SET(dma->HISR, DMA_HISR_TCIF4 << offsets[streamNumber - 4]);
  }

  // TODO: Error handling
  return false;
}

static void clearTCIF(DMA_TypeDef* dma, int streamNumber) {
  uint32_t offsets[] = {0, 6, 16, 22};

  if (streamNumber < 4) {
    dma->LIFCR = (DMA_LIFCR_CTCIF0 << offsets[streamNumber - 0]);
  } else {
    dma->HIFCR = (DMA_HIFCR_CTCIF4 << offsets[streamNumber - 4]);
  }
}

void DMA::handleInterrupt(int streamNumber) {
  if (isTCIFSet(dma_, streamNumber)) {
    clearTCIF(dma_, streamNumber);

    if (streamEventHandlers_[streamNumber] != NULL) {
      streamEventHandlers_[streamNumber](
          StreamEvent{StreamEventType::TRANSFER_COMPLETE},
          streamEventHandlerContexts_[streamNumber]);
    }
  }
}

DMA DMA_1(DMA1);
DMA DMA_2(DMA2);
