include(CheckIPOSupported)

option(RECON_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

# Interprocedural optimization (LTO) for Release-ish builds
check_ipo_supported(RESULT _recon_ipo_ok OUTPUT _recon_ipo_msg)

find_program(RECON_SCCACHE sccache)
find_program(RECON_CCACHE  ccache)

# Detect an MSVC-like driver (cl.exe or clang-cl)
set(_recon_msvc_like FALSE)
if (MSVC OR CMAKE_C_COMPILER_FRONTEND_VARIANT STREQUAL "MSVC")
    set(_recon_msvc_like TRUE)
endif()

function(recon_target_defaults tgt)
    # --- Language mode: C23 everywhere, but don't set C_STANDARD on MSVC-like ---
    if (NOT _recon_msvc_like AND c_std_23 IN_LIST CMAKE_C_COMPILE_FEATURES)
        target_compile_features(${tgt} PUBLIC c_std_23)
        set_property(TARGET ${tgt} PROPERTY C_EXTENSIONS OFF)
    else()
        # MSVC/clang-cl (or older CMake without c_std_23): drive flag directly
        target_compile_options(${tgt} PUBLIC /std:clatest)
        # Do NOT set C_STANDARD here; avoids CMake injecting /std:c17 as well.
    endif()

    # --- Warnings, hygiene ---
    if (_recon_msvc_like)
        target_compile_options(${tgt} PRIVATE
                /permissive- /utf-8
                /Zc:preprocessor /Zc:inline /Zc:forScope /Zc:strictStrings
                /W4 /wd5105
        )
    else()
        target_compile_options(${tgt} PRIVATE
                -Wall -Wextra -Wpedantic
                -Wno-unused-parameter -Wno-unused-function
                -fvisibility=hidden
                -ffunction-sections -fdata-sections
                -ffp-contract=fast
        )
    endif()
    if (RECON_WARNINGS_AS_ERRORS)
        target_compile_options(${tgt} PRIVATE
                $<$<BOOL:${_recon_msvc_like}>:/WX>
                $<$<NOT:$<BOOL:${_recon_msvc_like}>>:-Werror>
        )
    endif()

    # --- CPU baseline: AVX2 on x86-64 ---
    if (CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|X86_64|AMD64|amd64)$")
        if (_recon_msvc_like)
            target_compile_options(${tgt} PRIVATE /arch:AVX2)
        else()
            target_compile_options(${tgt} PRIVATE -march=x86-64-v3)
        endif()
    endif()

    # --- Config-specific opts ---
    if (_recon_msvc_like)
        target_compile_options(${tgt} PRIVATE
                $<$<CONFIG:Debug>:/Od /Ob0>                                  # keep RTC in Debug
                $<$<CONFIG:Release,RelWithDebInfo,MinSizeRel>:/O2 /Oi /Gy /fp:fast>
        )
        target_link_options(${tgt} PRIVATE
                $<$<CONFIG:Debug>:/INCREMENTAL>
                $<$<NOT:$<CONFIG:Debug>>:/INCREMENTAL:NO /LTCG>
                $<$<CONFIG:Release,RelWithDebInfo,MinSizeRel>:/OPT:REF /OPT:ICF>
        )
    else()
        target_compile_options(${tgt} PRIVATE
                $<$<CONFIG:Debug>:-O0>
                $<$<CONFIG:RelWithDebInfo,Release>:-O2>
                $<$<CONFIG:MinSizeRel>:-Os>
        )
        target_link_options(${tgt} PRIVATE
                $<$<CONFIG:Release,RelWithDebInfo,MinSizeRel>:LINKER:--gc-sections,--as-needed>
        )
    endif()

    # --- Windows UTF-8 + Win8+ ---
    target_compile_definitions(${tgt} PUBLIC
            $<$<PLATFORM_ID:Windows>:UNICODE;_UNICODE;WINVER=0x0602;_WIN32_WINNT=0x0602;NOMINMAX;WIN32_LEAN_AND_MEAN>
            $<$<BOOL:${_recon_msvc_like}>:_CRT_SECURE_NO_WARNINGS>
    )

    # PIC and IPO
    set_target_properties(${tgt} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    if (_recon_ipo_ok)
        set_property(TARGET ${tgt} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
        set_property(TARGET ${tgt} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO ON)
    endif()

    if (RECON_SCCACHE)
        set_property(TARGET ${tgt} PROPERTY C_COMPILER_LAUNCHER   "${RECON_SCCACHE}")
        set_property(TARGET ${tgt} PROPERTY CXX_COMPILER_LAUNCHER "${RECON_SCCACHE}")
    elseif (RECON_CCACHE)
        set_property(TARGET ${tgt} PROPERTY C_COMPILER_LAUNCHER   "${RECON_CCACHE}")
        set_property(TARGET ${tgt} PROPERTY CXX_COMPILER_LAUNCHER "${RECON_CCACHE}")
    endif()
endfunction()