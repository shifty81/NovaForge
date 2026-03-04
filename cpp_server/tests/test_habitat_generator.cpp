// Tests for: Habitat Generator Tests
#include "test_log.h"
#include "pcg/habitat_generator.h"

using namespace atlas;

// ==================== Habitat Generator Tests ====================

static void testHabitatGeneration() {
    std::cout << "\n=== Habitat Generation ===" << std::endl;
    pcg::HabitatGenerator gen;
    auto habitat = gen.generate(111, 5);
    assertTrue(habitat.module_count > 0, "Habitat has modules");
    assertTrue(habitat.total_levels == 5, "Habitat has 5 levels");
}

static void testHabitatDeterminism() {
    std::cout << "\n=== Habitat Determinism ===" << std::endl;
    pcg::HabitatGenerator gen;
    auto h1 = gen.generate(222, 3);
    auto h2 = gen.generate(222, 3);
    assertTrue(h1.module_count == h2.module_count, "Same seed same module count");
    assertTrue(approxEqual(h1.total_power_draw, h2.total_power_draw), "Same seed same power draw");
}

static void testHabitatPowerBalance() {
    std::cout << "\n=== Habitat Power Balance ===" << std::endl;
    pcg::HabitatGenerator gen;
    auto habitat = gen.generate(444, 4);
    assertTrue(habitat.total_power_draw >= 0.0f, "Power draw non-negative");
    assertTrue(habitat.total_power_generation >= 0.0f, "Power generation non-negative");
    assertTrue(habitat.is_self_sufficient == (habitat.total_power_generation >= habitat.total_power_draw),
               "Self-sufficient flag matches calculation");
}


void run_habitat_generator_tests() {
    testHabitatGeneration();
    testHabitatDeterminism();
    testHabitatPowerBalance();
}
