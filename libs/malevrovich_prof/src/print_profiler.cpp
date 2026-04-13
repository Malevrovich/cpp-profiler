#include "malevrovich_prof/print_profiler.h"

#include "detail/table.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// dladdr + __cxa_demangle are available on macOS and Linux (POSIX).
#if defined(__APPLE__) || defined(__linux__)
#include <cxxabi.h>
#include <dlfcn.h>
#define MALEVROVICH_PROF_HAS_DLADDR 1
#endif

namespace malevrovich_prof {

namespace {

// ── Symbol resolution ──────────────────────────────────────────────────────────

/// Resolve \p fn to a human-readable (demangled) symbol name.
/// Returns empty string if resolution is not supported or failed.
std::string ResolveSymbol(void* fn) {
#if defined(MALEVROVICH_PROF_HAS_DLADDR)
  Dl_info info{};
  if (::dladdr(fn, &info) == 0 || info.dli_sname == nullptr) {
    return {};
  }
  int status = 0;
  char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
  if (status == 0 && demangled != nullptr) {
    std::string result(demangled);
    std::free(demangled);
    return result;
  }
  return std::string(info.dli_sname);
#else
  (void)fn;
  return {};
#endif
}

// ── Tick → time conversion ─────────────────────────────────────────────────────

struct TimeValue {
  double value;
  const char* unit;  // "ns", "µs", "ms", or "s"
};

/// Convert \p ticks to a human-readable time value.
/// Returns {0, "ns"} when ticks_per_sec is 0 (calibration unavailable).
TimeValue TicksToTime(uint64_t ticks, uint64_t ticks_per_sec) noexcept {
  if (ticks_per_sec == 0) {
    return {0.0, "ns"};
  }
  const double ns = static_cast<double>(ticks) * 1e9 / static_cast<double>(ticks_per_sec);
  if (ns < 1e3) {
    return {ns, "ns"};
  }
  if (ns < 1e6) {
    return {ns / 1e3, "µs"};
  }
  if (ns < 1e9) {
    return {ns / 1e6, "ms"};
  }
  return {ns / 1e9, "s"};
}

}  // namespace

// ── PrintProfiler ──────────────────────────────────────────────────────────────

PrintProfiler::PrintProfiler(std::FILE* out) noexcept : out_(out) {}

void PrintProfiler::Analyze(std::span<const FuncStat> stats) {
  // Sort by total_ticks descending — hottest functions first.
  std::vector<FuncStat> sorted(stats.begin(), stats.end());
  std::sort(sorted.begin(), sorted.end(),
            [](const FuncStat& a, const FuncStat& b) { return a.total_ticks > b.total_ticks; });

  const uint64_t tps = detail::GetTicksPerSec();

  // Choose a single time unit for the whole table based on the hottest row.
  const char* time_unit = "ns";
  if (!sorted.empty() && tps > 0) {
    time_unit = TicksToTime(sorted.front().total_ticks, tps).unit;
  }

  // Pre-resolve symbol names and compute the name column width.
  std::vector<std::string> names;
  names.reserve(sorted.size());
  std::size_t name_col = std::strlen("function");
  for (const auto& s : sorted) {
    names.push_back(ResolveSymbol(s.fn));
    if (!names.back().empty()) {
      name_col = std::max(name_col, names.back().size());
    }
  }
  constexpr std::size_t kMaxNameCol = 60;
  name_col = std::min(name_col, kMaxNameCol);

  // ── Header ─────────────────────────────────────────────────────────────────
  const std::string time_header = std::string("time (") + time_unit + ")";
  std::fprintf(out_, "%-18s  %-*s  %10s  %16s  %16s\n", "address", static_cast<int>(name_col),
               "function", "calls", "total ticks", time_header.c_str());
  std::fprintf(out_, "%-18s  %-*s  %10s  %16s  %16s\n", "------------------",
               static_cast<int>(name_col), std::string(name_col, '-').c_str(), "----------",
               "----------------", "----------------");

  // ── Rows ───────────────────────────────────────────────────────────────────
  for (std::size_t i = 0; i < sorted.size(); ++i) {
    const auto& s = sorted[i];
    const std::string& raw_name = names[i];

    // Truncate long names with an ellipsis.
    std::string display_name;
    if (raw_name.empty()) {
      display_name = "?";
    } else if (raw_name.size() <= name_col) {
      display_name = raw_name;
    } else {
      display_name = raw_name.substr(0, name_col - 1) + "…";
    }

    // Convert ticks to the chosen unit.
    double time_val = 0.0;
    if (tps > 0) {
      const double ns = static_cast<double>(s.total_ticks) * 1e9 / static_cast<double>(tps);
      if (time_unit[0] == 'n') {
        time_val = ns;
      } else if (time_unit[0] == '\xc2') {
        // "µs" starts with the UTF-8 sequence 0xC2 0xB5
        time_val = ns / 1e3;
      } else if (time_unit[0] == 'm') {
        time_val = ns / 1e6;
      } else {
        time_val = ns / 1e9;
      }
    }

    std::fprintf(out_, "%18p  %-*s  %10llu  %16llu  %16.3f\n", s.fn, static_cast<int>(name_col),
                 display_name.c_str(), static_cast<unsigned long long>(s.call_count),
                 static_cast<unsigned long long>(s.total_ticks), time_val);
  }
}

const char* PrintProfiler::Name() const noexcept {
  return "PrintProfiler";
}

}  // namespace malevrovich_prof
