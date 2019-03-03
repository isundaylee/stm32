#pragma once

#include <stddef.h>
#include <stdint.h>

#include <Utils.h>

#include <Coroutine.h>

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>

class CoroSPI {
public:
  enum class TransactionType {
    SYNC,
    DMA,
    AUTO,
  };

private:
  SPI* spi_;
  DMA::Channel dmaTx_;
  DMA::Channel dmaRx_;

  Scheduler& sched_;
  FixedSizeThrottler<16> throttler_;

  Scheduler::WaitToken pendingWaitToken_;

  void handleTxDMAEvent(DMA::StreamEvent event);
  static void handleTxDMAEventWrapper(DMA::StreamEvent event, void* context);

  Task<bool> transactInner(uint8_t* data, size_t len,
                           TransactionType type = TransactionType::AUTO);

public:
  CoroSPI(Scheduler& sched) : sched_(sched), throttler_(sched_, 1) {}

  void enable(SPI* spi, DMA::Channel dmaTx, DMA::Channel dmaRx);

  Task<bool> transact(uint8_t* data, size_t len,
                      TransactionType type = TransactionType::AUTO);
  Task<bool> transact(GPIO::Pin pinCs, uint8_t* data, size_t len,
                      TransactionType type = TransactionType::AUTO);
};
