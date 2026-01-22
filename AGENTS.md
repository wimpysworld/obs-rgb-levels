# AGENTS.md

OBS Studio plugin providing an RGB levels filter for independent colour channel adjustment.

## Project overview

- **Language:** C (single source file: `src/plugin-main.c`)
- **Build system:** CMake 3.28+ with OBS Plugin Template
- **Target:** OBS Studio 31.0.0+
- **Platforms:** Linux (primary), Windows, macOS

## Setup

Enter Nix development shell:

```bash
nix develop
```

Or install dependencies manually: `cmake`, `clang-format` (v16+), `cmake-format`

## Build commands

Configure and build for Linux:

```bash
cmake --preset ubuntu-x86_64
cmake --build build_x86_64 --config RelWithDebInfo
```

Available presets: `ubuntu-x86_64`, `macos`, `windows-x64`

## Project structure

```
src/
  plugin-main.c       # All plugin logic
  plugin-support.h    # Generated support header
data/
  rgb_levels.effect   # GPU shader
  locale/en-US.ini    # Localisation strings
cmake/                # OBS plugin template build system
buildspec.json        # Version and build metadata
```

## Code style

C code formatted with clang-format (v16+):

- Indent: 8 spaces (tabs for continuation)
- Column limit: 120
- Braces: Allman style for functions, K&R for control statements
- Pointer alignment: right (`char *foo`)

Format before committing:

```bash
clang-format -i src/plugin-main.c
```

CMake files formatted with cmake-format (config: `.gersemirc`):

```bash
cmake-format -i CMakeLists.txt
```

## Commit conventions

Follow Conventional Commits with scope where applicable:

- `feat:` new features
- `fix:` bug fixes
- `fix(ci):` CI-specific fixes
- `docs:` documentation changes
- `chore:` version bumps, maintenance
- `refactor:` code restructuring

## Architecture notes

Single-file plugin implementing OBS source filter API:

- `rgb_levels_create()` - loads GPU effect shader
- `rgb_levels_render()` - applies RGB min/max scaling per frame
- `rgb_levels_properties()` - defines UI sliders (0-255 range per channel)

Shader in `data/rgb_levels.effect` performs the actual colour transformation.

## Constraints

- OBS Studio 31.0.0 minimum required
- CMake 3.28-3.30 supported
- Linux is the only tested platform; Windows/macOS builds provided but patches welcome
- Version managed in `buildspec.json` (single source of truth)
