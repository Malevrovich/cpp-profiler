#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

namespace malevrovich_prof {

// ── Aggregated per-function statistics ────────────────────────────────────────

struct FuncStat {
  void* fn;              ///< Function address.
  uint64_t call_count;   ///< Total number of calls recorded.
  uint64_t total_ticks;  ///< Sum of (exit_timestamp - enter_timestamp) over all calls.
};

// ── Cold-path backend interface ────────────────────────────────────────────────

/// @brief Backend that consumes aggregated per-function statistics.
///
/// Implement this interface to build a concrete analysis backend
/// (call-graph, flame graph, histogram, …).
/// This interface is NOT on the hot path — it is called once per Flush().
class IProfiler {
 public:
  virtual ~IProfiler() = default;

  /// @brief Process aggregated statistics for all recorded functions.
  /// @param stats View over per-function stats collected since the last flush.
  virtual void Analyze(std::span<const FuncStat> stats) = 0;

  /// @brief Human-readable name of this backend (used for diagnostics).
  virtual const char* Name() const noexcept = 0;
};

// ── Public API ─────────────────────────────────────────────────────────────────

/// @brief Optionally override the default table capacity before recording starts.
///
/// The table is initialised automatically before main() with a default capacity
/// of 4096 unique functions. Call this function only if you need a different size.
/// Must be called before any instrumented code runs.
/// @param max_unique_functions Expected upper bound on unique instrumented functions.
void StartRecording(std::size_t max_unique_functions) noexcept;

/// @brief Flush aggregated stats to \p profiler and reset all counters.
/// @param profiler Backend that will receive and analyse the stats.
void Flush(IProfiler& profiler) noexcept;

/// @brief Disable instrumentation and release the table.
void StopRecording() noexcept;

/// @brief Set the backend that is called automatically at program exit.
///
/// By default a \c PrintProfiler writing to stdout is used.
/// Pass \c nullptr to disable the automatic flush entirely.
/// The pointer must remain valid until program exit.
/// @param profiler Backend to use for the automatic flush, or nullptr to disable.
void SetAutoFlush(IProfiler* profiler) noexcept;

}  // namespace malevrovich_prof
