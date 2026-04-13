# malevrovich-prof

A lightweight C++ profiler library based on `-finstrument-functions`.  
It aggregates per-function call counts and CPU-tick totals with near-zero overhead on the hot path, then prints a sorted report at program exit — no manual instrumentation required.

---

## Features

- **Zero-touch profiling** — stats are printed automatically when the program exits
- **Hot-path aggregation** — uses a flat hash map (`ankerl::unordered_dense`) directly in the enter/exit hooks; no queues, no locks
- **CPU timestamp counters** — `rdtsc` on x86-64, `cntvct_el0` on ARM64
- **Symbol resolution** — `dladdr` + `abi::__cxa_demangle` resolve and demangle function names at report time (macOS / Linux)
- **Custom backends** — implement `IProfiler` to build flame graphs, call trees, histograms, etc.
- **CMake helper** — `UseMalevrovichProf.cmake` integrates the library in two lines

---

## Requirements

| Requirement | Version |
|---|---|
| CMake | ≥ 3.20 |
| C++ standard | C++20 |
| Compiler | GCC or Clang (needs `-finstrument-functions`) |
| Platform | macOS or Linux |

---

## Quick start

### 1. Add the library to your CMake project

Copy `cmake/UseMalevrovichProf.cmake` into your project's `cmake/` directory, then in your root `CMakeLists.txt`:

```cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UseMalevrovichProf)

# Option A — local copy of the library
setup_malevrovich_prof(PATH /path/to/malevrovich-prof)

# Option B — fetch from GitHub automatically
# setup_malevrovich_prof(
#     GIT_REPOSITORY https://github.com/your-org/malevrovich-prof.git
#     GIT_TAG        main
# )
```

### 2. Instrument your target

```cmake
add_executable(my_app main.cpp)
target_enable_malevrovich_prof(my_app)
```

`target_enable_malevrovich_prof` does two things:
- links `malevrovich_prof::malevrovich_prof`
- adds `-finstrument-functions` to the target's compile options

### 3. Write your code — nothing else needed

```cpp
// main.cpp
void HeavyWork() { /* ... */ }

int main() {
    HeavyWork();
    // Stats are printed to stdout automatically at exit.
    return 0;
}
```

Build and run:

```
$ cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
$ cmake --build build
$ ./build/my_app

address             function            calls       total ticks         time (ms)
------------------  ------------------  ----------  ----------------  ----------------
      0x100168ec8   HeavyWork()              42000          12345678            12.346
      0x100168e50   main                         1             54321             0.054
```

The `time` column is auto-scaled to the most readable unit (`ns`, `µs`, `ms`, or `s`) based on the hottest row. All rows use the same unit so values are directly comparable.

---

## API reference

All symbols live in the `malevrovich_prof` namespace.  
Include `"malevrovich_prof/profiler.h"` for the core API.

### `FuncStat`

```cpp
struct FuncStat {
    void*    fn;           // Function address
    uint64_t call_count;   // Total number of calls recorded
    uint64_t total_ticks;  // Sum of (exit_tick − enter_tick) over all calls
};
```

### `IProfiler`

```cpp
class IProfiler {
public:
    virtual ~IProfiler() = default;
    virtual void Analyze(std::span<const FuncStat> stats) = 0;
    virtual const char* Name() const noexcept = 0;
};
```

Implement this interface to create a custom analysis backend.  
`Analyze` is called **once per `Flush()`** — it is not on the hot path.

### `StartRecording(max_unique_functions)`

```cpp
void StartRecording(std::size_t max_unique_functions) noexcept;
```

Optional. Pre-allocates the internal hash table for `max_unique_functions` entries and resets all counters.  
The table is initialised automatically before `main()` with a default capacity of **4096** unique functions.  
Call this only if you expect more unique functions or want to reset counters mid-run.

### `Flush(profiler)`

```cpp
void Flush(IProfiler& profiler) noexcept;
```

Passes all aggregated stats to `profiler.Analyze()` and resets the counters.  
Can be called multiple times during a run to get incremental snapshots.

### `StopRecording()`

```cpp
void StopRecording() noexcept;
```

Clears the table and stops accumulating stats.  
Useful when you want to profile only a specific section of the program.

### `SetAutoFlush(profiler)`

```cpp
void SetAutoFlush(IProfiler* profiler) noexcept;
```

Sets the backend used for the **automatic flush at program exit**.  
By default a `PrintProfiler` writing to `stdout` is used.  
Pass `nullptr` to disable the automatic flush entirely.  
The pointer must remain valid until program exit.

---

## Built-in `PrintProfiler`

Include `"malevrovich_prof/print_profiler.h"`.

```cpp
class PrintProfiler : public IProfiler {
public:
    explicit PrintProfiler(std::FILE* out = stdout) noexcept;
    void Analyze(std::span<const FuncStat> stats) override;
    const char* Name() const noexcept override;
};
```

Sorts rows by `total_ticks` descending (hottest functions first) and prints a formatted table.  
Function addresses are resolved to demangled C++ names via `dladdr` + `abi::__cxa_demangle` where available.

