# Building Nova Forge

> **Single source of truth** for building, running, and testing Nova Forge.
> All other build instructions in the project point here.

---

## Prerequisites

| Dependency | Version | Notes |
|------------|---------|-------|
| **CMake** | 3.15+ | Build system generator |
| **C++17 compiler** | GCC 9+, Clang 10+, MSVC 2019+ | |
| **GLFW3** | 3.3+ | Window/input |
| **GLEW** | 2.1+ | OpenGL extension loading |
| **GLM** | 0.9.9+ | Math library |
| **nlohmann-json** | 3.x | JSON parsing |
| **FreeType** | 2.x | Font rendering |
| **OpenAL** | *(optional)* | Audio |

### Linux (Ubuntu / Debian)

```bash
sudo apt-get install build-essential cmake \
  libgl1-mesa-dev libglew-dev libglfw3-dev libglm-dev \
  nlohmann-json3-dev libopenal-dev libfreetype-dev
```

### macOS

```bash
brew install cmake glfw glm glew nlohmann-json openal-soft freetype
```

### Windows

Install [Visual Studio 2022](https://visualstudio.microsoft.com/) with the
**"Desktop development with C++"** workload, then install dependencies via
[vcpkg](https://github.com/microsoft/vcpkg):

```cmd
cd C:\
git clone https://github.com/microsoft/vcpkg.git
cd vcpkg && bootstrap-vcpkg.bat
vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows ^
              nlohmann-json:x64-windows freetype:x64-windows
```

Optional audio: `vcpkg install openal-soft:x64-windows`

---

## Recommended Build — `build_all`

The **recommended** way to build Nova Forge is the unified `build_all` script.
It builds every target (Engine, Editor, Client with F12 editor tools, Server,
and Tests) into a single `build/bin/` directory that can be zipped and shared.

### Linux / macOS

```bash
./scripts/build_all.sh
```

### Windows (Git Bash or cmd)

```cmd
scripts\build_all.bat
```

### What `build_all` produces

```
build/
└── bin/
    ├── atlas_client          # Game client (F12 toggles editor overlay)
    ├── atlas_dedicated_server # Dedicated server
    ├── AtlasEditor           # Standalone editor
    ├── AtlasTests            # Engine test suite
    ├── test_systems           # Server test suite
    ├── shaders/              # GLSL shaders (copied automatically)
    ├── assets/               # Textures and models
    ├── ui_resources/         # UI assets
    └── data/                 # Game data (JSON — copied automatically)
```

### Flags

| Flag | Effect |
|------|--------|
| `--debug` | Build with debug symbols |
| `--clean` | Remove `build/` before building |
| `--test` | Run engine + server tests after build |
| `--open` | *(Windows)* Open Visual Studio solution after build |

Example:

```bash
./scripts/build_all.sh --clean --test
```

---

## Running the Game

After building with `build_all`:

```bash
# Start the client (includes embedded server for solo play)
./build/bin/atlas_client "YourName"

# Or run a dedicated server separately
./build/bin/atlas_dedicated_server
```

### F12 Editor Overlay

Press **F12** in-game to toggle the editor tool overlay. This gives you access
to all 17 editor panels (PCG preview, ship archetypes, scene graph, etc.)
without leaving the running game.

The overlay is compiled in by default. To strip it for a release build, pass
`-DNOVAFORGE_EDITOR_TOOLS=OFF` to CMake.

---

## Makefile Shortcuts

The root `Makefile` provides convenience targets for common tasks.
Run `make help` to see them all.

| Command | What it does |
|---------|-------------|
| `make build` | Build everything via `scripts/build.sh` (Release) |
| `make build-debug` | Same, but Debug configuration |
| `make build-all` | Build via `scripts/build_all.sh` (unified output) |
| `make build-client-editor` | Client only, with F12 editor tools → `build/bin/atlas_client` |
| `make build-client` | Client only, **without** editor tools (standalone CMake from `cpp_client/`) |
| `make build-server` | Dedicated server only → `cpp_server/build/` |
| `make build-engine` | Atlas Engine static library only |
| `make build-editor` | Standalone Atlas Editor → `build/bin/AtlasEditor` |
| `make test` | Run all tests (server + engine) |
| `make test-server` | Server system tests only |
| `make test-engine` | Engine unit tests only |
| `make clean` | Remove all build directories |
| `make check-deps` | Verify build dependencies are installed |

### Which target should I use?

| Goal | Command |
|------|---------|
| **First time / full build** | `./scripts/build_all.sh` or `make build-all` |
| **Iterate on client with editor tools** | `make build-client-editor` |
| **Iterate on server only** | `make build-server` |
| **Run all tests** | `make test` |
| **Clean everything** | `make clean` |

---

## CMake Options

For manual CMake usage from the project root:

```bash
mkdir -p build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DBUILD_ATLAS_ENGINE=ON \
         -DBUILD_CLIENT=ON \
         -DBUILD_SERVER=ON \
         -DNOVAFORGE_EDITOR_TOOLS=ON
cmake --build . --config Release -j$(nproc)
```

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_ATLAS_ENGINE` | `ON` | Build the Atlas Engine static library |
| `BUILD_CLIENT` | `ON` | Build the game client (`atlas_client`) |
| `BUILD_SERVER` | `ON` | Build the dedicated server |
| `BUILD_ATLAS_EDITOR` | `ON` | Build the standalone Atlas Editor |
| `BUILD_ATLAS_TESTS` | `ON` | Build engine test suite |
| `BUILD_TESTS` | `ON` | Build server test suite |
| `NOVAFORGE_EDITOR_TOOLS` | `ON` | Embed editor tool layer in game client (F12 toggle) |
| `USE_SYSTEM_LIBS` | `ON` | Use system-installed libraries |
| `USE_STEAM_SDK` | `OFF` | Enable Steamworks integration |

---

## Windows — Visual Studio

### Generate & Open Solution

```cmd
scripts\generate_solution.bat --open
```

This creates `build_vs\Atlas.sln` with all targets.

### Build from VS Solution

```cmd
scripts\build_vs.bat
scripts\build_vs.bat --debug --open   # Debug + open VS
```

### Build All Targets (Recommended)

```cmd
scripts\build_all.bat --test
```

---

## Testing

### Run All Tests

```bash
make test
```

### Server Tests Only

```bash
make test-server
# Or manually:
cd cpp_server/build
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTS=ON -DUSE_STEAM_SDK=OFF
cmake --build . --config Release --target test_systems -j$(nproc)
./bin/test_systems
```

### Engine Tests Only

```bash
make test-engine
```

---

## Troubleshooting

### Black screen (HUD visible but no 3D scene)

The renderer needs shader files next to the executable. The `build_all` script
and the CMake post-build step copy shaders automatically. If you run the binary
from an unexpected directory, the `resolvePath()` utility searches up to 6
parent directories and also checks `cpp_client/` sub-paths. If shaders still
aren't found, check the console output for:

```
Failed to load basic shader — scene will not render.
  Ensure shaders/ directory is next to the executable or set CWD to the project root.
```

**Fix**: Run the client from the directory containing `shaders/`, or rebuild
with `build_all` which copies everything to `build/bin/`.

### F12 does nothing

Editor tools must be compiled in. If you built with `make build-client`
(standalone), the editor sources aren't available. Use `make build-client-editor`
or `./scripts/build_all.sh` instead.

### CMake generator conflicts

If you switch between VS generators or between `build-client` (standalone) and
`build-all` (root), stale CMake caches can cause errors. Run `make clean` or
delete the `build/` and `cpp_client/build/` directories.

### Missing dependencies on Windows

```cmd
vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows ^
              nlohmann-json:x64-windows freetype:x64-windows
```

See `docs/guides/VS2022_SETUP_GUIDE.md` for a step-by-step walkthrough.

---

## Deprecated Scripts

The following legacy docs and scripts have been moved to `docs/archive/`
because they reference the old Python-based project or outdated build
workflows:

| Archived File | Reason |
|---------------|--------|
| `getting-started/GETTING_STARTED.md` | References Python server/client |
| `getting-started/3D_CLIENT_QUICKSTART.md` | References Panda3D Python client |
| `getting-started/GUI_QUICKSTART.md` | References pygame 2D client |
| `getting-started/TESTING_AND_LAUNCHER_GUIDE.md` | References Python test framework |
| `QUICK_REFERENCE.md` | References `build_and_test.py` and Python workflow |

The canonical build instructions are in this file (`docs/BUILDING.md`).

---

## Related Documentation

- [Atlas Integration Guide](ATLAS_INTEGRATION.md) — Engine architecture and directory layout
- [Editor Tools Reference](EDITOR_TOOLS.md) — F12 overlay, 32 tools, keybindings
- [Contributing](CONTRIBUTING.md) — How to contribute code and content
- [Coding Guidelines](CODING_GUIDELINES.md) — C++ style conventions
- [Roadmap](ROADMAP.md) — Project status and priorities
