#pragma once

#include "malevrovich_prof/profiler.h"

#include <cstdio>
#include <span>

namespace malevrovich_prof {

/// @brief Built-in IProfiler that prints a sorted stats table to a FILE stream.
///
/// Rows are sorted by total_ticks descending so the hottest functions appear
/// at the top. Output goes to \p out (default: stdout).
///
/// Usage:
/// @code
///   malevrovich_prof::PrintProfiler printer;   // prints to stdout
///   malevrovich_prof::Flush(printer);
/// @endcode
class PrintProfiler : public IProfiler {
 public:
  /// @brief Construct a printer that writes to \p out.
  /// @param out Destination FILE stream. Must remain valid for the lifetime of
  ///            this object. Defaults to stdout.
  explicit PrintProfiler(std::FILE* out = stdout) noexcept;

  void Analyze(std::span<const FuncStat> stats) override;

  const char* Name() const noexcept override;

 private:
  std::FILE* out_;
};

}  // namespace malevrovich_prof
