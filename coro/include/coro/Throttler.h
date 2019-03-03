#pragma once

#include "coro/Scheduler.h"

#include "container/CircularBuffer.h"

#include <stddef.h>
#include <stdlib.h>

using container::CircularBuffer;
using container::FixedSizeCircularBuffer;
using std::experimental::coroutine_handle;

template <size_t QueueSize> struct FixedSizeThrottler;

struct Throttler {
public:
  Throttler() = delete;

  struct EnterAwaiter {
  public:
    EnterAwaiter(Throttler& throt) : throt_(throt) {}

    bool await_ready() { return throt_.limit_ > 0; }
    void await_suspend(coroutine_handle<> coro) {
      throt_.queue_.push_back(coro);
    }
    void await_resume() { throt_.limit_--; }

  private:
    Throttler& throt_;
  };

  auto enter() { return EnterAwaiter{*this}; }

  void leave() {
    if (queue_.empty()) {
      limit_++;
    } else {
      auto next = queue_.front();
      queue_.pop_front();
      sched_.enqueueTask(next);
    }
  }

  template <size_t> friend struct FixedSizeThrottler;

private:
  Scheduler& sched_;
  size_t limit_;
  CircularBuffer<coroutine_handle<>>& queue_;

  Throttler(Scheduler& sched, size_t limit,
            CircularBuffer<coroutine_handle<>>& queue)
      : sched_(sched), limit_(limit), queue_(queue) {}
};

template <size_t QueueSize> struct FixedSizeThrottler : public Throttler {
public:
  FixedSizeThrottler(Scheduler& sched, size_t limit)
      : Throttler(sched, limit, queue_) {}

private:
  FixedSizeCircularBuffer<coroutine_handle<>, QueueSize> queue_;
};
