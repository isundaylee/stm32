#include "CoroSPI.h"

static const size_t CORO_SPI_USE_DMA_LENGTH_THRESHOLD = 20;

void CoroSPI::bindToScheduler(Scheduler* sched) { sched_ = sched; }

void CoroSPI::enable(SPI* spi, DMA::Channel dmaTx, DMA::Channel dmaRx) {
  spi_ = spi;
  dmaTx_ = dmaTx;
  dmaRx_ = dmaRx;

  dmaTx.dma->configureStream(
      dmaTx.stream, dmaTx.channel, DMA::Direction::MEM_TO_PERI, 0,
      DMA::FIFOThreshold::DIRECT, false, DMA::Priority::HIGH, nullptr,
      DMA::Size::BYTE, true, &spi_->getRaw()->DR, DMA::Size::BYTE, false,
      CoroSPI::handleTxDMAEventWrapper, this);
  dmaRx.dma->configureStream(
      dmaRx.stream, dmaRx.channel, DMA::Direction::PERI_TO_MEM, 0,
      DMA::FIFOThreshold::DIRECT, false, DMA::Priority::VERY_HIGH,
      &spi_->getRaw()->DR, DMA::Size::BYTE, false, nullptr, DMA::Size::BYTE,
      true, nullptr, nullptr);
}

Task<bool> CoroSPI::transact(uint8_t* data, size_t len, TransactionType type) {
  if (busy_) {
    co_return false;
  }

  bool useDMA;

  switch (type) {
  case TransactionType::SYNC: {
    useDMA = false;
    break;
  }

  case TransactionType::DMA: {
    useDMA = true;
    break;
  }

  case TransactionType::AUTO: {
    useDMA = (len >= CORO_SPI_USE_DMA_LENGTH_THRESHOLD);
    break;
  }
  }

  if (!useDMA) {
    // TODO: Fix this lololol...
    uint16_t* buf = new uint16_t[len];
    for (size_t i = 0; i < len; i++) {
      buf[i] = data[i];
    }

    spi_->transact(buf, len);

    for (size_t i = 0; i < len; i++) {
      data[i] = buf[i];
    }

    delete[] buf;

    co_return true;
  } else {
    dmaTx_.dma->reconfigureMemory(dmaTx_.stream, len, data, DMA::Size::BYTE,
                                  true);
    dmaRx_.dma->reconfigureMemory(dmaRx_.stream, len, data, DMA::Size::BYTE,
                                  true);

    pendingWaitToken_ = sched_->allocateWaitToken();

    spi_->enableRxDMA();
    dmaTx_.dma->enableStream(dmaTx_.stream);
    dmaRx_.dma->enableStream(dmaRx_.stream);
    spi_->enableTxDMA();

    busy_ = true;
    co_await sched_->waitUntil(pendingWaitToken_);
    busy_ = false;

    spi_->waitUntilNotBusy();
    auto rxBytesLeft = dmaRx_.dma->getNumberOfData(dmaRx_.stream);
    if (rxBytesLeft != 0) {
      DEBUG_ASSERT(BIT_IS_SET(spi_->getRaw()->SR, SPI_SR_OVR),
                   "Rx DMA incomplete without SPI overrun.");

      // Disable Rx stream and clear OVR flag
      dmaRx_.dma->disableStream(dmaRx_.stream);
      FORCE_READ(spi_->getRaw()->DR);
      FORCE_READ(spi_->getRaw()->SR);
    }

    co_return(rxBytesLeft == 0);
  }
}

Task<bool> CoroSPI::transact(GPIO::Pin pinCs, uint8_t* data, size_t len,
                             TransactionType type) {
  pinCs.gpio->clear(pinCs.pin);
  auto success = co_await transact(data, len, type);
  pinCs.gpio->set(pinCs.pin);

  co_return success;
}

void CoroSPI::handleTxDMAEvent(DMA::StreamEvent event) {
  switch (event.type) {
  case DMA::StreamEventType::TRANSFER_COMPLETE: {
    sched_->postCompletion(pendingWaitToken_);
  }
  }
}

/* static */ void CoroSPI::handleTxDMAEventWrapper(DMA::StreamEvent event,
                                                   void* context) {
  reinterpret_cast<CoroSPI*>(context)->handleTxDMAEvent(event);
}
