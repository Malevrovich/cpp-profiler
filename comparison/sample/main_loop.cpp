// Entry point for the `sample` profiler target.
// Runs the workload in a loop for ~10 seconds so the macOS `sample` tool
// has enough time to collect statistical samples.

#include "../workload.h"

#include <chrono>
#include <cstdio>

int main() {
  using Clock = std::chrono::steady_clock;
  const auto deadline = Clock::now() + std::chrono::seconds(10);
  int iterations = 0;
  while (Clock::now() < deadline) {
    RunWorkload();
    ++iterations;
  }
  std::printf("Completed %d iterations in ~10 s\n", iterations);
  return 0;
}
