# Find Vulkan SDK
find_package(Vulkan REQUIRED)

# Initialize list to track all SPIR-V binaries
set(SPIRV_BINARY_FILES "" CACHE INTERNAL "List of SPIR-V binary files")

# Determine Vulkan version from SDK
function(get_vulkan_target_env)
    # Default to vulkan1.3 if we can't determine
    set(target_env "vulkan1.3")

    # Try to get version from Vulkan headers
    if(Vulkan_INCLUDE_DIRS)
        file(READ "${Vulkan_INCLUDE_DIRS}/vulkan/vulkan_core.h" vulkan_core_content)

        # Extract VK_HEADER_VERSION_COMPLETE
        if(vulkan_core_content MATCHES "#define VK_HEADER_VERSION_COMPLETE VK_MAKE_API_VERSION\\(([0-9]+), ([0-9]+), ([0-9]+), ([0-9]+)\\)")
            set(variant ${CMAKE_MATCH_1})
            set(major ${CMAKE_MATCH_2})
            set(minor ${CMAKE_MATCH_3})
            set(patch ${CMAKE_MATCH_4})

            # Simply concatenate major.minor for target environment
            set(target_env "vulkan${major}.${minor}")

            message(STATUS "Detected Vulkan SDK version: ${major}.${minor}.${patch}, using target-env: ${target_env}")
        endif()
    endif()

    set(VULKAN_TARGET_ENV ${target_env} PARENT_SCOPE)
endfunction()

# Get the target environment (unless already set)
if(NOT DEFINED VULKAN_TARGET_ENV)
    get_vulkan_target_env()
else()
    message(STATUS "Using predefined Vulkan target-env: ${VULKAN_TARGET_ENV}")
endif()

# Function to compile shaders
function(add_shader TARGET SHADER)
    find_program(GLSLANG_VALIDATOR
            NAMES glslangValidator
            HINTS
            $ENV{VULKAN_SDK}/Bin
            $ENV{VULKAN_SDK}/Bin32
    )

    if(NOT GLSLANG_VALIDATOR)
        message(FATAL_ERROR "glslangValidator not found! Make sure Vulkan SDK is installed and VULKAN_SDK environment variable is set.")
    endif()

    set(current_shader_path ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
    get_filename_component(current_shader_name ${SHADER} NAME)
    set(current_output_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${current_shader_name}.spv)

    # Create output directory
    get_filename_component(current_output_dir ${current_output_path} DIRECTORY)
    file(MAKE_DIRECTORY ${current_output_dir})

    # Add custom command for shader compilation
    add_custom_command(
            OUTPUT ${current_output_path}
            COMMAND ${GLSLANG_VALIDATOR}
            -V                          # Generate SPIR-V
            --target-env ${VULKAN_TARGET_ENV}  # Target Vulkan version from SDK
            -o ${current_output_path}   # Output file
            ${current_shader_path}      # Input file
            DEPENDS ${current_shader_path}
            COMMENT "Compiling shader ${SHADER}"
            VERBATIM
    )

    # Add shader to target
    set_source_files_properties(${current_output_path} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${current_output_path})

    # Add to global list of SPIR-V binaries
    set(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES} ${current_output_path} CACHE INTERNAL "List of SPIR-V binary files")
endfunction()

