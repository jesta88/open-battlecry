add_executable(game
    main.c
    gfx.c
    file_io.c
    xcr.c
    image.c
    stb_image.c
    stb_truetype.c
    ani.c
    entity.c
    map.c
    pathfind.c
    font.c
    audio.c
    hud.c
    menu.c
    resource.c
    mine.c
    building.c
    baked_loader.c
    console.c
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

# Copy assets to build output directory
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:game>/assets"
    COMMENT "Copying assets to build output"
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
