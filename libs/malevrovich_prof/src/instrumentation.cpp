#include "malevrovich_prof/instrumentation.h"

#include "detail/clock.h"
#include "detail/table.h"

#include <cstdint>

extern "C" {

__attribute__((no_instrument_function)) void __cyg_profile_func_enter(void* this_fn,
                                                                      void* /*call_site*/) {
  auto& table = malevrovich_prof::detail::GetTable();
  auto& s = table[this_fn];
  s.call_count++;
  if (s.depth < malevrovich_prof::detail::FuncStats::kMaxDepth) {
    s.enter_stack[s.depth++] = malevrovich_prof::detail::ReadTimestamp();
  }
}

__attribute__((no_instrument_function)) void __cyg_profile_func_exit(void* this_fn,
                                                                     void* /*call_site*/) {
  const uint64_t now = malevrovich_prof::detail::ReadTimestamp();
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
