#include <Utils.h>

#include <Coroutine.h>

#include <Timer.h>

using Sched = Scheduler<16, 16>;

Sched sched;
Sched::WaitToken waitTokens[3];

void handleTimerInterrupt(void* context) {
  sched.postCompletion(*reinterpret_cast<Sched::WaitToken*>(context));
}

Task<> delay(Sched& sched, Timer& timer, int ms, Sched::WaitToken& waitToken) {
  waitToken = sched.allocateWaitToken();
  timer.enable(16000, ms, Timer::Action::ONE_SHOT, handleTimerInterrupt,
               &waitToken);
  co_await sched.waitUntil(waitToken);
}

Task<> f(Sched& sched, char const* word, Timer& timer, int ms,
         Sched::WaitToken& waitToken) {
  for (;;) {
    co_await delay(sched, timer, ms, waitToken);
    DEBUG_PRINT("%s\r\n", word);
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  DEBUG_INIT();

  DEBUG_PRINT("Hello, coroutine!\r\n");
  Task<> tasks[] = {f(sched, "Ping", Timer_2, 500, waitTokens[0]),
                    f(sched, "Waz", Timer_3, 750, waitTokens[1]),
                    f(sched, "Pong", Timer_5, 1250, waitTokens[2])};
  sched.run();
  DEBUG_PRINT("Bye, coroutine!\r\n");

  WAIT_UNTIL(false);
}
