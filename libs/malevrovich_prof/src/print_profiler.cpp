#include "malevrovich_prof/print_profiler.h"

#include <algorithm>
#include <vector>

namespace malevrovich_prof {

PrintProfiler::PrintProfiler(std::FILE* out) noexcept : out_(out) {}

void PrintProfiler::Analyze(std::span<const FuncStat> stats) {
  // Sort by total_ticks descending — hottest functions first.
  std::vector<FuncStat> sorted(stats.begin(), stats.end());
  std::sort(sorted.begin(), sorted.end(),
            [](const FuncStat& a, const FuncStat& b) { return a.total_ticks > b.total_ticks; });

  std::fprintf(out_, "%-18s  %10s  %16s\n", "address", "calls", "total ticks");
  std::fprintf(out_, "%-18s  %10s  %16s\n", "------------------", "----------", "----------------");
  for (const auto& s : sorted) {
    std::fprintf(out_, "%18p  %10llu  %16llu\n", s.fn,
                 static_cast<unsigned long long>(s.call_count),
                 static_cast<unsigned long long>(s.total_ticks));
  }
}

const char* PrintProfiler::Name() const noexcept {
  return "PrintProfiler";
}

}  // namespace malevrovich_prof
