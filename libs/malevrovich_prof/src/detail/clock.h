#pragma once

// Internal header — do NOT include from user code.

#include <cstdint>
#include <ctime>

namespace malevrovich_prof::detail {

// ── CPU timestamp counter ──────────────────────────────────────────────────────

[[nodiscard]] __attribute__((no_instrument_function)) inline uint64_t ReadTimestamp() noexcept {
#if defined(__x86_64__) || defined(__i386__)
  uint32_t lo, hi;
  __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
  return (static_cast<uint64_t>(hi) << 32) | lo;
#elif defined(__aarch64__)
  uint64_t val;
  __asm__ __volatile__("mrs %0, cntvct_el0" : "=r"(val));
  return val;
#else
  return 0;
#endif
}

// ── Calibration ────────────────────────────────────────────────────────────────

/// Measure the number of timestamp-counter ticks per second by sampling both
/// the counter and CLOCK_MONOTONIC over a short busy-wait (~5 ms).
/// Returns 0 if the platform does not support the counter.
[[nodiscard]] __attribute__((no_instrument_function)) inline uint64_t
MeasureTicksPerSecond() noexcept {
#if defined(__x86_64__) || defined(__i386__) || defined(__aarch64__)
  struct timespec t0{}, t1{};
  ::clock_gettime(CLOCK_MONOTONIC, &t0);
  const uint64_t c0 = ReadTimestamp();

  // Spin for ~5 ms.
  constexpr long kSpinNs = 5'000'000L;
  while (true) {
    ::clock_gettime(CLOCK_MONOTONIC, &t1);
    const long elapsed_ns = (t1.tv_sec - t0.tv_sec) * 1'000'000'000L + (t1.tv_nsec - t0.tv_nsec);
    if (elapsed_ns >= kSpinNs) {
      break;
    }
  }
  const uint64_t c1 = ReadTimestamp();

  const long elapsed_ns = (t1.tv_sec - t0.tv_sec) * 1'000'000'000L + (t1.tv_nsec - t0.tv_nsec);
  if (elapsed_ns <= 0) {
    return 0;
  }
  // ticks_per_sec = (c1 - c0) * 1e9 / elapsed_ns
  return static_cast<uint64_t>((c1 - c0) * 1'000'000'000ULL / static_cast<uint64_t>(elapsed_ns));
#else
  return 0;
#endif
}

}  // namespace malevrovich_prof::detail
