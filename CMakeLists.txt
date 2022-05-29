cmake_minimum_required(VERSION 3.21)
project(battlecry LANGUAGES C RC VERSION 0.1.0)

set(CMAKE_C_STANDARD 23)
set(ignore ${CMAKE_CXX_COMPILER})

# Freetype
find_package(freetype CONFIG REQUIRED)

# SDL2
find_package(SDL2 CONFIG REQUIRED)

# Renderer
add_library(renderer_vk SHARED
        src/renderer_vk/renderer.c)

add_library(renderer_sdl SHARED
        src/renderer_sdl/renderer.c "src/renderer_sdl/renderer.h")

# Engine
add_library(engine STATIC
        src/engine/application.h
        src/engine/application.c
        src/engine/bits.inl
        src/engine/config.c
        src/engine/config.h
        src/engine/filesystem.c
        src/engine/filesystem.h
        src/engine/hashmap.h
        src/engine/hashmap.c
        src/engine/input.c
        src/engine/input.h
        src/engine/job.h
        src/engine/job.c
        src/engine/log.c
        src/engine/log.h
        src/engine/memory.c
        src/engine/random.c
        src/engine/random.h
        src/engine/string.inl
        src/engine/types.h
        src/engine/windows.h
        src/engine/xxhash.h
        src/engine/xxhash.c
        src/engine/asset/font.h
        src/engine/asset/font.c
        src/engine/asset/image.c
        src/engine/asset/image.h
        src/engine/asset/asset.h
        src/engine/asset/asset.c
        src/engine/graphics/renderer_sdl.c
        src/engine/graphics/graphics_sdl.c
        src/engine/asset/wuffs.h)

target_compile_options(engine PUBLIC /arch:AVX)
target_link_libraries(engine PRIVATE SDL2::SDL2 SDL2::SDL2main freetype)
if(RENDERER_VK)
    target_link_libraries(engine PRIVATE renderer_vk)
else()
    target_link_libraries(engine PRIVATE renderer_sdl)
endif()

# Game
add_executable(game
        src/game/game.c
        src/game/entity.c
        src/game/entity.h
        src/game/state.c
        src/game/scene.h
        src/game/unit.c
        src/game/unit.h
        src/game/configs.h
        src/game/game_state.h
        src/game/command.c
        src/game/command.h
        src/game/types.h)

target_link_libraries(game PRIVATE engine)

# Asset converter
add_executable(asset_converter
        src/asset_converter/main.c
        src/asset_converter/xcr.c
        src/asset_converter/xcr.h
        src/asset_converter/animation.h
        src/asset_converter/rle.h)
target_include_directories(asset_converter PRIVATE src)
target_link_libraries(asset_converter PRIVATE core)