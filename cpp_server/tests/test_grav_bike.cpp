// Tests for: Grav Bike Tests
#include "test_log.h"
#include "pcg/grav_bike_generator.h"

using namespace atlas;

// ==================== Grav Bike Tests ====================

static void testGravBikeGeneration() {
    std::cout << "\n=== Grav Bike Generation ===" << std::endl;
    pcg::GravBikeGenerator gen;
    auto bike = gen.generate(1000, "Solari");
    assertTrue(bike.config.max_speed >= 30.0f, "Speed above minimum");
    assertTrue(bike.config.fuel_capacity > 0.0f, "Has fuel capacity");
}

static void testGravBikeDeterminism() {
    std::cout << "\n=== Grav Bike Determinism ===" << std::endl;
    pcg::GravBikeGenerator gen;
    auto b1 = gen.generate(2000, "Veyren");
    auto b2 = gen.generate(2000, "Veyren");
    assertTrue(approxEqual(b1.config.max_speed, b2.config.max_speed), "Same seed same speed");
    assertTrue(approxEqual(b1.hull_strength, b2.hull_strength), "Same seed same hull");
}


void run_grav_bike_tests() {
    testGravBikeGeneration();
    testGravBikeDeterminism();
}
