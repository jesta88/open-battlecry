add_executable(game
    main.c
    gfx.c
    file_io.c
    xcr.c
    image.c
    stb_image.c
    ani.c
    entity.c
)

recon_target_defaults(game)

target_link_libraries(game PRIVATE
    volk::volk
    SDL3::SDL3
)

target_compile_definitions(game PRIVATE
    VK_NO_PROTOTYPES
    $<$<CONFIG:Debug>:WBC_DEBUG>
)

# Shader compilation (GLSL -> SPIR-V) - requires Vulkan SDK
if(Vulkan_FOUND)
    include(CompileShaders)
    add_shaders_directory(game "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    create_shader_target()
    add_dependencies(game compile_shaders)
else()
    message(STATUS "Vulkan SDK not found - shader compilation disabled. Install Vulkan SDK for shader support.")
endif()
