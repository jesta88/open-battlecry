cmake_minimum_required(VERSION 3.21)
project(battlecry LANGUAGES C RC VERSION 0.1.0)

set(CMAKE_C_STANDARD 23)
set(ignore ${CMAKE_CXX_COMPILER})
set(CMAKE_VERBOSE_MAKEFILE ON)

# Vulkan
find_package(Vulkan REQUIRED)

# freetype
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/third_party/freetype/lib/freetype.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/third_party/freetype/include)

# Tools
add_executable(bintoc
        src/tools/bintoc.c)

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
        src/client/camera.c
        src/client/client.h
        src/client/main.c
        src/client/resource.c
        src/client/graphics.c
        src/common/log.c
        src/common/log.h
        src/third_party/volk.c)

add_dependencies(client shaders)
target_link_libraries(client PRIVATE Vulkan::Vulkan)
if (WIN32)
target_compile_definitions(client PRIVATE -DVK_USE_PLATFORM_WIN32_KHR)
endif()

# Game
add_executable(game
        src/game/entity.c
        src/game/game.c)

target_link_libraries(game PRIVATE engine)

# Asset converter
#add_executable(asset_converter
#        src/asset_converter/main.c
#        src/asset_converter/xcr.c
#        src/asset_converter/xcr.h
#        src/asset_converter/animation.h
#        src/asset_converter/rle.h)
#target_include_directories(asset_converter PRIVATE src)
#target_link_libraries(asset_converter PRIVATE core)