#include <Utils.h>

#include <Coroutine.h>

#include <Timer.h>

using Sched = Scheduler<16, 16>;

Sched sched;

void handleTimerInterrupt(void* context) {
  sched.postCompletion(Sched::WaitToken::fromContext(context));
}

Task<> delay(Sched& sched, Timer& timer, int ms) {
  auto waitToken = sched.allocateWaitToken();
  timer.enable(16000, ms, Timer::Action::ONE_SHOT, handleTimerInterrupt,
               waitToken.toContext());
  co_await sched.waitUntil(waitToken);
}

Task<> f(Sched& sched, char const* word, Timer& timer, int ms) {
  for (;;) {
    co_await delay(sched, timer, ms);
    DEBUG_PRINT("%s\r\n", word);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  DEBUG_INIT();

  DEBUG_PRINT("Hello, coroutine!\r\n");
  Task<> tasks[] = {f(sched, "Ping", Timer_2, 500),
                    f(sched, "Waz", Timer_3, 750),
                    f(sched, "Pong", Timer_5, 1250)};
  sched.run();
  DEBUG_PRINT("Bye, coroutine!\r\n");

  WAIT_UNTIL(false);
}
