cmake_minimum_required(VERSION 3.20)
project(malevrovich_prof_example LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# ── Step 1: acquire the malevrovich-prof library ──────────────────────────────

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../cmake")
include(UseMalevrovichProf)

# Option A – local path (useful when this example lives inside the repo)
setup_malevrovich_prof(PATH "${CMAKE_CURRENT_LIST_DIR}/..")

# Option B – download from GitHub (comment out Option A and uncomment this)
# setup_malevrovich_prof(
#     GIT_REPOSITORY https://github.com/your-org/malevrovich-prof.git
#     GIT_TAG        main
# )

# ── Step 2: define your target and instrument it ──────────────────────────────

add_executable(example_app main.cpp)

target_enable_malevrovich_prof(example_app)
