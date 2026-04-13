#pragma once

#include <cstdint>
#include <memory>

namespace malevrovich_prof {

struct CallRecord {
  void* call_site;
  void* callee;
  uint64_t timestamp;
};

/// @brief Abstract profiler backend interface.
///
/// Implement this interface to create a custom profiler backend.
/// Register an instance via SetActiveProfiler() to activate it.
class IProfiler {
 public:
  virtual ~IProfiler() = default;

  /// @brief Called on every instrumented function entry.
  /// @param record Call-site metadata captured by the instrumentation hook.
  virtual void OnEnter(const CallRecord& record) = 0;

  /// @brief Called on every instrumented function exit.
  /// @param record Call-site metadata captured by the instrumentation hook.
  virtual void OnExit(const CallRecord& record) = 0;

  /// @brief Flush collected data and write the profiling report.
  virtual void Flush() = 0;

  /// @brief Human-readable name of this backend (used for diagnostics).
  virtual const char* Name() const noexcept = 0;
};

/// @brief Returns a non-owning pointer to the active profiler, or nullptr.
IProfiler* GetActiveProfiler() noexcept;

/// @brief Sets the active profiler backend, transferring ownership.
/// @param profiler Owning pointer to the backend. Pass nullptr to disable profiling.
void SetActiveProfiler(std::unique_ptr<IProfiler> profiler) noexcept;

}  // namespace malevrovich_prof
