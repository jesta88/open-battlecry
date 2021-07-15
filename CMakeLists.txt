cmake_minimum_required(VERSION 3.20)
project(battlecry C)

set(CMAKE_C_STANDARD 11)
set(ignore ${CMAKE_CXX_COMPILER})

set(SHARED_SRC
    src/shared/log.c
    src/shared/log.h
    src/shared/bits.inl
    src/shared/types.h
    src/shared/config.c
    src/shared/config.h
    src/shared/hash.c
    src/shared/hash.h
    src/shared/hash_map.h
    src/shared/hash_map.c
    src/shared/string.inl
    src/shared/random.c
    src/shared/random.h
    src/shared/allocator.h
    src/shared/allocator.c 
    src/shared/stb_sprintf.h
    src/shared/stb.c "src/shared/time.inl" "src/shared/sokol_time.h" "src/shared/sokol.c" "src/shared/init.h" "src/shared/init.c" )

# Game
add_library(game SHARED
    src/game/game.c
    src/game/game.h)

# Client
add_executable(client
    ${SHARED_SRC}
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
    "src/client/stb_rect_pack.h" 
    "src/client/stb_truetype.h" 
    "src/client/stb_image.h"
    "src/client/font.h" 
    "src/client/font.c" 
    "src/client/stb.c")

find_package(SDL2 CONFIG REQUIRED)
find_package(sdl2-image CONFIG REQUIRED)

target_link_libraries(client PRIVATE SDL2::SDL2 SDL2::SDL2main SDL2::SDL2_image)

# Server
add_executable(server
    src/server/main.c)
    

if (DEVELOPMENT)
    target_compile_definitions(game PRIVATE DEVELOPMENT)
    target_compile_definitions(client PRIVATE DEVELOPMENT)
    target_compile_definitions(server PRIVATE DEVELOPMENT)
endif()

# Copy DLLs to output directory
#file(GLOB DLL_FILES external/bin/*.dll)
#foreach(file ${DLL_FILES})
#    get_filename_component(FILE_NAME ${file} NAME)

#    add_custom_command(
#            TARGET client
#            POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different
#            ${file}
#            "$<TARGET_FILE_DIR:client>/${SDL2_LIBRARY_NAME}"
#            COMMENT "Copying ${FILE_NAME} to output directory"
#    )
#endforeach()
