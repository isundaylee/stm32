#include "coro/Throttler.h"

namespace coro {

Throttler::EnterAwaiter Throttler::enter() { return EnterAwaiter{*this}; }

void Throttler::leave() {
  if (!waitHead_) {
    limit_++;
  } else {
    auto next = waitHead_->awaiter_;
    waitHead_ = waitHead_->waitNext_;
    sched_.enqueueTask(next);
  }
}

void Throttler::EnterAwaiter::await_suspend(coroutine_handle<> coro) {
  awaiter_ = coro;
  waitNext_ = throt_.waitHead_;
  throt_.waitHead_ = this;
}

} // namespace coro
