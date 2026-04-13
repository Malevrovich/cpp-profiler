#include "malevrovich_prof/print_profiler.h"

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

// dladdr is available on macOS and Linux (POSIX).
#if defined(__APPLE__) || defined(__linux__)
#include <cxxabi.h>
#include <dlfcn.h>
#define MALEVROVICH_PROF_HAS_DLADDR 1
#endif

namespace malevrovich_prof {

namespace {

/// Resolve \p fn to a human-readable symbol name.
/// Returns the demangled C++ name when available, the raw mangled name as a
/// fallback, or an empty string if resolution is not supported / failed.
std::string ResolveSymbol(void* fn) {
#if defined(MALEVROVICH_PROF_HAS_DLADDR)
  Dl_info info{};
  if (::dladdr(fn, &info) == 0 || info.dli_sname == nullptr) {
    return {};
  }

  // Attempt C++ demangling.
  int status = 0;
  char* demangled = abi::__cxa_demangle(info.dli_sname, nullptr, nullptr, &status);
  if (status == 0 && demangled != nullptr) {
    std::string result(demangled);
    std::free(demangled);
    return result;
  }

  // Return raw mangled name as-is.
  return std::string(info.dli_sname);
#else
  (void)fn;
  return {};
#endif
}

}  // namespace

PrintProfiler::PrintProfiler(std::FILE* out) noexcept : out_(out) {}

void PrintProfiler::Analyze(std::span<const FuncStat> stats) {
  // Sort by total_ticks descending — hottest functions first.
  std::vector<FuncStat> sorted(stats.begin(), stats.end());
  std::sort(sorted.begin(), sorted.end(),
            [](const FuncStat& a, const FuncStat& b) { return a.total_ticks > b.total_ticks; });

  // Pre-resolve all symbol names so we can compute the column width.
  std::vector<std::string> names;
  names.reserve(sorted.size());
  std::size_t name_col = std::strlen("function");  // minimum header width
  for (const auto& s : sorted) {
    names.push_back(ResolveSymbol(s.fn));
    if (!names.back().empty()) {
      name_col = std::max(name_col, names.back().size());
    }
  }
  // Cap at a reasonable terminal width to avoid wrapping.
  constexpr std::size_t kMaxNameCol = 60;
  name_col = std::min(name_col, kMaxNameCol);

  // Header
  std::fprintf(out_, "%-18s  %-*s  %10s  %16s\n", "address", static_cast<int>(name_col), "function",
               "calls", "total ticks");
  std::fprintf(out_, "%-18s  %-*s  %10s  %16s\n", "------------------", static_cast<int>(name_col),
               std::string(name_col, '-').c_str(), "----------", "----------------");

  for (std::size_t i = 0; i < sorted.size(); ++i) {
    const auto& s = sorted[i];
    const std::string& name = names[i];

    if (name.empty()) {
      // No symbol info — print address only in the name column.
      std::fprintf(out_, "%18p  %-*s  %10llu  %16llu\n", s.fn, static_cast<int>(name_col), "?",
                   static_cast<unsigned long long>(s.call_count),
                   static_cast<unsigned long long>(s.total_ticks));
    } else if (name.size() <= name_col) {
      std::fprintf(out_, "%18p  %-*s  %10llu  %16llu\n", s.fn, static_cast<int>(name_col),
                   name.c_str(), static_cast<unsigned long long>(s.call_count),
                   static_cast<unsigned long long>(s.total_ticks));
    } else {
      // Truncate long names with an ellipsis.
      std::string truncated = name.substr(0, name_col - 1) + "…";
      std::fprintf(out_, "%18p  %-*s  %10llu  %16llu\n", s.fn, static_cast<int>(name_col),
                   truncated.c_str(), static_cast<unsigned long long>(s.call_count),
                   static_cast<unsigned long long>(s.total_ticks));
    }
  }
}

const char* PrintProfiler::Name() const noexcept {
  return "PrintProfiler";
}

}  // namespace malevrovich_prof
