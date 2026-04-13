cmake_minimum_required(VERSION 3.20)
project(malevrovich_prof_examples LANGUAGES CXX)

# ── Acquire the library ────────────────────────────────────────────────────────
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(UseMalevrovichProf)
setup_malevrovich_prof(PATH "${CMAKE_CURRENT_LIST_DIR}/..")

# ── Helper: add an instrumented example executable ────────────────────────────
function(add_example name dir)
    add_executable(${name} ${dir}/main.cpp)
    target_enable_malevrovich_prof(${name})
endfunction()

# ── Examples ──────────────────────────────────────────────────────────────────
add_example(ex01_basic             01_basic)
add_example(ex02_sorting           02_sorting)
add_example(ex03_matrix            03_matrix)
add_example(ex04_string_processing 04_string_processing)
