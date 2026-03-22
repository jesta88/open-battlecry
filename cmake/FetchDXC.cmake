include(FetchContent)

set(SPIRV_CROSS_ENABLE_TESTS OFF CACHE BOOL "SPIRV_CROSS_ENABLE_TESTS")
set(SPIRV_CROSS_ENABLE_CPP OFF CACHE BOOL "SPIRV_CROSS_ENABLE_CPP")
set(SPIRV_CROSS_CLI OFF CACHE BOOL "SPIRV_CROSS_CLI")

FetchContent_Declare(
        SPIRV-Cross
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Cross.git
        GIT_TAG main
)
FetchContent_MakeAvailable(SPIRV-Cross)