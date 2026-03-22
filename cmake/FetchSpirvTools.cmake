include(FetchContent)

set(SPIRV_SKIP_TESTS ON CACHE BOOL "SPIRV_SKIP_TESTS")
set(SPIRV_SKIP_EXECUTABLES ON CACHE BOOL "SPIRV_SKIP_EXECUTABLES")

FetchContent_Declare(
        SPIRV-Tools
        GIT_REPOSITORY https://github.com/KhronosGroup/SPIRV-Tools.git
        GIT_TAG main
)
FetchContent_MakeAvailable(SPIRV-Tools)