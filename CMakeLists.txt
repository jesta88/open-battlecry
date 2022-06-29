cmake_minimum_required(VERSION 3.22)
project(battlecry VERSION 0.1.0)

set(CMAKE_C_STANDARD 23)
set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Vulkan
find_package(Vulkan REQUIRED)

# VMA
add_library(vma STATIC 
    src/third_party/volk.h
    src/third_party/vma/vk_mem_alloc.h
    src/third_party/vma/vk_mem_alloc.cpp)

target_include_directories(vma PUBLIC src/third_party/vma)

target_link_libraries(vma 
    PRIVATE
        Vulkan::Headers)

target_compile_definitions(vma
    PRIVATE
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN VK_USE_PLATFORM_WIN32_KHR>
        $<$<PLATFORM_ID:Linux>:VK_USE_PLATFORM_XCB_KHR VK_USE_PLATFORM_XCB_XRANDR_EXT>)

# freetype
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/third_party/freetype/lib/freetype.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/third_party/freetype/include)

# Tools
add_executable(bintoc src/tools/bintoc.c)
target_link_libraries(bintoc PRIVATE Vulkan::Headers)

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
            COMMAND ${CMAKE_BINARY_DIR}/bintoc.exe ${SPIRV} ${FILE_NAME}_spv ${CMAKE_SOURCE_DIR}/src/shaders/${FILE_NAME}.c
            DEPENDS ${GLSL})
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach(GLSL)

add_custom_target(shaders
        DEPENDS ${SPIRV_BINARY_FILES})
add_dependencies(shaders bintoc)

# Client
add_executable(client
        WIN32
        src/client/camera.c
        src/client/client.h
        src/client/client.c
        src/client/resource.c
        src/client/graphics.c
        src/common/log.c
        src/common/log.h
        src/third_party/volk.c
        src/shaders/shaders.h
        src/shaders/sprite.vert.c
        src/shaders/sprite.frag.c)

add_dependencies(client shaders)

target_compile_definitions(client
    PRIVATE
        $<$<PLATFORM_ID:Windows>:WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS VK_USE_PLATFORM_WIN32_KHR>
        $<$<PLATFORM_ID:Linux>:VK_USE_PLATFORM_XCB_KHR VK_USE_PLATFORM_XCB_XRANDR_EXT>
)
target_link_libraries(client PRIVATE Vulkan::Headers vma)
set_target_properties(client PROPERTIES LINKER_LANGUAGE C)

if (MSVC)
target_compile_options(client PRIVATE /nologo /GS- /std:c17 /Zc:preprocessor /fp:fast /arch:AVX)
endif()

# Asset converter
add_executable(asset_converter
        src/asset_converter/main.c
        src/asset_converter/xcr.c
        src/asset_converter/xcr.h
        src/asset_converter/animation.h
        src/asset_converter/rle.h)