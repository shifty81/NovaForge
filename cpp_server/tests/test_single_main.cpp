/**
 * Per-system test runner template for NovaForge.
 *
 * Each per-system test executable compiles this file alongside a single
 * test_*.cpp file.  The preprocessor macro NOVAFORGE_TEST_FUNC must be
 * defined to the name of the run_*_tests() function (e.g.
 * run_capacitor_system_tests).
 *
 * This provides the global test counters that test_log.h requires and a
 * minimal main() that calls only the one requested test suite.
 *
 * Usage in CMake:
 *   add_executable(test_capacitor_system
 *       tests/test_single_main.cpp
 *       tests/test_capacitor_system.cpp)
 *   target_compile_definitions(test_capacitor_system PRIVATE
 *       NOVAFORGE_TEST_FUNC=run_capacitor_system_tests)
 */
#include <iostream>

// Global test counters — same as test_main.cpp
int testsRun = 0;
int testsPassed = 0;

// Forward-declare the single test function selected at compile time.
// The NOVAFORGE_TEST_FUNC macro is set per-target in CMake.
#ifndef NOVAFORGE_TEST_FUNC
#error "NOVAFORGE_TEST_FUNC must be defined (e.g. run_capacitor_system_tests)"
#endif

extern void NOVAFORGE_TEST_FUNC();

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Nova Forge — Single System Test" << std::endl;
    std::cout << "========================================" << std::endl;

    NOVAFORGE_TEST_FUNC();

    std::cout << "\n========================================" << std::endl;
    std::cout << "Results: " << testsPassed << "/" << testsRun << " tests passed" << std::endl;
    std::cout << "========================================" << std::endl;

    return (testsPassed == testsRun) ? 0 : 1;
}
