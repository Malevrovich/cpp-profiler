#include "malevrovich_prof/profiler.h"

static void Fib(int n) {
  if (n <= 1)
    return;
  Fib(n - 1);
  Fib(n - 2);
}

int main() {
  Fib(20);
  // Stats are flushed automatically at program exit via PrintProfiler (stdout).
  // Call malevrovich_prof::SetAutoFlush(nullptr) to disable,
  // or malevrovich_prof::SetAutoFlush(&myBackend) to use a custom backend.
  return 0;
}
