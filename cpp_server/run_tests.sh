#!/bin/bash
# Test runner for Nova Forge C++ Dedicated Server
# Ensures tests are run from the repository root for correct data path resolution

set -e

# --- Logging Setup ---
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOG_DIR="$SCRIPT_DIR/logs"
mkdir -p "$LOG_DIR"
LOG_FILE="$LOG_DIR/server_tests_$(date '+%Y%m%d_%H%M%S').log"
exec > >(tee -a "$LOG_FILE") 2>&1
echo "Log file: $LOG_FILE"
echo ""

echo "========================================="
echo "Nova Forge Server Test Runner"
echo "========================================="
echo ""

# Repository root is one level up from cpp_server/
REPO_ROOT="$( cd "$SCRIPT_DIR/.." && pwd )"

# Test executable path
TEST_EXECUTABLE="$SCRIPT_DIR/build/bin/test_systems"

# Check if test executable exists
if [ ! -f "$TEST_EXECUTABLE" ]; then
    echo "Error: Test executable not found at $TEST_EXECUTABLE"
    echo "Please build the server first with: ./build.sh"
    exit 1
fi

# Make sure we're in the repository root before running tests
# This ensures the data/ directory is found correctly
cd "$REPO_ROOT"

echo "Running tests from: $(pwd)"
echo "Test executable: $TEST_EXECUTABLE"
echo ""

# Run the tests
"$TEST_EXECUTABLE" "$@"

exit_code=$?

if [ $exit_code -eq 0 ]; then
    echo ""
    echo "========================================="
    echo "✓ All tests passed successfully!"
    echo "========================================="
else
    echo ""
    echo "========================================="
    echo "✗ Tests failed with exit code: $exit_code"
    echo "========================================="
fi

exit $exit_code
