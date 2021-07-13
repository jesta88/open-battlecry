cmake_minimum_required(VERSION 3.20)
project(battlelords C)

set(CMAKE_C_STANDARD 17)

add_executable(battlelords
    src/client/main.c
    src/client/client.c
    src/client/input.c
    src/client/input.h
    src/log.c
    src/log.h
    src/bits.h
    src/client/renderer_sdl.c
    src/client/renderer.h
    src/types.h
    src/client/window.h
    src/client/window_sdl.c
    src/assert.h
    src/config.c
    src/config.h
    src/hash.c
    src/hash.h
    src/hash_map.h
    src/hash_map.c src/random.c src/random.h src/allocator.h src/allocator.c src/client/audio.c src/client/audio.h)

target_include_directories(battlelords PRIVATE external/include)
target_link_directories(battlelords PRIVATE external/lib)
target_link_libraries(battlelords PRIVATE SDL2 SDL2main SDL2_image)

# Copy DLLs to output directory
file(GLOB DLL_FILES external/bin/*.dll)
foreach(file ${DLL_FILES})
    get_filename_component(FILE_NAME ${file} NAME)

    add_custom_command(
            TARGET battlelords
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${file}
            ${CMAKE_CURRENT_BINARY_DIR}/${FILE_NAME}
            COMMENT "Copying ${FILE_NAME} to output directory"
    )
endforeach()
