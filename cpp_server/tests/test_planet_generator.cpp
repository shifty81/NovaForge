// Tests for: Planet Generator Tests
#include "test_log.h"
#include "pcg/planet_generator.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Planet Generator Tests ====================

static void testPlanetGeneration() {
    std::cout << "\n=== Planet Generation ===" << std::endl;
    pcg::PlanetGenerator gen;
    auto planet = gen.generate(555, pcg::PlanetType::Rocky);
    assertTrue(planet.radius >= 2000.0f && planet.radius <= 8000.0f, "Rocky radius in range");
    assertTrue(planet.gravity > 0.0f, "Planet has gravity");
}

static void testPlanetDeterminism() {
    std::cout << "\n=== Planet Determinism ===" << std::endl;
    pcg::PlanetGenerator gen;
    auto p1 = gen.generate(888, pcg::PlanetType::Ice);
    auto p2 = gen.generate(888, pcg::PlanetType::Ice);
    assertTrue(approxEqual(p1.radius, p2.radius), "Same seed same radius");
    assertTrue(approxEqual(p1.gravity, p2.gravity), "Same seed same gravity");
}

static void testPlanetResources() {
    std::cout << "\n=== Planet Resources ===" << std::endl;
    pcg::PlanetGenerator gen;
    auto planet = gen.generate(333, pcg::PlanetType::Rocky);
    assertTrue(!planet.resources.empty(), "Rocky planet has resources");
    assertTrue(static_cast<int>(planet.resources.size()) >= 3, "Rocky has at least 3 resources");
}

static void testPlanetTerraformable() {
    std::cout << "\n=== Planet Terraformable ===" << std::endl;
    assertTrue(pcg::PlanetGenerator::isTerraformable(pcg::PlanetType::Rocky), "Rocky is terraformable");
    assertTrue(pcg::PlanetGenerator::isTerraformable(pcg::PlanetType::Desert), "Desert is terraformable");
    assertTrue(!pcg::PlanetGenerator::isTerraformable(pcg::PlanetType::Gas), "Gas is not terraformable");
    assertTrue(!pcg::PlanetGenerator::isTerraformable(pcg::PlanetType::Lava), "Lava is not terraformable");
}


void run_planet_generator_tests() {
    testPlanetGeneration();
    testPlanetDeterminism();
    testPlanetResources();
    testPlanetTerraformable();
}
