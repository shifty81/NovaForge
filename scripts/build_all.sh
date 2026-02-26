#!/bin/bash
# Nova Forge - Build All Targets
# Works on Windows (Git Bash / MSYS2), Linux, and macOS.
#
# On Windows this script auto-detects Visual Studio and vcpkg so you can
# run it from Git Bash exactly as you would the .bat file.  It builds the
# engine, editor, client, server, and tests into build/bin/.
#
# Usage:
#   ./build_all.sh                     Build all targets (Release)
#   ./build_all.sh --debug             Build with debug symbols
#   ./build_all.sh --clean             Clean before building
#   ./build_all.sh --test              Run tests after build
#   ./build_all.sh --clean --debug --test   Combine flags

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

# --- Logging Setup ---
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"
LOG_FILE="$LOG_DIR/build_all_$(date '+%Y%m%d_%H%M%S').log"
exec > >(tee -a "$LOG_FILE") 2>&1
echo "Log file: $LOG_FILE"
echo ""

# ── Detect platform ──────────────────────────────────────────────
IS_WINDOWS=false
case "$(uname -s)" in
    MINGW*|MSYS*|CYGWIN*) IS_WINDOWS=true ;;
    *) IS_WINDOWS=false ;;
esac

if [ "$IS_WINDOWS" = true ]; then
    echo "================================================"
    echo "Nova Forge - Windows Build (All Targets)"
    echo "================================================"
else
    echo "================================================"
    echo "Nova Forge - Build All Targets"
    echo "================================================"
fi
echo ""

# ── Parse arguments ──────────────────────────────────────────────
BUILD_TYPE="${1:-Release}"
CLEAN_BUILD=false
RUN_TESTS=false

for arg in "$@"; do
    case $arg in
        --debug)   BUILD_TYPE="Debug" ;;
        --clean)   CLEAN_BUILD=true ;;
        --test)    RUN_TESTS=true ;;
        --help|-h)
            echo "Usage: $0 [Release|Debug] [--debug] [--clean] [--test]"
            echo ""
            echo "  Release    Build optimised binaries (default)"
            echo "  --debug    Build with debug symbols"
            echo "  --clean    Remove build directory before building"
            echo "  --test     Run tests after a successful build"
            echo ""
            exit 0
            ;;
    esac
done

# ── Dependency check ─────────────────────────────────────────────
echo "Checking dependencies..."

if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found!"
    echo "  Install CMake from https://cmake.org/download/"
    if [ "$IS_WINDOWS" = true ]; then
        echo "  Or enable it in the Visual Studio Installer."
    fi
    exit 1
fi
echo "  CMake $(cmake --version | head -1)"

