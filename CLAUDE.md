# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Open Battlecry is an open-source modern remaster of Warlords Battlecry 2/3 by Infinite Interactive. The original game source code (C-style C++) is in the `ORIGINAL DO NOT SAVE .../` folder for reference.

The `src2/` through `src7/` directories are prior iteration attempts from earlier repos — each was abandoned and restarted. They contain reference code (job systems, allocators, Vulkan renderers, asset tools) but new work builds in `src/`.

### Technical Constraints
- **Pure modern C** (C23 where supported, C17 minimum)
- **Modern Vulkan 1.4 backend** — dynamic rendering, synchronization2, descriptor indexing (bindless)
- **Minimal dependencies** — prefer hand-written solutions over libraries
- **Windows-only for now**, but architecture should allow adding platforms later

## Build Commands

```bash
# Configure with CMake presets (recommended)
cmake --preset debug
cmake --build --preset debug

# Build specific targets
cmake --build --preset debug --target game
cmake --build --preset debug --target asset_baker

# Asset baking (run once to convert WBC3 data)
./build/bin/asset_baker.exe "C:/Games/Warlords Battlecry 3" "baked"

# Run game (requires baked data directory)
./build/bin/game.exe "baked"
```

Requires: Vulkan SDK installed (for shader compilation). Vulkan headers are auto-fetched if SDK is absent, but `glslangValidator` needs the SDK.

## CMake Build Targets

- **game** — Main executable. Links to Volk + SDL3. Loads from baked data directory.
- **asset_baker** — Offline tool. Converts WBC3 install data into baked assets. No Vulkan/SDL3 dependency.
- **compile_shaders** — Custom target: compiles GLSL → SPIR-V via `glslangValidator`. Game depends on this.

## Repository Layout

- `src/` — **Active game source** (the current iteration)
- `tools/` — **Asset baker tool** (converts WBC3 data to modern formats)
- `src2/` through `src7/` — Prior iterations (reference/archive only)
- `ORIGINAL DO NOT SAVE .../` — Original WBC3 source (C-style C++, reference only)
- `cmake/` — CMake modules: `Options.cmake` (compiler defaults), `CompileShaders.cmake`, `Fetch*.cmake` (Volk, SDL3, VMA, etc.)
- `assets/` — Game assets (fonts, localization — copied to build output)
- `baked/` — Baked output from asset_baker (not checked in, generated)

### Active Source Structure (`src/`)

```
src/
  main.c                — Game loop, state machine (menu/play/pause), map init, terrain rendering,
                           input, selection, camera, minimap, baked data loading
  gfx.h/gfx.c          — Vulkan renderer: batched SSBO sprites, bindless textures, camera, keyboard+mouse input
  map.h/map.c           — Tile-based map: terrain types, movement costs, cell flags, procedural test maps (CELL_W=32, CELL_H=24)
  pathfind.h/pathfind.c — A* pathfinding: bucketed open list, Chebyshev heuristic, 8-directional
  entity.h/entity.c     — Unit types, entities, state machine, combat, path-following movement, Y-sorted draw, separation
  building.h/building.c — Building types, placement, construction, production queues, passive income
  resource.h/resource.c — 4-resource economy (gold/metal/crystal/stone), per-team banks
  mine.h/mine.c         — Resource nodes, auto-generation, claiming by proximity
  font.h/font.c         — TTF font atlas baking (stb_truetype), screen-fixed text drawing via sprite regions
  audio.h/audio.c       — SDL3 audio: WAV loading from memory, multi-channel playback via SDL_AudioStream
  hud.h/hud.c           — HUD overlay: unit info panel, resource bar, FPS counter
  menu.h/menu.c         — Main menu + pause menu with keyboard navigation
  baked_format.h        — Shared binary record structs (used by both game and asset_baker)
  baked_loader.h/c      — Load terrain BMPs, unit sprites, unit stats, sounds from baked directory
  xcr.h/xcr.c           — WBC3 XCR archive loader with decryption (used by asset_baker at build time)
  image.h/image.c       — RLE sprite decoder + BMP decoder (via stb_image)
  ani.h/ani.c           — WBC3 ANI metadata parser + animation player (frame cycling, UV computation)
  file_io.h/file_io.c   — Binary file reader
  stb_image.c/h         — stb_image (BMP only)
  stb_truetype.c/h      — stb_truetype (TTF rasterization)
  shaders/
    sprite.vert          — SSBO-based batched vertex shader with UV sub-regions
    sprite.frag          — Bindless texture sampling with nonuniformEXT
```

### Tools Structure (`tools/`)

```
tools/
  asset_baker.c          — CLI tool: reads WBC3 install, writes baked/ output
  cfg_parse.h/cfg_parse.c — Army.cfg fixed-width column parser (unit codes, names, classification)
  stb_image_write.h/c   — BMP writing for baked sprite sheets
```

