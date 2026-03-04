// Tests for: Asteroid Field Generator Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "pcg/pcg_context.h"
#include "pcg/asteroid_field_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Asteroid Field Generator Tests ====================

static void testAsteroidFieldGeneration() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator basic generation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5000, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 0.8f);
    assertTrue(!field.asteroids.empty(), "Field has asteroids");
    assertTrue(field.totalAsteroids >= 10, "Field has >= 10 asteroids");
    assertTrue(field.fieldRadius > 0.0f, "Field has positive radius");
    assertTrue(field.totalYield > 0.0f, "Field has positive yield");
}

static void testAsteroidFieldDeterminism() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator determinism ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5100, 1 };
    auto f1 = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 0.5f);
    auto f2 = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 0.5f);
    assertTrue(f1.asteroids.size() == f2.asteroids.size(), "Same asteroid count");
    bool allMatch = true;
    for (size_t i = 0; i < f1.asteroids.size(); ++i) {
        if (f1.asteroids[i].type != f2.asteroids[i].type) { allMatch = false; break; }
        if (f1.asteroids[i].radius != f2.asteroids[i].radius) { allMatch = false; break; }
    }
    assertTrue(allMatch, "Same seed → identical asteroid field");
}

static void testAsteroidFieldExplicitCount() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator explicit count ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5200, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 25, 0.6f);
    assertTrue(static_cast<int>(field.asteroids.size()) == 25, "Explicit count of 25 asteroids");
    assertTrue(field.totalAsteroids == 25, "totalAsteroids matches");
}

static void testAsteroidFieldHighSecTypes() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator high-sec types ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5300, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 50, 0.9f);
    // High-sec should have no Mercoxit
    bool hasMercoxit = false;
    int commonCount = 0;
    for (const auto& a : field.asteroids) {
        if (a.type == atlas::pcg::AsteroidType::Mercoxit) hasMercoxit = true;
        if (a.type == atlas::pcg::AsteroidType::Ferrite ||
            a.type == atlas::pcg::AsteroidType::Galvite) commonCount++;
    }
    assertTrue(!hasMercoxit, "High-sec belt has no Mercoxit");
    assertTrue(commonCount > 0, "High-sec belt has common ores");
}

static void testAsteroidFieldYieldCalculation() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator yield calculation ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5400, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 20, 0.5f);
    float calculated = atlas::pcg::AsteroidFieldGenerator::calculateTotalYield(field);
    assertTrue(calculated > 0.0f, "Calculated yield is positive");
    // totalYield should match sum of individual yields.
    float manual = 0.0f;
    for (const auto& a : field.asteroids) manual += a.mineralYield;
    assertTrue(std::abs(calculated - manual) < 0.01f, "calculateTotalYield matches sum");
}

static void testAsteroidFieldPositiveValues() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator positive values ===" << std::endl;
    bool allOk = true;
    for (uint64_t i = 1; i <= 50; ++i) {
        atlas::pcg::PCGContext ctx{ i * 97, 1 };
        auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 0.5f);
        for (const auto& a : field.asteroids) {
            if (a.radius <= 0.0f || a.mineralYield <= 0.0f) { allOk = false; break; }
        }
    }
    assertTrue(allOk, "All asteroids have positive radius and yield");
}

static void testAsteroidFieldScaleFactor() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator scale factor ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5500, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 20, 0.5f);
    bool allPositive = true;
    bool scaleVaries = false;
    float firstScale = 0.0f;
    for (size_t i = 0; i < field.asteroids.size(); ++i) {
        float sf = field.asteroids[i].scaleFactor;
        if (sf <= 0.0f) { allPositive = false; break; }
        if (i == 0) firstScale = sf;
        else if (std::abs(sf - firstScale) > 0.01f) scaleVaries = true;
    }
    assertTrue(allPositive, "All asteroids have positive scale factor");
    assertTrue(scaleVaries, "Scale factors vary across asteroids");
}

static void testAsteroidFieldRockMeshArchive() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator rock mesh archive ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5600, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(
        ctx, 15, 0.6f, "rocks_stylized_color.zip");
    assertTrue(!field.asteroids.empty(), "Field has asteroids");
    bool allTagged = true;
    for (const auto& a : field.asteroids) {
        if (a.meshArchive != "rocks_stylized_color.zip") { allTagged = false; break; }
    }
    assertTrue(allTagged, "All asteroids carry rock mesh archive path");
    // Scale factors should still be set.
    assertTrue(field.asteroids[0].scaleFactor > 0.0f, "First asteroid has positive scale");
}

static void testAsteroidFieldNoArchiveByDefault() {
    std::cout << "\n=== PCG: AsteroidFieldGenerator no archive by default ===" << std::endl;
    atlas::pcg::PCGContext ctx{ 5700, 1 };
    auto field = atlas::pcg::AsteroidFieldGenerator::generate(ctx, 10, 0.8f);
    bool allEmpty = true;
    for (const auto& a : field.asteroids) {
        if (!a.meshArchive.empty()) { allEmpty = false; break; }
    }
    assertTrue(allEmpty, "Without archive arg, meshArchive is empty");
}


void run_asteroid_field_generator_tests() {
    testAsteroidFieldGeneration();
    testAsteroidFieldDeterminism();
    testAsteroidFieldExplicitCount();
    testAsteroidFieldHighSecTypes();
    testAsteroidFieldYieldCalculation();
    testAsteroidFieldPositiveValues();
    testAsteroidFieldScaleFactor();
    testAsteroidFieldRockMeshArchive();
    testAsteroidFieldNoArchiveByDefault();
}
