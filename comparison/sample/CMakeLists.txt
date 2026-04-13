cmake_minimum_required(VERSION 3.20)
project(comparison_sample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# `sample` is a macOS built-in sampling profiler — no special compile flags needed.
# Build with -g so symbol names are resolved in the sample output.

# Single-run binary (same as other comparison targets).
add_executable(workload_sample
    ${CMAKE_CURRENT_LIST_DIR}/../main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../workload.cpp
)
target_include_directories(workload_sample PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)
target_compile_options(workload_sample PRIVATE -g -O2)

# Loop binary — runs the workload for ~10 s so `sample` has time to collect data.
add_executable(workload_sample_loop
    ${CMAKE_CURRENT_LIST_DIR}/main_loop.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../workload.cpp
)
target_include_directories(workload_sample_loop PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)
target_compile_options(workload_sample_loop PRIVATE -g -O2)
