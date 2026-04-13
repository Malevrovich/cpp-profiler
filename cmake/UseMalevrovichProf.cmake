# UseMalevrovichProf.cmake
# ──────────────────────────────────────────────────────────────────────────────
# Two-step API that mirrors the gprof workflow:
#
#   Step 1 – acquire the library (call once, near the top of your root CMakeLists):
#
#     # Option A – local source tree
#     setup_malevrovich_prof(PATH /path/to/malevrovich-prof)
#
#     # Option B – download from GitHub
#     setup_malevrovich_prof(
#         GIT_REPOSITORY https://github.com/your-org/malevrovich-prof.git
#         GIT_TAG        main)
#
#   Step 2 – instrument a target (call once per target):
#
#     target_enable_malevrovich_prof(my_executable)
#
# ──────────────────────────────────────────────────────────────────────────────

include(FetchContent)

# ── setup_malevrovich_prof([PATH <dir>] | [GIT_REPOSITORY <url> GIT_TAG <tag>]) ─
#
# Makes malevrovich_prof::malevrovich_prof available in the current build.
# Must be called before any target_enable_malevrovich_prof() call.
#
function(setup_malevrovich_prof)
    # ── parse arguments ────────────────────────────────────────────────────────
    cmake_parse_arguments(
        MP                              # prefix
        ""                              # options  (flags with no value)
        "PATH;GIT_REPOSITORY;GIT_TAG"   # one-value keywords
        ""                              # multi-value keywords
        ${ARGN}
    )

    if(TARGET malevrovich_prof::malevrovich_prof)
        # Already set up (e.g. called twice, or library is part of the same tree).
        return()
    endif()

    if(DEFINED MP_PATH)
        # ── Option A: local directory ──────────────────────────────────────────
        if(NOT IS_DIRECTORY "${MP_PATH}")
            message(FATAL_ERROR
                "setup_malevrovich_prof: PATH \"${MP_PATH}\" is not a directory.")
        endif()

        add_subdirectory("${MP_PATH}"
                         "${CMAKE_BINARY_DIR}/_malevrovich_prof"
                         EXCLUDE_FROM_ALL)

    elseif(DEFINED MP_GIT_REPOSITORY)
        # ── Option B: fetch from GitHub / any Git remote ───────────────────────
        if(NOT DEFINED MP_GIT_TAG)
            set(MP_GIT_TAG "main")
            message(WARNING
                "setup_malevrovich_prof: GIT_TAG not specified, defaulting to \"main\".")
        endif()

        FetchContent_Declare(
            malevrovich_prof_fetch
            GIT_REPOSITORY "${MP_GIT_REPOSITORY}"
            GIT_TAG        "${MP_GIT_TAG}"
            GIT_SHALLOW    TRUE
        )
        FetchContent_MakeAvailable(malevrovich_prof_fetch)

    else()
        message(FATAL_ERROR
            "setup_malevrovich_prof: you must provide either PATH or GIT_REPOSITORY.")
    endif()

    # Sanity-check: the sub-project must have defined the alias target.
    if(NOT TARGET malevrovich_prof::malevrovich_prof)
        message(FATAL_ERROR
            "setup_malevrovich_prof: the sub-project did not define "
            "malevrovich_prof::malevrovich_prof. "
            "Check that libs/malevrovich_prof/CMakeLists.txt is present and correct.")
    endif()
endfunction()


# ── target_enable_malevrovich_prof(<target>) ──────────────────────────────────
#
# Links malevrovich_prof::malevrovich_prof and adds -finstrument-functions
# to <target>.  setup_malevrovich_prof() must have been called first.
#
function(target_enable_malevrovich_prof target)
    if(NOT TARGET malevrovich_prof::malevrovich_prof)
        message(FATAL_ERROR
            "target_enable_malevrovich_prof: malevrovich_prof::malevrovich_prof "
            "not found. Call setup_malevrovich_prof() before "
            "target_enable_malevrovich_prof().")
    endif()

    target_link_libraries(${target} PRIVATE malevrovich_prof::malevrovich_prof)

    target_compile_options(${target} PRIVATE
        -finstrument-functions
        # Uncomment for richer debug symbols in reports:
        # -g
        # -O0
    )
endfunction()
