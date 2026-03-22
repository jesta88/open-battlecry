# FetchVolk.cmake
# Ensures volk::volk exists. Prefer header-only usage.

include_guard(GLOBAL)

if (TARGET volk::volk)
    return()
endif()

set(RECON_VOLK_GIT_TAG "" CACHE STRING "Volk tag/commit (e.g. sdk-1.4.xxx)")
if (RECON_VOLK_GIT_TAG STREQUAL "")
    message(FATAL_ERROR "[volk] Set RECON_VOLK_GIT_TAG to a specific tag/commit.")
endif()

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(volk
        GIT_REPOSITORY https://github.com/zeux/volk.git
        GIT_TAG        ${RECON_VOLK_GIT_TAG}
        GIT_SHALLOW    TRUE
)

# Modern best practice: configure + add upstream targets
FetchContent_MakeAvailable(volk)

# Provide a stable namespaced target:
if (TARGET volk::volk)
    # great, upstream already exported it
elseif (TARGET volk)
    add_library(volk::volk ALIAS volk)
else()
    # Fall back to our own header-only interface target
    add_library(recon_volk_headers INTERFACE)
    target_include_directories(recon_volk_headers INTERFACE ${volk_SOURCE_DIR})
    target_compile_definitions(recon_volk_headers INTERFACE VOLK_HEADER_ONLY=1)
    add_library(volk::volk ALIAS recon_volk_headers)
endif()

message(STATUS "[volk] Using ${RECON_VOLK_GIT_TAG}")
