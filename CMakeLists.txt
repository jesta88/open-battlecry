cmake_minimum_required(VERSION 3.22)
project(battlecry VERSION 0.1.0)

option(FORCE_USE_SDL "Force using SDL as the system backend" OFF)

# Vulkan
find_package(Vulkan REQUIRED)

# SDL
if (NOT WIN32 OR FORCE_USE_SDL)
    add_library(SDL SHARED IMPORTED)
    set_target_properties(SDL PROPERTIES
            IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/third_party/SDL/bin/SDL2.dll
            IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/src/third_party/SDL/lib/libSDL2.dll.a
            INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/third_party/SDL/include)
endif()

# Vulkan Memory Allocator
add_library(vma STATIC
        src/third_party/volk/volk.h
    src/third_party/vma/vk_mem_alloc.h
    src/third_party/vma/vk_mem_alloc.cpp)
target_link_libraries(vma PRIVATE Vulkan::Headers)
target_include_directories(vma PUBLIC src/third_party/vma)
target_compile_options(vma PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/nologo /GS- /std:c17 /Zc:preprocessor /fp:fast /arch:AVX>
        $<$<C_COMPILER_ID:GNU>:-mavx -std=c2x>)
target_compile_definitions(vma PRIVATE
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN VK_USE_PLATFORM_WIN32_KHR>
        $<$<PLATFORM_ID:Linux>:VK_USE_PLATFORM_XCB_KHR VK_USE_PLATFORM_XCB_XRANDR_EXT>)

# freetype
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/third_party/freetype/lib/freetype.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/third_party/freetype/include)

# libktx
add_library(ktx STATIC IMPORTED)
set_target_properties(ktx PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/third_party/ktx/lib/ktx.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/third_party/ktx/include)

# Shader Compiler
add_executable(shader_compiler src/tools/shader_compiler/main.c)
target_link_libraries(shader_compiler PRIVATE Vulkan::Headers)
target_compile_options(shader_compiler PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/nologo /GS- /std:c17 /Zc:preprocessor /fp:fast /arch:AVX>
        $<$<C_COMPILER_ID:GNU>:-mavx -std=c2x>)
target_compile_definitions(shader_compiler PRIVATE $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN>)

# Shaders
file(GLOB_RECURSE GLSL_SOURCE_FILES
        "src/shaders/*.frag"
        "src/shaders/*.vert")