---

## Usage scenarios

### Default — zero configuration

```cpp
int main() {
    DoWork();
    // PrintProfiler fires automatically at exit.
}
```

### Redirect output to a file

```cpp
#include "malevrovich_prof/print_profiler.h"
#include "malevrovich_prof/profiler.h"

int main() {
    std::FILE* f = std::fopen("profile.txt", "w");
    static malevrovich_prof::PrintProfiler file_printer(f);
    malevrovich_prof::SetAutoFlush(&file_printer);

    DoWork();
    // Stats written to profile.txt at exit.
    std::fclose(f);
}
```

### Disable auto-flush, flush manually

```cpp
#include "malevrovich_prof/profiler.h"
#include "malevrovich_prof/print_profiler.h"

int main() {
    malevrovich_prof::SetAutoFlush(nullptr);  // disable auto-flush

    Phase1();
    malevrovich_prof::PrintProfiler p;
    malevrovich_prof::Flush(p);               // snapshot after Phase1

    malevrovich_prof::StartRecording(4096);   // reset counters
    Phase2();
    malevrovich_prof::Flush(p);               // snapshot after Phase2
}
```

### Custom backend

```cpp
#include "malevrovich_prof/profiler.h"
#include <span>

class MyBackend : public malevrovich_prof::IProfiler {
public:
    void Analyze(std::span<const malevrovich_prof::FuncStat> stats) override {
        for (const auto& s : stats) {
            // write to database, send over network, build flame graph, …
        }
    }
    const char* Name() const noexcept override { return "MyBackend"; }
};

int main() {
    static MyBackend backend;
    malevrovich_prof::SetAutoFlush(&backend);
    DoWork();
}
```

### Profile only a specific section

```cpp
int main() {
    malevrovich_prof::SetAutoFlush(nullptr);  // no auto-flush

    warmup();

    malevrovich_prof::StartRecording(1024);   // reset + start fresh
    hot_section();
    malevrovich_prof::PrintProfiler p;
    malevrovich_prof::Flush(p);

    malevrovich_prof::StopRecording();        // stop accumulating
    cleanup();
}
```

---

## Project layout

```
malevrovich-prof/
├── cmake/
│   └── UseMalevrovichProf.cmake        # CMake integration helper
├── libs/
│   └── malevrovich_prof/
│       ├── CMakeLists.txt
│       ├── include/
│       │   └── malevrovich_prof/
│       │       ├── profiler.h          # Public API (FuncStat, IProfiler, Flush, …)
│       │       ├── print_profiler.h    # Built-in PrintProfiler
│       │       └── instrumentation.h  # extern "C" hook declarations
│       └── src/
│           ├── profiler.cpp            # Global table, auto-init/flush, public API
│           ├── instrumentation.cpp     # Hot-path __cyg_profile_func_enter/exit
│           ├── print_profiler.cpp      # PrintProfiler implementation
│           └── detail/
│               ├── clock.h            # Internal: ReadTimestamp + MeasureTicksPerSecond
│               └── table.h            # Internal: FuncStats + StatsTable (not public)
└── example/
    ├── CMakeLists.txt
    └── main.cpp
```

---

## How it works

1. **`-finstrument-functions`** — the compiler injects calls to `__cyg_profile_func_enter(fn, call_site)` and `__cyg_profile_func_exit(fn, call_site)` at every function entry and exit in the instrumented target.

2. **Hot-path hooks** (`instrumentation.cpp`) — on enter, the current CPU timestamp (`rdtsc` / `cntvct_el0`) is pushed onto a small per-function inline stack and the call counter is incremented. On exit, the timestamp is popped and `total_ticks += exit_tick − enter_tick`. All state lives in an `ankerl::unordered_dense::map<void*, FuncStats>` that is pre-allocated before `main()`.

3. **`__attribute__((constructor))`** — `AutoInit()` runs before `main()`, reserves the hash table, and calibrates the CPU frequency (see step 6).

4. **`__attribute__((destructor))`** — `AutoFlush()` runs after `main()` returns and calls `Flush(*g_auto_flush_backend)` if a backend is set.

5. **`Flush()`** — iterates the hash map once, builds a `std::vector<FuncStat>`, and passes it to `IProfiler::Analyze()`. The backend is entirely off the hot path.

6. **CPU frequency calibration** (`detail/clock.h`) — `MeasureTicksPerSecond()` samples both `clock_gettime(CLOCK_MONOTONIC)` and the TSC/counter over a ~5 ms busy-wait to compute ticks-per-second. This value is stored globally and used at report time to convert raw ticks to wall-clock time.

7. **Symbol resolution & time formatting** — `PrintProfiler::Analyze()` calls `dladdr()` on each address to get the nearest symbol name, then `abi::__cxa_demangle()` to produce a readable C++ name. The `time` column is auto-scaled to `ns`/`µs`/`ms`/`s` based on the hottest row so all values share the same unit.
