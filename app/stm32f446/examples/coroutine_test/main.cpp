#include <Utils.h>

#include <Coroutine.h>

using coro::Generator;

Generator<int> f(int limit) {
  for (int i = 0; i <= limit; i++) {
    co_yield i;
  }
}

////////////////////////////////////////////////////////////////////////////////
// Main!
////////////////////////////////////////////////////////////////////////////////

extern "C" void main() {
  DEBUG_INIT();

  DEBUG_PRINT("Hello, coroutine!\r\n");

  for (auto answer : f(42)) {
    DEBUG_PRINT("Answer: %d\r\n", answer);
  }

  WAIT_UNTIL(false);
}
