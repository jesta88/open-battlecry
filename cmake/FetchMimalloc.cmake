# Ensures mimalloc::mimalloc exists (static by default; toggle shared with RECON_MIMALLOC_SHARED).
if (TARGET mimalloc::mimalloc)
    return()
endif()

if (NOT DEFINED RECON_USE_SYSTEM_MIMALLOC)
    set(RECON_USE_SYSTEM_MIMALLOC $<NOT:$<BOOL:WIN32>> CACHE BOOL "Prefer system mimalloc if available")
endif()

# Pin a release tag or commit (set in your presets/CI).
set(RECON_MIMALLOC_GIT_TAG "" CACHE STRING "mimalloc release tag or commit to fetch")

if (RECON_USE_SYSTEM_MIMALLOC)
    find_package(mimalloc QUIET CONFIG)
    if (TARGET mimalloc::mimalloc)
        message(STATUS "[mimalloc] Using system package (CONFIG).")
        return()
    endif()
endif()

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
if (RECON_MIMALLOC_GIT_TAG STREQUAL "")
    message(FATAL_ERROR "[mimalloc] Set RECON_MIMALLOC_GIT_TAG to a specific release (e.g. v2.1.x).")
endif()

FetchContent_Declare(mimalloc
        GIT_REPOSITORY https://github.com/microsoft/mimalloc.git
        GIT_TAG        ${RECON_MIMALLOC_GIT_TAG}
        GIT_SHALLOW    TRUE
)

# Build options
option(RECON_MIMALLOC_SHARED "Build mimalloc as a shared library" OFF)
set(MI_BUILD_SHARED     ${RECON_MIMALLOC_SHARED} CACHE BOOL "" FORCE)
set(MI_BUILD_STATIC     $<NOT:${RECON_MIMALLOC_SHARED}> CACHE BOOL "" FORCE)
set(MI_BUILD_TESTS      OFF CACHE BOOL "" FORCE)
set(MI_BUILD_OBJECT     OFF CACHE BOOL "" FORCE)
set(MI_INSTALL_TOPLEVEL OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(mimalloc)

# Unify target name
if (TARGET mimalloc)
    add_library(mimalloc::mimalloc ALIAS mimalloc)
elseif (TARGET mimalloc-static)
    add_library(mimalloc::mimalloc ALIAS mimalloc-static)
elseif (TARGET mimalloc-shared)
    add_library(mimalloc::mimalloc ALIAS mimalloc-shared)
else()
    message(FATAL_ERROR "[mimalloc] Expected mimalloc target not found.")
endif()

message(STATUS "[mimalloc] Using fetched ${RECON_MIMALLOC_GIT_TAG} (${RECON_MIMALLOC_SHARED}?shared:static)")
