#pragma once

#include <stddef.h>
#include <stdint.h>

#include <Utils.h>

#include <Coroutine.h>

#include <DMA.h>
#include <GPIO.h>
#include <SPI.h>

class CoroSPI {
private:
  SPI* spi_;
  DMA::Channel dmaTx_;
  DMA::Channel dmaRx_;

  Scheduler* sched_ = nullptr;

  bool busy_;
  Scheduler::WaitToken pendingWaitToken_;

  void handleTxDMAEvent(DMA::StreamEvent event);
  static void handleTxDMAEventWrapper(DMA::StreamEvent event, void* context);

public:
  enum class TransactionType {
    SYNC,
    DMA,
    AUTO,
  };

  void bindToScheduler(Scheduler* sched);
  void enable(SPI* spi, DMA::Channel dmaTx, DMA::Channel dmaRx);

  Task<bool> transact(uint8_t* data, size_t len,
                      TransactionType type = TransactionType::AUTO);
  Task<bool> transact(GPIO::Pin pinCs, uint8_t* data, size_t len,
                      TransactionType type = TransactionType::AUTO);
};
