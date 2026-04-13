cmake_minimum_required(VERSION 3.20)
project(comparison_gprof LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(workload_gprof
    ${CMAKE_CURRENT_LIST_DIR}/../main.cpp
    ${CMAKE_CURRENT_LIST_DIR}/../workload.cpp
)

target_include_directories(workload_gprof PRIVATE ${CMAKE_CURRENT_LIST_DIR}/..)

# gprof requires -pg at both compile and link time.
target_compile_options(workload_gprof PRIVATE -pg -g -O2)
target_link_options(workload_gprof PRIVATE -pg)
