cmake_minimum_required(VERSION 3.20)
project(malevrovich-prof VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── cmake helpers ──────────────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# ── sub-projects ───────────────────────────────────────────────────────────────
add_subdirectory(libs/malevrovich_prof)
add_subdirectory(example)
add_subdirectory(examples)
