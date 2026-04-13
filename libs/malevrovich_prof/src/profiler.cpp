#include "malevrovich_prof/profiler.h"

#include <memory>

namespace malevrovich_prof {

IProfiler* GetActiveProfiler() noexcept {
  return nullptr;
}

void SetActiveProfiler(std::unique_ptr<IProfiler> /*profiler*/) noexcept {}

}  // namespace malevrovich_prof
