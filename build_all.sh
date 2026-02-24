#!/bin/bash
# Nova Forge - Unified Build Script
# Builds engine, editor, client, and server into a single output directory.
# The resulting build/bin/ folder can be zipped and shared for testing.

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

echo "================================================"
echo "Nova Forge - Unified Build (All Targets)"
echo "================================================"
echo ""

# ── Parse arguments ──────────────────────────────────────────────
BUILD_TYPE="${1:-Release}"
CLEAN_BUILD=false

for arg in "$@"; do
    case $arg in
        --debug)   BUILD_TYPE="Debug" ;;
        --clean)   CLEAN_BUILD=true ;;
        --help|-h)
            echo "Usage: $0 [Release|Debug] [--debug] [--clean]"
            echo ""
            echo "  Release    Build optimised binaries (default)"
            echo "  --debug    Build with debug symbols"
            echo "  --clean    Remove build directory before building"
            echo ""
            exit 0
            ;;
    esac
done

# ── Dependency check ─────────────────────────────────────────────
echo "Checking dependencies..."
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found!"
    echo "Please install CMake from https://cmake.org/download/"
    exit 1
fi
echo "  ✓ CMake $(cmake --version | head -1)"

if command -v g++ &> /dev/null; then
    echo "  ✓ g++ $(g++ --version | head -1)"
elif command -v clang++ &> /dev/null; then
    echo "  ✓ clang++ $(clang++ --version | head -1)"
else
    echo "ERROR: No C++ compiler found!"
    echo "Install build-essential (Linux) or Xcode (macOS)"
    exit 1
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

echo "Configuration:"
echo "  Build Type:   $BUILD_TYPE"
echo "  Build Dir:    $SCRIPT_DIR/$BUILD_DIR"
echo "  Output Dir:   $SCRIPT_DIR/$BUILD_DIR/bin"
echo ""

echo "Configuring CMake ($BUILD_TYPE) — all targets..."
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DUSE_SYSTEM_LIBS=ON \
    -DBUILD_ATLAS_ENGINE=ON \
    -DBUILD_ATLAS_EDITOR=ON \
    -DBUILD_CLIENT=ON \
    -DBUILD_SERVER=ON \
    -DBUILD_ATLAS_TESTS=ON

if [ $? -ne 0 ]; then
    echo ""
    echo "================================================"
    echo "CMAKE CONFIGURATION FAILED"
    echo "================================================"
    echo ""
    echo "Install dependencies:"
    echo "  Ubuntu/Debian: sudo apt-get install libgl1-mesa-dev libglew-dev libglfw3-dev libglm-dev nlohmann-json3-dev libopenal-dev"
    echo "  macOS: brew install glfw glm glew nlohmann-json openal-soft"
    exit 1
fi

# ── Build ────────────────────────────────────────────────────────
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
echo ""
echo "Building all targets with $JOBS parallel jobs..."
cmake --build . --config "$BUILD_TYPE" -- -j$JOBS

if [ $? -eq 0 ]; then
    echo ""
    echo "================================================"
    echo "BUILD SUCCESSFUL"
    echo "================================================"
    echo ""
    echo "Output directory: $SCRIPT_DIR/$BUILD_DIR/bin/"
    echo ""
    echo "Executables:"
    find bin -type f -executable 2>/dev/null | while read -r exe; do
        echo "  $exe"
    done
    echo ""

    # ── Copy data alongside binaries for a portable package ──────
    DATA_DEST="$SCRIPT_DIR/$BUILD_DIR/bin/data"
    if [ ! -d "$DATA_DEST" ] && [ -d "$SCRIPT_DIR/data" ]; then
        echo "Copying game data into output directory..."
        cp -r "$SCRIPT_DIR/data" "$DATA_DEST"
        echo "  ✓ data/ copied to $DATA_DEST"
        echo ""
    fi

    echo "To share for testing:"
    echo "  cd $SCRIPT_DIR"
    echo "  zip -r novaforge_build.zip build/bin/"
    echo ""
    echo "Log: $LOG_FILE"
    exit 0
else
    echo ""
    echo "================================================"
    echo "BUILD FAILED"
    echo "================================================"
    echo ""
    echo "Log: $LOG_FILE"
    exit 1
fi
