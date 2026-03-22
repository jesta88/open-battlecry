# FetchVMA.cmake
# Ensures VulkanMemoryAllocator::VulkanMemoryAllocator exists (header-only).

include_guard(GLOBAL)

if (TARGET VulkanMemoryAllocator::VulkanMemoryAllocator)
    return()
endif()

set(RECON_VMA_GIT_TAG "" CACHE STRING "VMA tag/commit (e.g. v3.x.x)")
if (RECON_VMA_GIT_TAG STREQUAL "")
    message(FATAL_ERROR "[VMA] Set RECON_VMA_GIT_TAG to a specific tag/commit.")
endif()

include(FetchContent)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG        ${RECON_VMA_GIT_TAG}
        GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(vma)

# Header location varies across releases; expose a single interface target.
add_library(recon_vma_headers INTERFACE)
target_include_directories(recon_vma_headers INTERFACE
        ${vma_SOURCE_DIR}
        ${vma_SOURCE_DIR}/include
        ${vma_SOURCE_DIR}/src
)
# Consumers: #include <vk_mem_alloc.h>

# Now create the namespaced alias (this is the correct way)
add_library(VulkanMemoryAllocator::VulkanMemoryAllocator ALIAS recon_vma_headers)

message(STATUS "[VMA] Using ${RECON_VMA_GIT_TAG} (header-only)")
