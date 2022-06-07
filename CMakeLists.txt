cmake_minimum_required(VERSION 3.21)
project(battlecry LANGUAGES C RC VERSION 0.1.0)

set(CMAKE_C_STANDARD 23)
set(ignore ${CMAKE_CXX_COMPILER})

# Vulkan
find_package(Vulkan REQUIRED)

# SDL2
add_library(SDL2 SHARED IMPORTED)
set_target_properties(SDL2 PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/third_party/SDL2/bin/SDL2.dll
        IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/src/third_party/SDL2/lib/SDL2.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/third_party/SDL2/include)

add_library(SDL2main STATIC IMPORTED)
set_target_properties(SDL2main PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/third_party/SDL2/lib/SDL2main.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/third_party/SDL2/include)

# freetype
add_library(freetype STATIC IMPORTED)
set_target_properties(freetype PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/src/third_party/freetype/lib/freetype.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/src/third_party/freetype/include)

set (COMMON_SRC
        src/common/assert.c
        src/common/asset.c
        src/common/bits.inl
        src/common/config.c
        src/common/engine.c
        src/common/filesystem.c
        src/common/font.c
        src/common/hashmap.c
        src/common/image.c
        src/common/input.c
        src/common/job.c
        src/common/log.c
        src/common/memory.c
        src/common/random.c
        src/common/string.inl
        src/common/thread.c
        src/common/types.h
        src/common/windows.h)

# Renderer
add_library(renderer SHARED
        src/renderer/renderer.c)

target_compile_options(renderer PRIVATE /arch:AVX)
target_link_libraries(renderer PRIVATE SDL2 freetype Vulkan)

# Client
add_executable(client
        src/client/camera.c
        src/client/main.c
        src/client/resource.c)

target_compile_options(client PRIVATE /arch:AVX)
target_link_libraries(client PRIVATE SDL2 SDL2main)

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