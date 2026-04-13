#include "malevrovich_prof/instrumentation.h"

#include "detail/table.h"

#include <cstdint>

namespace {

__attribute__((no_instrument_function)) inline uint64_t ReadTimestamp() noexcept {
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

}  // namespace

extern "C" {

__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void* this_fn,
                                                                      void* /*call_site*/) {
  auto& table = malevrovich_prof::detail::GetTable();
  auto& s = table[this_fn];
  s.call_count++;
  if (s.depth < malevrovich_prof::detail::FuncStats::kMaxDepth) {
    s.enter_stack[s.depth++] = ReadTimestamp();
  }
}

__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void* this_fn,
                                                                     void* /*call_site*/) {
  const uint64_t now = ReadTimestamp();
  auto& table = malevrovich_prof::detail::GetTable();
  auto it = table.find(this_fn);
  if (it == table.end())
    return;
  auto& s = it->second;
  if (s.depth > 0) {
    s.total_ticks += now - s.enter_stack[--s.depth];
  }
}

}  // extern "C"
