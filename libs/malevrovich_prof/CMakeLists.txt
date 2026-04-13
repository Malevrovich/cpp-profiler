add_library(malevrovich_prof STATIC
    src/profiler.cpp
    src/instrumentation.cpp
)

# Public include dir: consumers do  #include "malevrovich_prof/profiler.h"
target_include_directories(malevrovich_prof
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(malevrovich_prof PUBLIC cxx_std_17)

# Alias so consumers can write  target_link_libraries(my_target malevrovich_prof::malevrovich_prof)
add_library(malevrovich_prof::malevrovich_prof ALIAS malevrovich_prof)
