// Tests for: Turret Generator Tests
#include "test_log.h"
#include "pcg/turret_generator.h"

using namespace atlas;

// ==================== Turret Generator Tests ====================

static void testTurretGeneration() {
    std::cout << "\n=== Turret Generation ===" << std::endl;
    pcg::TurretGenerator gen;
    auto turret = gen.generate(42, pcg::TurretSize::Medium, pcg::TurretType::Energy, "Solari");
    assertTrue(turret.profile.base_damage > 0.0f, "Turret has damage");
    assertTrue(turret.optimal_range > 0.0f, "Turret has range");
    assertTrue(turret.power_draw > 0.0f, "Turret has power draw");
}

static void testTurretDeterminism() {
    std::cout << "\n=== Turret Determinism ===" << std::endl;
    pcg::TurretGenerator gen;
    auto t1 = gen.generate(777, pcg::TurretSize::Small, pcg::TurretType::Projectile, "Veyren");
    auto t2 = gen.generate(777, pcg::TurretSize::Small, pcg::TurretType::Projectile, "Veyren");
    assertTrue(approxEqual(t1.profile.base_damage, t2.profile.base_damage), "Same seed same damage");
    assertTrue(approxEqual(t1.optimal_range, t2.optimal_range), "Same seed same range");
}

static void testTurretSizeScaling() {
    std::cout << "\n=== Turret Size Scaling ===" << std::endl;
    pcg::TurretGenerator gen;
    auto small = gen.generate(100, pcg::TurretSize::Small, pcg::TurretType::Projectile, "");
    auto large = gen.generate(100, pcg::TurretSize::Large, pcg::TurretType::Projectile, "");
    assertTrue(large.profile.base_damage > small.profile.base_damage, "Large turret more damage than small");
}


void run_turret_generator_tests() {
    testTurretGeneration();
    testTurretDeterminism();
    testTurretSizeScaling();
}
