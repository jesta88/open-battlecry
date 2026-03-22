# FetchSDL3.cmake
#
# Ensures an imported target SDL3::SDL3 exists.
# - Prefer a system-installed SDL3 when RECON_USE_SYSTEM_SDL3=ON.
# - Otherwise FetchContent a pinned SDL3 release (shared library only).
#
# Customize the tag with RECON_SDL3_GIT_TAG (release tag or commit SHA).

# If someone already found/added SDL3, do nothing.
if (TARGET SDL3::SDL3)
    return()
endif()

# ---- Options ---------------------------------------------------------------

# Default: use system SDL3 on non-Windows, fetch on Windows (overridable).
if (NOT DEFINED RECON_USE_SYSTEM_SDL3)
    if (WIN32)
        set(RECON_USE_SYSTEM_SDL3 OFF CACHE BOOL "Prefer system-installed SDL3 if available")
    else()
        set(RECON_USE_SYSTEM_SDL3 ON  CACHE BOOL "Prefer system-installed SDL3 if available")
    endif()
endif()

set(RECON_SDL3_GIT_TAG "release-3.2.20" CACHE STRING "SDL3 release tag or commit hash to fetch")

# ---- Try system SDL3 first (optional) --------------------------------------

if (RECON_USE_SYSTEM_SDL3)
    # CONFIG mode first (package config), fall back to MODULE if present.
    find_package(SDL3 QUIET CONFIG)
    if (TARGET SDL3::SDL3)
        message(STATUS "[SDL3] Using system-installed SDL3 (CONFIG)")
        return()
    endif()

    find_package(SDL3 QUIET)
    if (TARGET SDL3::SDL3)
        message(STATUS "[SDL3] Using system-installed SDL3 (MODULE)")
        return()
    endif()

    message(STATUS "[SDL3] System SDL3 not found; falling back to FetchContent.")
endif()

# ---- Fetch SDL3 (shared only) ---------------------------------------------

include(FetchContent)

# Keep pinned revisions stable unless you explicitly change RECON_SDL3_GIT_TAG
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        ${RECON_SDL3_GIT_TAG}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   FALSE
)

# Configure SDL's CMake options via cache entries BEFORE MakeAvailable.
# Shared-only; no tests/examples/install.
set(SDL_SHARED           ON  CACHE BOOL "" FORCE)
set(SDL_STATIC           OFF CACHE BOOL "" FORCE)
set(SDL_TESTS            OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY     OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES         OFF CACHE BOOL "" FORCE)
set(SDL_DISABLE_INSTALL  ON  CACHE BOOL "" FORCE)

# set(SDL_HAPTIC         OFF CACHE BOOL "" FORCE)
# set(SDL_JOYSTICK       OFF CACHE BOOL "" FORCE)
# set(SDL_HIDAPI         OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SDL3)

if (NOT TARGET SDL3::SDL3)
    message(FATAL_ERROR "[SDL3] Expected target SDL3::SDL3 was not created by the fetched project.")
endif()

message(STATUS "[SDL3] Using fetched SDL3 @ ${RECON_SDL3_GIT_TAG} (shared)")
