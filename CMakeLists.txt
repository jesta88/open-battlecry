cmake_minimum_required(VERSION 3.20)
project(battlecry C)

set(CMAKE_C_STANDARD 11)
set(ignore ${CMAKE_CXX_COMPILER})

add_library(SDL2 SHARED IMPORTED)
set_target_properties(SDL2 PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/deps/lib/SDL2.dll
        IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/deps/lib/SDL2.lib
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/include)

add_library(engine STATIC
        src/engine/log.c
        src/engine/log.h
        src/engine/bits.inl
        src/engine/math.h
        src/engine/config.c
        src/engine/config.h
        src/engine/hash.c
        src/engine/hash.h
        src/engine/hash_map.h
        src/engine/hash_map.c
        src/engine/string.inl
        src/engine/random.c
        src/engine/random.h
        src/engine/allocator.h
        src/engine/allocator.c
        src/engine/stb_sprintf.h
        src/engine/stb.c
        src/engine/os.h
        src/engine/os.c
        src/engine/file.h
        src/engine/file.c
        src/engine/path.h
        src/engine/path.c
        src/engine/math.inl
        src/engine/types.h src/engine/engine.h)
target_include_directories(engine PUBLIC deps/include)

# Game
add_library(game SHARED
        src/game/game.c
        src/game/game.h
        src/game/entity.c
        src/game/entity.h
        src/game/scene.c
        src/game/scene.h
        src/game/unit.c
        src/game/unit.h
        src/game/configs.h
        src/game/game_state.h
        src/game/command.c
        src/game/command.h
        src/game/types.h)
target_link_libraries(game PUBLIC engine)

# Client
add_executable(client
        src/client/client.c
        src/client/input_sdl.c
        src/client/input.h
        src/client/renderer_sdl.c
        src/client/renderer.h
        src/client/window.h
        src/client/window_sdl.c
        src/client/audio_sdl.c
        src/client/audio.h
        src/client/camera.c
        src/client/camera.h
        src/client/client.h
        src/client/font.h
        src/client/font.c
        src/client/image.c
        src/client/image.h
        src/client/asset.h
        src/client/asset.c
        src/client/console.c
        src/client/console.h
        src/client/strings.inl
        src/client/text.h
        src/client/graphics.h
        src/client/graphics_sdl.c)
target_link_libraries(client PRIVATE SDL2)
target_compile_options(client PRIVATE $<$<BOOL:${MSVC}>:/arch:AVX>)

# Server
add_executable(server
        src/server/main.c)

if (DEBUG)
    target_compile_definitions(engine PRIVATE DEBUG)
    target_compile_definitions(game PRIVATE DEBUG)
    target_compile_definitions(client PRIVATE DEBUG)
    target_compile_definitions(server PRIVATE DEBUG)
endif ()

if (PROFILE)
    target_compile_definitions(engine PRIVATE PROFILE)
    target_compile_definitions(game PRIVATE PROFILE)
    target_compile_definitions(client PRIVATE PROFILE)
    target_compile_definitions(server PRIVATE PROFILE)
endif ()

# Baker
#add_executable(bake src/bake/main.c src/bake/net.h src/bake/net.c src/bake/font_baker.c)
#target_link_libraries(bake PRIVATE freetype)

# Output 
set_target_properties(game client server
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${CMAKE_BUILD_TYPE}")