# ── Compiler / generator detection ───────────────────────────────
CMAKE_EXTRA_ARGS=""
if [ "$IS_WINDOWS" = true ]; then
    # On Windows, look for Visual Studio — g++/clang++ are not required.
    VS_GENERATOR=""

    # Check VS 2022
    for edition in Community Professional Enterprise BuildTools; do
        if [ -d "/c/Program Files/Microsoft Visual Studio/2022/$edition" ] || \
           [ -d "C:/Program Files/Microsoft Visual Studio/2022/$edition" ]; then
            VS_GENERATOR="Visual Studio 17 2022"
            break
        fi
    done

    # Fall back to VS 2019
    if [ -z "$VS_GENERATOR" ]; then
        for edition in Community Professional Enterprise BuildTools; do
            if [ -d "/c/Program Files (x86)/Microsoft Visual Studio/2019/$edition" ] || \
               [ -d "C:/Program Files (x86)/Microsoft Visual Studio/2019/$edition" ]; then
                VS_GENERATOR="Visual Studio 16 2019"
                break
            fi
        done
    fi

    if [ -z "$VS_GENERATOR" ]; then
        echo "ERROR: Visual Studio 2019 or 2022 not found!"
        echo "  Install Visual Studio with the 'Desktop development with C++' workload."
        echo "  https://visualstudio.microsoft.com/downloads/"
        exit 1
    fi
    echo "  Visual Studio: $VS_GENERATOR"
    CMAKE_EXTRA_ARGS="-G \"$VS_GENERATOR\" -A x64"

    # ── Detect vcpkg ─────────────────────────────────────────────
    VCPKG_CMAKE=""
    if [ -f "/c/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
        VCPKG_CMAKE="C:/vcpkg/scripts/buildsystems/vcpkg.cmake"
        echo "  vcpkg: C:\\vcpkg"
    elif [ -f "$USERPROFILE/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
        VCPKG_CMAKE="$USERPROFILE/vcpkg/scripts/buildsystems/vcpkg.cmake"
        echo "  vcpkg: $USERPROFILE\\vcpkg"
    elif [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
        VCPKG_CMAKE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
        echo "  vcpkg: $VCPKG_ROOT"
    else
        echo "  vcpkg: NOT FOUND (dependencies must be installed manually)"
        echo ""
        echo "  Recommended: install vcpkg and the required packages:"
        echo "    cd C:\\"
        echo "    git clone https://github.com/microsoft/vcpkg.git"
        echo "    cd vcpkg && bootstrap-vcpkg.bat"
        echo "    vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows"
        echo ""
    fi

    if [ -n "$VCPKG_CMAKE" ]; then
        CMAKE_EXTRA_ARGS="$CMAKE_EXTRA_ARGS -DCMAKE_TOOLCHAIN_FILE=$VCPKG_CMAKE"
    fi
else
    # Linux / macOS — need g++ or clang++
    if command -v g++ &> /dev/null; then
        echo "  g++ $(g++ --version | head -1)"
    elif command -v clang++ &> /dev/null; then
        echo "  clang++ $(clang++ --version | head -1)"
    else
        echo "ERROR: No C++ compiler found!"
        echo "  Ubuntu/Debian: sudo apt-get install build-essential"
        echo "  macOS: xcode-select --install"
        exit 1
    fi
fi
echo ""

# ── Clean if requested ───────────────────────────────────────────
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning build directory..."
    rm -rf build
    echo ""
fi

# ── Configure ────────────────────────────────────────────────────
BUILD_DIR="build"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Clear CMake cache to avoid generator conflicts
if [ -f "CMakeCache.txt" ]; then
    echo "Clearing CMake cache to avoid generator conflicts..."
    rm -f CMakeCache.txt
    rm -rf CMakeFiles
    echo ""
fi

echo "Configuration:"
echo "  Build Type:   $BUILD_TYPE"
echo "  Build Dir:    $SCRIPT_DIR/$BUILD_DIR"
echo "  Output Dir:   $SCRIPT_DIR/$BUILD_DIR/bin"
echo "  Run Tests:    $RUN_TESTS"
if [ "$IS_WINDOWS" = true ]; then
    echo "  Generator:    $VS_GENERATOR"
    echo "  Architecture: x64"
fi
echo ""

echo "Configuring CMake ($BUILD_TYPE) — all targets..."
eval cmake .. \
    $CMAKE_EXTRA_ARGS \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DUSE_SYSTEM_LIBS=ON \
    -DBUILD_ATLAS_ENGINE=ON \
    -DBUILD_ATLAS_EDITOR=ON \
    -DBUILD_CLIENT=ON \
    -DBUILD_SERVER=ON \
    -DBUILD_ATLAS_TESTS=ON \
    -DBUILD_TESTS=ON

if [ $? -ne 0 ]; then
    echo ""
    echo "================================================"
    echo "CMAKE CONFIGURATION FAILED"
    echo "================================================"
    echo ""
    if [ "$IS_WINDOWS" = true ]; then
        echo "Install dependencies via vcpkg:"
        echo "  vcpkg install glfw3:x64-windows glm:x64-windows glew:x64-windows nlohmann-json:x64-windows"
        echo ""
        echo "Optional audio support:"
        echo "  vcpkg install openal-soft:x64-windows"
    else
        echo "Install dependencies:"
        echo "  Ubuntu/Debian: sudo apt-get install libgl1-mesa-dev libglew-dev libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev"
        echo "  macOS: brew install glfw glm glew nlohmann-json openal-soft"
    fi
    exit 1
fi

# ── Build ────────────────────────────────────────────────────────
echo ""
echo "Building all targets (Engine, Editor, Client, Server, Tests)..."
if [ "$IS_WINDOWS" = true ]; then
    # Use CMake's cross-platform --parallel flag to avoid MSYS2 path
    # translation issues with MSBuild's /m flag.
    cmake --build . --config "$BUILD_TYPE" --parallel
else
    JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
    cmake --build . --config "$BUILD_TYPE" -- -j$JOBS
fi

if [ $? -ne 0 ]; then
    echo ""
    echo "================================================"
    echo "BUILD FAILED"
    echo "================================================"
    echo ""
    echo "Log: $LOG_FILE"
    exit 1
fi

echo ""
echo "================================================"
echo "BUILD SUCCESSFUL"
echo "================================================"
echo ""

# ── List built executables ───────────────────────────────────────
echo "Built executables:"
if [ "$IS_WINDOWS" = true ]; then
    # Multi-config generators (VS) put binaries in bin/<Config>/
    BIN_DIR="bin/$BUILD_TYPE"
    if [ ! -d "$BIN_DIR" ]; then BIN_DIR="bin"; fi
    find "$BIN_DIR" -maxdepth 1 -name "*.exe" 2>/dev/null | while read -r exe; do
        echo "  $(basename "$exe")"
    done
else
    find bin -type f -executable 2>/dev/null | while read -r exe; do
        echo "  $exe"
    done
fi
echo ""

# ── Copy game data alongside binaries ────────────────────────────
if [ "$IS_WINDOWS" = true ]; then
    DATA_DEST="$BIN_DIR/data"
else
    DATA_DEST="bin/data"
fi

if [ ! -d "$DATA_DEST" ] && [ -d "$SCRIPT_DIR/data" ]; then
    echo "Copying game data into output directory..."
    cp -r "$SCRIPT_DIR/data" "$DATA_DEST"
    echo "  data/ copied"
    echo ""
fi

# ── Run tests if requested ───────────────────────────────────────
if [ "$RUN_TESTS" = true ]; then
    echo "================================================"
    echo "Running Tests"
    echo "================================================"
    echo ""
    TEST_FAIL=0

    if [ "$IS_WINDOWS" = true ]; then
        TEST_DIR="$BIN_DIR"
    else
        TEST_DIR="bin"
        if [ ! -d "$TEST_DIR" ]; then TEST_DIR="atlas_tests"; fi
    fi

    ATLAS_TEST="$TEST_DIR/AtlasTests"
    if [ "$IS_WINDOWS" = true ]; then ATLAS_TEST="$TEST_DIR/AtlasTests.exe"; fi
    if [ -f "$ATLAS_TEST" ]; then
        echo "--- Atlas Engine Tests ---"
        "$ATLAS_TEST" || TEST_FAIL=1
        echo ""
    else
        echo "AtlasTests not found at $ATLAS_TEST — skipping engine tests"
    fi

    SRV_TEST="$TEST_DIR/test_systems"
    if [ "$IS_WINDOWS" = true ]; then SRV_TEST="$TEST_DIR/test_systems.exe"; fi
    if [ -f "$SRV_TEST" ]; then
        echo "--- Server System Tests ---"
        "$SRV_TEST" || TEST_FAIL=1
        echo ""
    else
        echo "test_systems not found at $SRV_TEST — skipping server tests"
    fi

    if [ "$TEST_FAIL" -eq 1 ]; then
        echo "WARNING: Some tests failed."
    else
        echo "All tests passed."
    fi
    echo ""
fi

# ── Quick Start guide ────────────────────────────────────────────
echo "================================================"
echo "Quick Start"
echo "================================================"
echo ""
if [ "$IS_WINDOWS" = true ]; then
    echo "  Editor:   $BIN_DIR/AtlasEditor.exe"
    echo "  Client:   $BIN_DIR/atlas_client.exe"
    echo "  Server:   $BIN_DIR/atlas_dedicated_server.exe"
    echo ""
    echo "  Open in Visual Studio:"
    echo "    start build/Atlas.sln"
    echo ""
    echo "  Package for sharing:"
    echo "    powershell Compress-Archive -Path build/$BIN_DIR/* -DestinationPath novaforge_build.zip"
else
    echo "  Editor:  ./build/bin/AtlasEditor"
    echo "  Client:  ./build/bin/atlas_client"
    echo "  Server:  ./build/bin/atlas_dedicated_server"
    echo ""
    echo "  Package for sharing:"
    echo "    zip -r novaforge_build.zip build/bin/"
fi
echo ""
echo "Log: $LOG_FILE"
exit 0
