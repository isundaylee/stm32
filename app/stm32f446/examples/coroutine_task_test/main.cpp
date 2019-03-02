#include <Utils.h>

#include <Coroutine.h>

#include <Timer.h>

static const size_t NUM_TIMERS = 3;

using Sched = Scheduler<16, 16>;

Sched sched;
Sched::WaitToken tokens[NUM_TIMERS];

Task<int> waitForTimer(Sched& sched, size_t timerNumber) {
  co_await sched.waitUntil(tokens[timerNumber]);
  co_return 100;
}

Task<> f(Sched& sched) {
  int answer = co_await waitForTimer(sched, 0);
  DEBUG_PRINT("Answer is: %d\r\n", answer);
}

void handleTimerInterrupt() {
  static size_t count = 0;

  if (count >= 1 && count <= NUM_TIMERS) {
    sched.postCompletion(tokens[count - 1]);
  }

  count++;
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  DEBUG_INIT();

  for (size_t i = 0; i < NUM_TIMERS; i++) {
    tokens[i] = sched.allocateWaitToken();
  }

  Timer_2.enable(2000, 6000, handleTimerInterrupt);

  DEBUG_PRINT("Hello, coroutine!\r\n");
  auto task = f(sched);
  sched.run();
  DEBUG_PRINT("Bye, coroutine!\r\n");

  WAIT_UNTIL(false);
}
