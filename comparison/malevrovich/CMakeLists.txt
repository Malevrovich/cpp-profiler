cmake_minimum_required(VERSION 3.20)
project(comparison_malevrovich LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Acquire the library ────────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake")
include(UseMalevrovichProf)
setup_malevrovich_prof(PATH "${CMAKE_CURRENT_LIST_DIR}/../..")

# ── Build and instrument the workload ─────────────────────────────────────────
add_executable(workload_malevrovich
    ${CMAKE_CURRENT_LIST_DIR}/../main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../workload.cpp
)

target_include_directories(workload_malevrovich PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)

target_enable_malevrovich_prof(workload_malevrovich)
