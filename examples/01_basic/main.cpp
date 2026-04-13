// ─────────────────────────────────────────────────────────────────────────────
// Example 01 — Recursive Fibonacci
//
// A classic recursive algorithm with deep call stacks.
// Build with target_enable_malevrovich_prof() to see per-function call counts
// and total CPU time in the automatic report at program exit.
// ─────────────────────────────────────────────────────────────────────────────

#include <cstdio>

static long long Fib(int n) {
  if (n <= 1)
    return n;
  return Fib(n - 1) + Fib(n - 2);
}

int main() {
  for (int i = 10; i <= 35; i += 5) {
    std::printf("Fib(%2d) = %lld\n", i, Fib(i));
  }
  return 0;
}
