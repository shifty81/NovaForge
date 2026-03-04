#pragma once
/**
 * Shared test infrastructure for NovaForge server tests.
 *
 * Provides assertion helpers, approximate comparison, and a component-add
 * shorthand used by every per-system test file.
 */

#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <iostream>
#include <string>
#include <cmath>
#include <memory>

// Global test counters — defined in test_main.cpp
extern int testsRun;
extern int testsPassed;

inline void assertTrue(bool condition, const std::string& testName) {
    testsRun++;
    if (condition) {
        testsPassed++;
        std::cout << "  \xe2\x9c\x93 " << testName << std::endl;
    } else {
        std::cout << "  \xe2\x9c\x97 " << testName << " FAILED" << std::endl;
    }
}

inline bool approxEqual(float a, float b, float epsilon = 0.01f) {
    return std::fabs(a - b) < epsilon;
}

/// Helper to add a component and return a raw pointer to it
template<typename T>
T* addComp(atlas::ecs::Entity* e) {
    auto c = std::make_unique<T>();
    T* ptr = c.get();
    e->addComponent(std::move(c));
    return ptr;
}
