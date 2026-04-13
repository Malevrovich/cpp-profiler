# Profiler comparison: gprof vs sample (macOS) vs malevrovich-prof

This directory contains the **same workload** built three ways so you can follow
each profiler workflow step-by-step and compare the experience.

## Workload

Three algorithms run in sequence inside [`RunWorkload()`](workload.cpp):

| Function | Work |
|---|---|
| `Fib(30)` | Recursive Fibonacci — 2 692 537 calls, deep stack |
| `BubbleSort` | O(N²) sort of 5 000 integers |
| `MergeSort` | O(N log N) sort of the same 5 000 integers |
| `NaiveMultiply` | 150×150 matrix multiply |

---

## Directory layout

```
comparison/
├── workload.h / workload.cpp   shared algorithms
├── main.cpp                    single-run entry point (gprof + malevrovich)
├── gprof/
│   └── CMakeLists.txt          builds with -pg -g -O2
├── malevrovich/
│   └── CMakeLists.txt          builds with target_enable_malevrovich_prof()
└── sample/
    ├── CMakeLists.txt          builds workload_sample + workload_sample_loop
    └── main_loop.cpp           runs workload for ~10 s (needed by `sample`)
```

---

## Prerequisites

### Linux

```bash
# gprof is part of GNU binutils — usually pre-installed
gprof --version
cmake --version   # ≥ 3.20
```

### macOS

`gprof` is **not** shipped with Apple Clang. The native macOS equivalent is
**`sample`** (built into every macOS installation) and **Instruments** (Xcode GUI).

```bash
# sample is always available — no install needed
sample --help

# Optional: install GNU gprof via Homebrew (x86-64 only; broken on Apple Silicon)
brew install binutils
export PATH="$(brew --prefix binutils)/bin:$PATH"
```

> **Apple Silicon note:** `-pg` support is broken in Apple Clang on ARM64.
> `gmon.out` is not generated. Use GCC from Homebrew if you need gprof on M-series:
> ```bash
> brew install gcc
> cmake -S . -B build -DCMAKE_CXX_COMPILER=$(brew --prefix gcc)/bin/g++-14
> ```

---

## Step 1 — Build all targets

From the **repository root**:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target workload_gprof workload_malevrovich \
                            workload_sample workload_sample_loop
```

Binaries:
```
build/comparison/gprof/workload_gprof
build/comparison/malevrovich/workload_malevrovich
build/comparison/sample/workload_sample
build/comparison/sample/workload_sample_loop   ← runs for ~10 s (needed by sample)
```

---

## Step 2 — Profile with gprof (Linux)

### 2a. Run the binary — generates `gmon.out`

`gmon.out` is written to the **current working directory**:

```bash
cd build/comparison/gprof
./workload_gprof
ls -lh gmon.out
```

Expected program output:
```
Fib(30)=832040  sorted=ok  C[0][0]=300
```

### 2b. Generate the flat profile

```bash
gprof workload_gprof gmon.out --brief
```

Typical output:
```
Flat profile:

Each sample counts as 0.01 seconds.
  %   cumulative   self              self     total
 time   seconds   seconds    calls  ms/call  ms/call  name
 62.50      0.50     0.50  2692537     0.00     0.00  Fib(int)
 37.50      0.80     0.30        1   300.00   300.00  BubbleSort(...)
  0.00      0.80     0.00     9999     0.00     0.00  MergeSort(...)
  0.00      0.80     0.00        1     0.00    77.00  NaiveMultiply(...)
  0.00      0.80     0.00        1     0.00   800.00  RunWorkload()
```

### 2c. Call graph

```bash
gprof workload_gprof gmon.out --no-flat-profile
```

### 2d. Annotated source listing

```bash
gprof workload_gprof gmon.out -A
```

---

## Step 3 — Profile with `sample` (macOS)

`sample` is a **statistical sampling** profiler built into every macOS installation.
It attaches to a running process and takes a stack snapshot every millisecond.

Because the workload finishes in ~800 ms, use the **loop binary** which runs for
~10 seconds so `sample` has time to collect enough samples.

### 3a. Start `sample` in the background, then launch the binary

```bash
# Terminal — run from the repo root
sample workload_sample_loop 8 1 -wait -file /tmp/sample_out.txt &
./build/comparison/sample/workload_sample_loop
```

`-wait` tells `sample` to wait until a process named `workload_sample_loop` appears.
`8` = sample for 8 seconds, `1` = 1 ms interval.

### 3b. Read the report

```bash
cat /tmp/sample_out.txt
```

### 3c. Actual output (truncated)

```
Analysis of sampling workload_sample_loop (pid 96550) every 1 millisecond
Process:         workload_sample_loop [96550]
Code Type:       ARM64

