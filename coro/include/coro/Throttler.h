#pragma once

#include "coro/Scheduler.h"

#include "container/CircularBuffer.h"

#include <stddef.h>
#include <stdlib.h>

using container::CircularBuffer;
using container::FixedSizeCircularBuffer;
using std::experimental::coroutine_handle;

struct Throttler {
public:
  struct EnterAwaiter;

  Throttler(Scheduler& sched, size_t limit)
      : sched_(sched), limit_(limit), waitHead_(nullptr) {}

  EnterAwaiter enter();
  void leave();

private:
  Scheduler& sched_;

  size_t limit_;
  EnterAwaiter* waitHead_;
};

struct Throttler::EnterAwaiter {
public:
  EnterAwaiter(Throttler& throt) : throt_(throt) {}

  bool await_ready() { return throt_.limit_ > 0; }
  void await_suspend(coroutine_handle<> coro);
  void await_resume() { throt_.limit_--; }

private:
  Throttler& throt_;

  coroutine_handle<> awaiter_;
  EnterAwaiter* waitNext_;

  friend Throttler;
};
