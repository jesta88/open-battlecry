# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Open Battlecry is an open-source modern remaster of Warlords Battlecry 2/3 by Infinite Interactive. The original game source code (C-style C++) is in the `ORIGINAL DO NOT SAVE .../` folder for reference.

The various `src/` through `src6/` directories are prior iteration attempts from earlier repos — each was abandoned and restarted as the author learned more about game engine architecture and low-level programming. This repo consolidates all of them. The next iteration will be a clean, serious implementation.

### Technical Constraints
- **Pure modern C** (C23 where supported, C17 minimum)
- **Modern Vulkan backend** — dynamic rendering, descriptor heaps, bindless resources
- **Minimal dependencies** — prefer hand-written solutions over libraries
- **Windows-only for now**, but architecture should allow adding platforms later

## Build Commands

```bash
# Configure (Ninja generator, used by CMakeSettings.json)
cmake -G Ninja -B build -S .

# Build all targets
cmake --build build

# Build a specific target
cmake --build build --target game
cmake --build build --target engine
cmake --build build --target renderer
cmake --build build --target shaders
cmake --build build --target asset_converter
cmake --build build --target texture_viewer
```

Requires: Vulkan SDK installed with `VULKAN_SDK` environment variable set.

## CMake Build Targets

- **game** - Main executable (links to engine)
- **engine** - Core engine shared library (builds from src6/ and src/ sources)
- **renderer** - Vulkan graphics shared library
- **shaders** - Custom target: compiles GLSL -> SPIR-V -> C headers
- **shader_compiler** - Tool that converts SPIR-V binaries to C headers
- **asset_converter** - Converts game assets with BC7 texture compression
- **texture_viewer** - KTX texture inspector (Win32 GUI app)

## Repository Layout

The `src/` through `src6/` directories are **prior iterations** (reference/archive). Each explored different engine architectures. Key reusable ideas and patterns can be found across them, but new work should not build on them directly.

- `ORIGINAL DO NOT SAVE .../` — Original Warlords Battlecry source (C-style C++, reference only)
- `src/` — First iteration; includes tools (asset_converter, shader_compiler, texture_viewer), shaders, and vendored third-party libs
- `src2/` through `src5/` — Intermediate iterations exploring modular architecture, job systems, client/server splits
- `src6/` — Most recent iteration with modular engine structure (core, graphics, system, resources subsystems)
- `cmake/` — CMake helper modules (FetchContent for deps, shader compilation, compiler defaults)
- `assets/` — Game assets (fonts, localization)
- `scripts/` — Build scripts (PowerShell shader compilation)

### Entry Point Pattern (from src6)
The game defines `engine_main()` returning an `engine_desc` struct with callbacks: `init`, `quit`, `load_resources`, `unload_resources`, `update(float delta_time)`, `draw`.

## Code Conventions

- **Language:** Pure C — C23 where supported, C17 minimum. No C++.
- **Style:** Microsoft-based `.clang-format` — no column limit, left-aligned pointers, no include sorting
- **Naming:** `snake_case` for functions and variables; module prefixes (e.g., `gfx_`, `wb_`)
- **Types:** Custom base types defined in `src/engine/std.h` (`uint8_t`..`uint64_t`, `bool` via `_Bool`)
- **Memory:** Custom allocator interface (`engine_allocator`), VMA for Vulkan, mimalloc available
- **Platform:** Windows-only for now. Custom minimal Win32 headers in `src6/system/windows/` instead of `<windows.h>`. Architecture should remain platform-abstractable for future portability.
- **Vulkan approach:** Modern — dynamic rendering, descriptor heaps, bindless resources
- **Compiler flags:** MSVC: `/Zc:preprocessor /fp:fast /arch:AVX`; GCC: `-mavx -std=c2x`; AVX2 baseline via `cmake/Options.cmake`; LTO for release
- **Build caching:** sccache/ccache auto-detected via `cmake/Options.cmake`
- **Dependencies:** Minimal. Prefer hand-rolled solutions. Current vendored: Volk (Vulkan loader), VMA, KTX, STB, BC7E, mimalloc