foreach(GLSL ${GLSL_SOURCE_FILES})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(OUT_DIR "${CMAKE_SOURCE_DIR}/src/shaders/")
    set(SPIRV "${OUT_DIR}${FILE_NAME}.spv")
    add_custom_command(
            OUTPUT ${SPIRV}
            COMMENT "Compiling ${GLSL}"
            COMMAND $ENV{VULKAN_SDK}/Bin/glslangValidator.exe -V ${GLSL} -o ${SPIRV}
            COMMAND ${CMAKE_BINARY_DIR}/shader_compiler.exe ${SPIRV} ${FILE_NAME}_spv ${CMAKE_SOURCE_DIR}/src/shaders/${FILE_NAME}.c
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(shaders
        DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(shaders shader_compiler)

# Renderer
add_library(renderer SHARED 
    "src/_temp/graphics/graphics.h"
    "src/_temp/graphics/graphics.c")

target_link_libraries(renderer PRIVATE Vulkan::Headers vma ktx)
set_target_properties(renderer PROPERTIES LINKER_LANGUAGE C)
target_compile_options(renderer PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/nologo /GS- /std:c17 /Zc:preprocessor /fp:fast /arch:AVX>
        $<$<C_COMPILER_ID:GNU>:-mavx -std=c2x>)

# Engine
add_library(engine SHARED
        src/_temp/log.c
        src/_temp/engine_windows.c
        src/_temp/application.c
        src/_temp/system/application_private.h
        src/_temp/engine.h
        src/_temp/engine.c
        src/_temp/engine_private.h
        src/_temp/timer.c src/_temp/events.c src/engine/system.h src/engine/system.c src/engine/strings.h src/engine/strings.c src/engine/debug.h src/engine/log.c src/engine/log.h src/engine/keys.c src/engine/keys.h src/carbon/win32_window.c src/carbon/include/carbon_window.h src/carbon/types.h src/carbon/carbon_animation.c src/carbon/carbon_animation.h src/carbon/win32_common.h)
target_link_libraries(engine PRIVATE
        $<$<BOOL:${FORCE_USE_SDL}>:SDL>
        $<$<PLATFORM_ID:Windows, Linux>:Vulkan::Headers vma>)
target_include_directories(engine PUBLIC src/engine)
target_compile_options(engine PUBLIC
        $<$<C_COMPILER_ID:MSVC>:/nologo /GS- /std:c17 /Zc:preprocessor /fp:fast /arch:AVX>
        $<$<C_COMPILER_ID:GNU>:-mavx -std=c2x>)
target_link_options(engine PRIVATE)
target_compile_definitions(engine PRIVATE
        $<$<BOOL:${FORCE_USE_SDL}>:FORCE_USE_SDL>
        $<$<CONFIG:Debug>:DEBUG>
        $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS VC_EXTRALEAN WIN32_LEAN_AND_MEAN VK_USE_PLATFORM_WIN32_KHR>
        $<$<PLATFORM_ID:Linux>:VK_USE_PLATFORM_XCB_KHR VK_USE_PLATFORM_XCB_XRANDR_EXT>)
set_target_properties(engine PROPERTIES LINKER_LANGUAGE C)

# Game
add_executable(game
        src/game/game.c)
target_link_libraries(game PRIVATE engine)
target_compile_options(game PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/nologo /GS- /std:c17 /Zc:preprocessor /fp:fast /arch:AVX>
        $<$<C_COMPILER_ID:GNU>:-mavx -std=c2x>)

# Asset converter
add_executable(asset_converter
        src/tools/asset_converter/main.c
        src/tools/asset_converter/xcr.c
        src/tools/asset_converter/xcr.h
        src/tools/asset_converter/animation.h
        src/tools/asset_converter/ktx.h
        src/tools/asset_converter/ktx.c
        src/tools/asset_converter/image.c
        src/tools/asset_converter/image.h
        src/tools/asset_converter/encode.c
        src/tools/asset_converter/encode.h
        src/engine/std.h
        src/_temp/log.c
        src/_temp/io.c
        src/_temp/system/memory.h
        src/_temp/system/memory.c
        src/_temp/system/windows/windows_base.h
        src/_temp/system/windows/windows_file.h
        src/_temp/system/windows/windows_io.h
        src/_temp/system/windows/windows_misc.h
        src/third_party/ktx/include/ktx.h
        src/third_party/ktx/include/ktxvulkan.h
        src/third_party/stb/stb_image.h
        src/third_party/stb/stb_image.c
        src/third_party/bc7e/bc7e_ispc.h
        src/third_party/bc7e/bc7e.obj
        src/third_party/bc7e/bc7e_avx.obj
        src/third_party/bc7e/bc7e_avx2.obj
        src/third_party/bc7e/bc7e_sse2.obj
        src/third_party/bc7e/bc7e_sse4.obj)
target_link_libraries(asset_converter PRIVATE ktx shlwapi)
target_compile_options(asset_converter PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/nologo /GS- /std:c17 /Zc:preprocessor /fp:fast /arch:AVX /openmp>
        $<$<C_COMPILER_ID:GNU>:-mavx -std=c2x -fopenmp>)
target_compile_definitions(asset_converter PRIVATE SUPPORT_BC7E=1 $<$<PLATFORM_ID:Windows>:_CRT_SECURE_NO_WARNINGS WIN32_LEAN_AND_MEAN>)

# Texture viewer
add_executable(texture_viewer WIN32
        src/tools/texture_viewer/main.c
    src/third_party/ktx/include/ktx.h
    src/third_party/ktx/include/ktxvulkan.h)
target_link_libraries(texture_viewer PRIVATE ktx)
target_compile_options(texture_viewer PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/nologo /GS- /std:c17 /Zc:preprocessor /fp:fast /arch:AVX>
        $<$<C_COMPILER_ID:GNU>:-mavx -std=c2x>)