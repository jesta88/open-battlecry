cmake_minimum_required(VERSION 3.20)
project(battlecry C)

set(CMAKE_C_STANDARD 11)
set(ignore ${CMAKE_CXX_COMPILER})

set(BASE_SRC
    src/base/log.c
    src/base/log.h
    src/base/bits.inl
    src/base/math.h
    src/base/config.c
    src/base/config.h
    src/base/hash.c
    src/base/hash.h
    src/base/hash_map.h
    src/base/hash_map.c
    src/base/string.inl
    src/base/random.c
    src/base/random.h
    src/base/allocator.h
    src/base/allocator.c
    src/base/stb_sprintf.h
    src/base/stb.c
    src/base/time.inl
    src/base/sokol_time.h
    src/base/sokol.c
    src/base/base.h
    src/base/base.c
    src/base/os.h
    src/base/os.c "src/base/file.h" "src/base/file.c" "src/base/path.h" "src/base/path.c" src/base/math.inl)

# Game
add_library(game SHARED
    src/game/game.c
    src/game/game.h src/game/entity.c src/game/entity.h src/game/scene.c src/game/scene.h src/game/unit.c src/game/unit.h src/game/configs.h src/game/game_state.h src/game/command.c src/game/command.h src/game/types.h)

# Client
add_executable(client
    ${BASE_SRC}
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
    "src/client/image.c" "src/client/image.h" "src/client/asset.h" "src/client/asset.c" src/client/console.c src/client/console.h)

find_package(SDL2 CONFIG REQUIRED)

target_link_libraries(client PRIVATE SDL2::SDL2 SDL2::SDL2main)
target_compile_options(client PRIVATE $<$<BOOL:${MSVC}>:/arch:AVX>)

# Server
add_executable(server
    src/server/main.c)

if (DEBUG)
    target_compile_definitions(game PRIVATE DEBUG)
    target_compile_definitions(client PRIVATE DEBUG)
    target_compile_definitions(server PRIVATE DEBUG)
endif()

if (DEVELOPMENT)
    target_compile_definitions(game PRIVATE DEVELOPMENT)
    target_compile_definitions(client PRIVATE DEVELOPMENT)
    target_compile_definitions(server PRIVATE DEVELOPMENT)
endif()

# Editor
add_executable(editor editor/src/main.c "editor/src/gui.h" "editor/src/gui.c")
target_link_libraries(editor PRIVATE user32 gdi32 comctl32)
#target_link_directories(editor PRIVATE editor/lib)
target_link_options(editor PRIVATE /SUBSYSTEM:WINDOWS)
#target_link_options(editor PRIVATE /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup)
#if (MSVC)
#    target_link_libraries(editor PRIVATE iup.lib gdi32 comdlg32 comctl32 uuid oleaut32 ole32)
#elseif (MINGW)
#    target_link_libraries(editor PRIVATE libiup.a gdi32 comdlg32 comctl32 uuid oleaut32 ole32)
#endif()