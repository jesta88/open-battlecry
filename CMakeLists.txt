cmake_minimum_required(VERSION 3.21)
project(battlecry LANGUAGES C RC VERSION 0.1.0)

set(CMAKE_C_STANDARD 23)
set(ignore ${CMAKE_CXX_COMPILER})

option(SERVER_BUILD "Strip non-server code and dependencies." OFF)

if (MSVC)
    add_compile_options(/W4 /WX /arch:AVX)
    set(SDL2_DLL SDL2.dll)
    set(SDL2_LIB SDL2.lib)
    set(SDL2main_LIB SDL2main.lib)
    set(FREETYPE_DLL freetype.dll)
    set(FREETYPE_LIB freetype.lib)
else()
    add_compile_options(-Wall -Werror -mavx)
    set(SDL2_DLL libSDL2.dll.a)
    set(SDL2_LIB libSDL2.a)
    set(SDL2main_LIB libSDL2main.a)
    set(FREETYPE_DLL libfreetype.dll.a)
    set(FREETYPE_LIB libfreetype.a)
endif()

add_library(SDL2 SHARED IMPORTED)
set_target_properties(SDL2 PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/deps/SDL2/lib/${SDL2_DLL}
        IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/deps/SDL2/lib/${SDL2_LIB}
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/SDL2/include)

add_library(SDL2main STATIC IMPORTED)
set_target_properties(SDL2main PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/deps/SDL2/lib/${SDL2main_LIB})

add_library(freetype SHARED IMPORTED)
set_target_properties(freetype PROPERTIES
        IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/deps/freetype/lib/${FREETYPE_DLL}
        IMPORTED_IMPLIB ${CMAKE_SOURCE_DIR}/deps/freetype/lib/${FREETYPE_LIB}
        INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_SOURCE_DIR}/deps/freetype/include)

add_library(engine STATIC
        src/engine/log.c
        include/engine/log.h
        src/engine/core/bits.inl
        include/engine/math.h
        src/engine/config.c
        include/engine/config.h
        src/engine/core/hash.c
        src/engine/core/hash.h
        src/engine/core/hash_map.h
        src/engine/core/hash_map.c
        src/engine/core/string.inl
        src/engine/core/random.c
        src/engine/core/random.h
        src/engine/core/allocator.h
        src/engine/core/allocator.c
        deps/stb_sprintf.h
        src/engine/core/stb.c
        src/engine/file.h
        src/engine/file.c
        src/engine/core/path.h
        src/engine/core/path.c
        src/engine/core/math.inl
        include/engine/types.h
        src/engine/platform.h
        src/engine/input.c
        include/engine/input.h
        src/engine/graphics/renderer_sdl.c
        include/engine/renderer.h
        src/engine/asset/font.h
        src/engine/asset/font.c
        src/engine/asset/image.c
        src/engine/asset/image.h
        src/engine/asset/asset.h
        src/engine/asset/asset.c
        src/game/strings.inl
        src/engine/graphics/text.h
        include/engine/graphics.h
        src/engine/graphics/graphics_sdl.c
        include/engine/application.h
        src/engine/platform/windows.h
        src/engine/application.c
        include/engine/memory.h
        include/engine/io.h src/engine/private.h include/engine/collections.h src/engine/pool.c include/engine/state.h)

if (SERVER_BUILD)
    message("Server build")
    target_compile_definitions(engine PUBLIC -DSERVER)
else()
    target_link_libraries(engine PRIVATE SDL2)
endif()
unset(SERVER_BUILD CACHE)

target_include_directories(engine
        PRIVATE deps/include
        PUBLIC include)

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

# Baker
add_executable(bake src/bake/font_baker.c src/bake/bake.c)
target_link_libraries(bake PRIVATE freetype)

# Output 
set_target_properties(engine game
        PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/lib"
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}/lib"
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/${CMAKE_BUILD_TYPE}")