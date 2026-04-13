include(FetchContent)

FetchContent_Declare(
  unordered_dense
  GIT_REPOSITORY https://github.com/martinus/unordered_dense.git
  GIT_TAG        main
  GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(unordered_dense)

add_library(malevrovich_prof STATIC
    src/profiler.cpp
    src/instrumentation.cpp
    src/print_profiler.cpp
)

target_include_directories(malevrovich_prof
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(malevrovich_prof PUBLIC unordered_dense::unordered_dense)

target_compile_features(malevrovich_prof PUBLIC cxx_std_20)

add_library(malevrovich_prof::malevrovich_prof ALIAS malevrovich_prof)
