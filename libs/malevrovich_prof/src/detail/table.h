#pragma once

// Internal header — do NOT include from user code.

#include "malevrovich_prof/profiler.h"

#include <ankerl/unordered_dense.h>
#include <cstdint>
#include <vector>

namespace malevrovich_prof::detail {

// Per-function aggregated stats stored directly in the hash table.
struct FuncStats {
  uint64_t call_count{0};
  uint64_t total_ticks{0};
  // Small stack of enter-timestamps for nested/recursive calls.
  // Using a fixed-size inline buffer avoids heap allocation for typical depths.
  static constexpr std::size_t kMaxDepth = 256;
  uint64_t enter_stack[kMaxDepth]{};
  uint32_t depth{0};
};

// The global aggregation table. Key = function address.
using StatsTable = ankerl::unordered_dense::map<void*, FuncStats>;

StatsTable& GetTable() noexcept;

/// Returns the calibrated CPU ticks-per-second, or 0 if unavailable.
uint64_t GetTicksPerSec() noexcept;

}  // namespace malevrovich_prof::detail
