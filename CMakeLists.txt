cmake_minimum_required(VERSION 4.1)
project(open-wbc VERSION 0.1.0 LANGUAGES C)

# 64-bit only
if(CMAKE_SIZEOF_VOID_P AND NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(FATAL_ERROR "64-bit builds required.")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "" FORCE)
endif()

list(PREPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

include(Options)

# Vulkan SDK (needed by CompileShaders.cmake for shader compilation)
find_package(Vulkan QUIET)

# Dependencies
set(RECON_VOLK_GIT_TAG "vulkan-sdk-1.4.304.0" CACHE STRING "Volk version tag")
set(RECON_SDL3_GIT_TAG "release-3.2.20" CACHE STRING "SDL3 version tag")

# Fetch Vulkan headers if SDK not installed (volk needs vulkan/vulkan.h)
if(NOT Vulkan_FOUND)
    include(FetchContent)
    FetchContent_Declare(vulkan-headers
        GIT_REPOSITORY https://github.com/KhronosGroup/Vulkan-Headers.git
        GIT_TAG        vulkan-sdk-1.4.304.0
        GIT_SHALLOW    TRUE
    )
    FetchContent_MakeAvailable(vulkan-headers)
    # Point volk to the fetched headers
    set(VULKAN_HEADERS_INSTALL_DIR "${vulkan-headers_SOURCE_DIR}" CACHE PATH "" FORCE)
endif()

include(FetchVolk)
include(FetchSDL3)

add_subdirectory(src)
