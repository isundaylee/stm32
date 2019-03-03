#include "coro/Throttler.h"

Throttler::EnterAwaiter Throttler::enter() { return EnterAwaiter{*this}; }

void Throttler::leave() {
  if (queue_.empty()) {
    limit_++;
  } else {
    auto next = queue_.front();
    queue_.pop_front();
    sched_.enqueueTask(next);
  }
}