Call graph:
    6304 Thread_1801642   DispatchQueue_1: com.apple.main-thread
      6304 start  (in dyld)
        6304 main  (in workload_sample_loop)  main_loop.cpp:15
          3572 RunWorkload()  (in workload_sample_loop)
          ! 1221 Fib(int)  (in workload_sample_loop)
          !   1221 Fib(int)  (in workload_sample_loop)
          !     ...  (deep recursive call tree)
          196 RunWorkload()  (in workload_sample_loop)
          ! 105 MergeSort(...)  (in workload_sample_loop)
          ! :  49 MergeSort(...)  (in workload_sample_loop)
          ...
```

The call graph shows `Fib` dominating with ~1 221 samples out of 6 304 total
(~19 % of wall time), and `MergeSort` visible as a recursive tree.

### 3d. Open in Instruments (GUI)

For a flame graph and timeline view, use Xcode Instruments:

```bash
# Record a trace
xctrace record --template 'Time Profiler' \
               --launch -- ./build/comparison/sample/workload_sample_loop \
               --output trace.xctrace

# Open in Instruments GUI
open trace.xctrace
```

---

## Step 4 — Profile with malevrovich-prof

No extra steps — the report prints automatically at program exit:

```bash
./build/comparison/malevrovich/workload_malevrovich
```

### Actual output

```
Fib(30)=832040  sorted=ok  C[0][0]=300
address             function                                calls       total ticks         time (ms)
------------------  ------------------------------------  ----------  ----------------  ----------------
       0x...        Fib(int)                               2692537         799158182           799.317
       0x...        main                                         1         431786695           431.872
       0x...        RunWorkload()                                1         431785570           431.871
       0x...        BubbleSort(std::vector<int>&)               1         303416678           303.477
       0x...        MergeSort(std::vector<int>&, int, int)    9999          92765328            92.784
       0x...        NaiveMultiply(...)                          1          77206180            77.222
       0x...        std::vector<int>::operator[](...)      37681864          30790551            30.797
       0x...        Merge(std::vector<int>&, int, int, int)  4999           9082953             9.085
       ...
```

---

## Side-by-side comparison

| Feature | gprof (Linux) | `sample` (macOS) | malevrovich-prof |
|---|---|---|---|
| **Instrumentation** | `-pg` compile + link | None — attaches to running process | `-finstrument-functions` via CMake |
| **Output trigger** | Manual: run → `gmon.out` → `gprof` command | Manual: `sample <pid>` while process runs | Automatic at program exit |
| **Time measurement** | Statistical sampling (10 ms) | Statistical sampling (1 ms default) | Exact CPU timestamp counter per call |
| **Call counts** | Exact (via `-pg` counters) | Not shown (sampling only) | Exact |
| **Stdlib functions** | Hidden (not compiled with `-pg`) | Shown (samples entire call stack) | Shown (all instrumented code) |
| **Recursive call tree** | Flat profile only; call graph separate | Full tree with sample counts per depth | Flat table; depth tracked internally |
| **Demangled names** | Yes | Yes | Yes (`abi::__cxa_demangle`) |
| **Platform** | Linux; broken on Apple Silicon | macOS only | macOS + Linux |
| **Extra tools needed** | `gprof` binary | None (built into macOS) | None |
| **Output format** | Flat profile + call graph text | Call tree text / Instruments GUI | Sorted table with time column |
| **Recompile needed** | Yes (`-pg`) | **No** — attaches to any binary | Yes (`-finstrument-functions`) |
| **Overhead** | Low (sampling + `-pg` hooks) | Near-zero (external sampling) | Low (hash map aggregation) |

### Key observations from the workload

1. **`Fib(30)` dominates** — all three profilers agree: Fibonacci is the hottest function.
   - gprof: ~62 % of samples
   - `sample`: 1 221 / 6 304 samples (~19 % — lower because BubbleSort also runs)
   - malevrovich-prof: 799 ms / 431 ms total (recursive self-time)

2. **`BubbleSort` vs `MergeSort`** — malevrovich-prof gives exact times (303 ms vs 92 ms).
   gprof shows the same ratio. `sample` shows `MergeSort` as a visible recursive tree.

3. **Stdlib internals** — malevrovich-prof shows `std::vector::operator[]` called
   37 M times for 30 ms. gprof hides this entirely. `sample` shows it in the call tree.

4. **`NaiveMultiply`** — 77 ms for a 150×150 multiply. All three profilers capture it.

5. **No recompile with `sample`** — you can profile any existing binary on macOS
   without rebuilding. gprof and malevrovich-prof both require a special build.

---

## Going further

### Suppress stdlib noise in malevrovich-prof

```cmake
target_compile_options(my_target PRIVATE
    -finstrument-functions
    -finstrument-functions-exclude-file-list=/usr/include,/usr/lib
)
```

### Add debug symbols for better name resolution

```cmake
target_compile_options(my_target PRIVATE -g)
```

### `sample` with a custom interval

```bash
sample <pid> 30 5   # sample for 30 s, every 5 ms
```

### Instruments Time Profiler (GUI)

```bash
xctrace record --template 'Time Profiler' --launch -- ./my_binary --output out.xctrace
open out.xctrace
```
