#include "malevrovich_prof/profiler.h"

#include "detail/table.h"
#include "malevrovich_prof/print_profiler.h"

#include <cstddef>
#include <vector>

namespace malevrovich_prof {

// ── Global aggregation table ───────────────────────────────────────────────────

static detail::StatsTable g_table;

namespace detail {

StatsTable& GetTable() noexcept {
  return g_table;
}

}  // namespace detail

// ── Auto-flush backend ─────────────────────────────────────────────────────────

// Non-owning pointer to the backend used by AutoFlush().
// Default: the static PrintProfiler below (set in AutoInit).
static IProfiler* g_auto_flush_backend = nullptr;

// ── Auto-initialise / auto-flush via constructor/destructor attributes ─────────

__attribute__((constructor)) static void AutoInit() noexcept {
  g_table.reserve(4096);
  // Install the default auto-flush backend (PrintProfiler to stdout).
  // Stored as a static local so it lives until the destructor runs.
  static PrintProfiler s_default_printer;
  g_auto_flush_backend = &s_default_printer;
}

__attribute__((destructor)) static void AutoFlush() noexcept {
  if (g_auto_flush_backend != nullptr) {
    Flush(*g_auto_flush_backend);
  }
}

// ── Public API ─────────────────────────────────────────────────────────────────

void StartRecording(std::size_t max_unique_functions) noexcept {
  g_table.clear();
  g_table.reserve(max_unique_functions);
}

void Flush(IProfiler& profiler) noexcept {
  static std::vector<FuncStat> batch;
  batch.clear();
  batch.reserve(g_table.size());

  for (const auto& [fn, s] : g_table) {
    batch.push_back({fn, s.call_count, s.total_ticks});
  }

  if (!batch.empty()) {
    profiler.Analyze(batch);
  }
}

void StopRecording() noexcept {
  g_table.clear();
}

void SetAutoFlush(IProfiler* profiler) noexcept {
  g_auto_flush_backend = profiler;
}

}  // namespace malevrovich_prof
