# Core
include(FetchMimalloc)

add_library(recon_core STATIC)
add_library(recon::core ALIAS recon_core)

target_sources(recon_core
        PRIVATE
        core/arena.c
        core/log.c
        core/memory.c
        PUBLIC
        FILE_SET HEADERS BASE_DIRS core/include FILES
        core/include/core/arena.h
        core/include/core/bits.h
        core/include/core/log.h
        core/include/core/memory.h
        core/include/core/types.h
)

target_link_libraries(recon_core PUBLIC mimalloc-static)
target_include_directories(recon_core PUBLIC include PRIVATE src)
recon_target_defaults(recon_core)

# Render API
add_library(recon_render_api INTERFACE
        render_api/include/render_api/render_api.h)
add_library(recon::render_api ALIAS recon_render_api)
target_include_directories(recon_render_api INTERFACE render_api/include)

# Engine
add_library(recon_engine STATIC
        engine/job_deque.h
        engine/job_internal.h
        engine/job_counter.h
        engine/job_deque.c
        engine/job_counter.c)
add_library(recon::engine ALIAS recon_engine)

target_sources(recon_engine
        PRIVATE
        engine/app.c
        engine/config.c
        engine/job.c
        PUBLIC
        FILE_SET HEADERS BASE_DIRS engine/include FILES
        engine/include/engine/app.h
        engine/include/engine/config.h
        engine/include/engine/input.h
        engine/include/engine/job.h
)

target_link_libraries(recon_engine
        PRIVATE SDL3::SDL3
        PUBLIC recon::core recon::render_api)
if (MSVC)
    target_link_libraries(recon_engine PRIVATE synchronization)
endif ()
target_include_directories(recon_engine PUBLIC include PRIVATE src)
recon_target_defaults(recon_engine)

# Renderer (Vulkan)
find_package(Vulkan)

include(FetchSDL3)
include(FetchVMA)
include(FetchVolk)

add_library(recon_renderer_vk STATIC)
add_library(recon::renderer_vk ALIAS recon_renderer_vk)

target_sources(recon_renderer_vk
        PRIVATE
        renderer_vk/renderer.c
        renderer_vk/resources.c
        renderer_vk/volk_impl.c
        PUBLIC
        FILE_SET HEADERS BASE_DIRS renderer_vk/include FILES
        renderer_vk/include/renderer/renderer.h
        renderer_vk/include/renderer/resources.h
        renderer_vk/include/renderer/shader.h
)

target_link_libraries(recon_renderer_vk
        PUBLIC  recon::core recon::render_api
        PRIVATE Vulkan::Vulkan SDL3::SDL3 recon_vma_headers
)
target_include_directories(recon_renderer_vk PUBLIC include ${volk_SOURCE_DIR} PRIVATE src)
recon_target_defaults(recon_renderer_vk)

# Shader compiling
include(CompileShaders)

add_shaders_directory(recon_renderer_vk src/shaders)

create_shader_target()

add_dependencies(recon_renderer_vk compile_shaders)

if(SPIRV_BINARY_FILES)
    message(STATUS "Shaders to be compiled:")
    foreach(shader ${SPIRV_BINARY_FILES})
        message(STATUS "  ${shader}")
    endforeach()
else()
    message(STATUS "No shaders found to compile")
endif()

# Game
add_executable(recon_game game/main.c)
add_executable(recon::game ALIAS recon_game)
target_link_libraries(recon_game PRIVATE recon::engine recon::renderer_vk SDL3::SDL3)
recon_target_defaults(recon_game)