# Function to compile all shaders in a directory
function(add_shaders_directory TARGET SHADER_DIR)
    file(GLOB_RECURSE SHADERS
            ${SHADER_DIR}/*.vert
            ${SHADER_DIR}/*.frag
            ${SHADER_DIR}/*.comp
            ${SHADER_DIR}/*.geom
            ${SHADER_DIR}/*.tesc
            ${SHADER_DIR}/*.tese
            ${SHADER_DIR}/*.mesh
            ${SHADER_DIR}/*.task
            ${SHADER_DIR}/*.glsl
    )

    foreach(SHADER ${SHADERS})
        file(RELATIVE_PATH rel_shader ${CMAKE_CURRENT_SOURCE_DIR} ${SHADER})
        add_shader(${TARGET} ${rel_shader})
    endforeach()

    file(GLOB_RECURSE SHADERS
            ${SHADER_DIR}/*.hlsl
    )

    foreach(SHADER ${SHADERS})
        file(RELATIVE_PATH rel_shader ${CMAKE_CURRENT_SOURCE_DIR} ${SHADER})
        add_hlsl_shader(${TARGET} ${rel_shader})
    endforeach()
endfunction()

# Function to compile HLSL shaders (since you have .hlsl files)
function(add_hlsl_shader TARGET SHADER)
    find_program(GLSLANG_VALIDATOR
            NAMES glslangValidator
            HINTS
            $ENV{VULKAN_SDK}/Bin
            $ENV{VULKAN_SDK}/Bin32
    )

    if(NOT GLSLANG_VALIDATOR)
        message(FATAL_ERROR "glslangValidator not found!")
    endif()

    set(current_shader_path ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})
    get_filename_component(current_shader_name ${SHADER} NAME_WE)
    set(current_output_path ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shaders/${current_shader_name}.spv)

    # Determine shader stage from filename
    if(current_shader_name MATCHES ".*vertex.*" OR current_shader_name MATCHES ".*_vs")
        set(shader_stage "vert")
    elseif(current_shader_name MATCHES ".*fragment.*" OR current_shader_name MATCHES ".*pixel.*" OR current_shader_name MATCHES ".*_ps")
        set(shader_stage "frag")
    elseif(current_shader_name MATCHES ".*mesh.*" OR current_shader_name MATCHES ".*_ms")
        set(shader_stage "mesh")
    elseif(current_shader_name MATCHES ".*task.*" OR current_shader_name MATCHES ".*_as")
        set(shader_stage "task")
    else()
        message(FATAL_ERROR "Cannot determine shader stage for HLSL file ${SHADER}")
    endif()

    # Create output directory
    get_filename_component(current_output_dir ${current_output_path} DIRECTORY)
    file(MAKE_DIRECTORY ${current_output_dir})

    # Add custom command for HLSL compilation
    add_custom_command(
            OUTPUT ${current_output_path}
            COMMAND ${GLSLANG_VALIDATOR}
            -V                          # Generate SPIR-V
            -D                          # HLSL mode
            -S ${shader_stage}          # Specify shader stage
            -e main                     # Entry point
            --target-env ${VULKAN_TARGET_ENV}  # Target Vulkan version from SDK
            -o ${current_output_path}   # Output file
            ${current_shader_path}      # Input file
            DEPENDS ${current_shader_path}
            COMMENT "Compiling HLSL shader ${SHADER}"
            VERBATIM
    )

    # Add shader to target
    set_source_files_properties(${current_output_path} PROPERTIES GENERATED TRUE)
    target_sources(${TARGET} PRIVATE ${current_output_path})

    # Add to global list of SPIR-V binaries
    set(SPIRV_BINARY_FILES ${SPIRV_BINARY_FILES} ${current_output_path} CACHE INTERNAL "List of SPIR-V binary files")
endfunction()

# Create custom target for shader compilation only
# This should be called after all shaders have been added
function(create_shader_target)
    if(SPIRV_BINARY_FILES)
        add_custom_target(compile_shaders
                DEPENDS ${SPIRV_BINARY_FILES}
                COMMENT "Compiling all shaders"
        )

        # Also create a clean target for shaders
        add_custom_target(clean_shaders
                COMMAND ${CMAKE_COMMAND} -E remove ${SPIRV_BINARY_FILES}
                COMMENT "Cleaning compiled shaders"
        )
    else()
        add_custom_target(compile_shaders
                COMMENT "No shaders to compile"
        )

        add_custom_target(clean_shaders
                COMMENT "No shaders to clean"
        )
    endif()
endfunction()