### Baked Data Structure (output of asset_baker)

```
baked/
  terrain/               — BMP tiles per terrain type (grass.bmp, dirt.bmp, ...)
  sides/<side_name>/     — Per-side unit sprites (BMP) + ANI metadata
  sounds/sfx/            — Sound effects (WAV, from SoundFX.xcr)
  sounds/voices/<side>/  — Per-side voice lines (WAV)
  data/units.bin         — Binary unit definitions (baked_unit_def records)
```

### Architecture

**Two-stage asset pipeline:**
1. `asset_baker` reads WBC3 install → decrypts XCR archives → decodes RLE sprites to BMP → parses Army.cfg → writes baked output
2. `game` reads baked directory → loads BMPs via stb_image → loads binary configs → uploads to GPU

**Renderer (`gfx.c`):** Vulkan 1.4 with SDL3 for windowing. All sprites batched into a single SSBO, drawn in one `vkCmdDraw` call per frame. Bindless textures via descriptor indexing (4096 max). Camera offset via push constants. Textures are `R8G8B8A8_UNORM`.

**GPU sprite struct (48 bytes):** `position[2], size[2], uv_offset[2], uv_scale[2], texture_index, color, pad[2]`. UV sub-regions enable sprite sheet frame animation without per-frame texture creation.

**Map system (`map.c`):** Tile grid of `map_cell` (terrain type, height, flags). 13 terrain types matching WBC3 (`TERRAIN_GRASS` through `TERRAIN_IMPASSABLE`). Cell flags for building/mine occupancy. 4 movement modes (land/fly/sea/float) with per-terrain cost table. Cell display size: 32×24 pixels (4:3, matching WBC3 tile art). Procedural test map generator for development.

**Pathfinding (`pathfind.c`):** A* with bucketed open list (4000 buckets), Chebyshev distance heuristic, 8-directional expansion. Generation counter avoids per-search memset. Operates on cell grid — pixel↔cell conversion in entity.c. Automatically routes around buildings via cell flags.

**Entity system:** AoS layout. `unit_type` holds shared data (textures, ANI, combat stats from baked_unit_def). `unit` holds per-instance state (position, path, state machine, health, animation player). State machine: IDLE → WALKING (path-following) → FIGHTING → DYING. Auto-aggro AI when idle. Soft unit separation forces. Y-sorted draw order for isometric depth.

**Economy:** 4 resource types per team. Mines auto-generate resources. Buildings cost resources, have construction timers, and can produce units. Production queue depth = 1 per building.

**Binary config format:** `[magic][version][count][record_size][records...]`. Fixed-size records loadable with single fread. `baked_format.h` shared between baker and game ensures struct agreement.

### WBC3 Format Details

- **XCR archives:** 28-byte file header + 532-byte resource headers (MSVC 4-byte aligned). Resources may be encrypted. Supports BMP, RLE, ANI, WAV, CFG, TER, FEA, ARM, BUI, GOM types.
- **RLE sprites:** 2-byte ID ("RL"/"RS") + header with 8×256 RGB565 palettes. Frames laid out horizontally, 8 directions vertically.
- **ANI metadata:** 234 bytes = 9 × 26-byte `xANIMATION_TYPE` (packed). Old format: 180 bytes = 9 × 20. Detected by resource size.
- **Animation suffixes:** `.RLE` (stand), `W.RLE` (walk), `F.RLE` (fight), `D.RLE` (die), `A.RLE` (ambient), `Z.RLE` (special), `C.RLE` (convert), `S.RLE` (spell), `I.RLE` (interface).
- **Army.cfg:** Fixed-width columns in `Assets/Data/System.xcr`. Code at col 4, status flags at col 9, name at col 62+. [ARMIES] and [HEROES] sections define all 200 unit types.

## Code Conventions

- **Language:** Pure C — C23 where supported, C17 minimum. No C++.
- **Style:** Microsoft-based `.clang-format` — no column limit, left-aligned pointers, no include sorting
- **Naming:** `snake_case` for functions and variables; module prefixes (`gfx_`, `map_`, `pathfind_`, `unit_`, `building_`, `mine_`, `resource_`, `font_`, `audio_`, `hud_`, `menu_`, `baked_`)
- **Types:** Standard C headers (`<stdint.h>`, `<stdbool.h>`), not custom redefinitions
- **Vulkan approach:** Vulkan 1.4 — dynamic rendering, synchronization2, descriptor indexing (bindless), per-swapchain-image semaphores
- **Compiler defaults:** Applied via `recon_target_defaults(target)` from `cmake/Options.cmake` — C23, AVX2, `/W4`, LTO for release, sccache/ccache
- **Dependencies:** Volk (Vulkan loader), SDL3 (windowing + audio), stb_image (BMP), stb_truetype (TTF), stb_image_write (baker only). Vulkan-Headers auto-fetched if SDK absent.
