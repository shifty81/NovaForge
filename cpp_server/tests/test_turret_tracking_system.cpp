// Tests for: Turret Tracking System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/turret_tracking_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Turret Tracking System Tests ====================

static void testTurretTrackingCreate() {
    std::cout << "\n=== TurretTracking: Create ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    assertTrue(sys.initialize("tt1", "turret_alpha"), "Init succeeds");
    assertTrue(!sys.isLocked("tt1"), "Not locked initially");
    assertTrue(sys.getTotalShots("tt1") == 0, "0 shots initially");
    assertTrue(sys.getTotalHits("tt1") == 0, "0 hits initially");
    assertTrue(approxEqual(sys.getAccuracy("tt1"), 0.0f), "0 accuracy without lock");
    assertTrue(sys.getTargetId("tt1").empty(), "No target initially");
}

static void testTurretTrackingInitValidation() {
    std::cout << "\n=== TurretTracking: InitValidation ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    assertTrue(!sys.initialize("tt1", ""), "Empty turret_id rejected");
    assertTrue(!sys.initialize("nonexistent", "turret_a"), "Missing entity rejected");
}

static void testTurretTrackingLock() {
    std::cout << "\n=== TurretTracking: Lock ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    sys.initialize("tt1", "turret_a");
    assertTrue(sys.lockTarget("tt1", "enemy_1"), "Lock target");
    assertTrue(sys.isLocked("tt1"), "Now locked");
    assertTrue(sys.getTargetId("tt1") == "enemy_1", "Target is enemy_1");
    assertTrue(!sys.lockTarget("tt1", ""), "Empty target rejected");
}

static void testTurretTrackingUnlock() {
    std::cout << "\n=== TurretTracking: Unlock ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    sys.initialize("tt1", "turret_a");
    assertTrue(!sys.unlockTarget("tt1"), "Cannot unlock when not locked");
    sys.lockTarget("tt1", "enemy_1");
    assertTrue(sys.unlockTarget("tt1"), "Unlock");
    assertTrue(!sys.isLocked("tt1"), "No longer locked");
    assertTrue(sys.getTargetId("tt1").empty(), "Target cleared");
    assertTrue(approxEqual(sys.getAccuracy("tt1"), 0.0f), "Accuracy reset");
}

static void testTurretTrackingAccuracy() {
    std::cout << "\n=== TurretTracking: Accuracy ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    sys.initialize("tt1", "turret_a");
    sys.setTrackingSpeed("tt1", 2.0f);
    sys.lockTarget("tt1", "enemy_1");
    sys.setTargetAngularVelocity("tt1", 0.0f); // stationary target
    sys.update(1.0f);
    // Accuracy = 2.0 / (2.0 + 0.0) = 1.0
    assertTrue(approxEqual(sys.getAccuracy("tt1"), 1.0f), "Perfect accuracy on stationary target");
    sys.setTargetAngularVelocity("tt1", 2.0f); // fast target
    sys.update(1.0f);
    // Accuracy = 2.0 / (2.0 + 2.0) = 0.5
    assertTrue(approxEqual(sys.getAccuracy("tt1"), 0.5f), "50% accuracy on fast target");
}

static void testTurretTrackingFireShot() {
    std::cout << "\n=== TurretTracking: FireShot ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    sys.initialize("tt1", "turret_a");
    assertTrue(!sys.fireShot("tt1"), "Cannot fire without lock");
    sys.lockTarget("tt1", "enemy_1");
    sys.setTrackingSpeed("tt1", 2.0f);
    sys.setTargetAngularVelocity("tt1", 0.0f);
    sys.update(1.0f);
    assertTrue(sys.fireShot("tt1"), "Fire shot");
    assertTrue(sys.getTotalShots("tt1") == 1, "1 shot fired");
    assertTrue(sys.getTotalHits("tt1") == 1, "1 hit (accuracy >= 0.5)");
}

static void testTurretTrackingHitRate() {
    std::cout << "\n=== TurretTracking: HitRate ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    sys.initialize("tt1", "turret_a");
    assertTrue(approxEqual(sys.getHitRate("tt1"), 0.0f), "0% hit rate initially");
    sys.lockTarget("tt1", "enemy_1");
    sys.setTrackingSpeed("tt1", 2.0f);
    sys.setTargetAngularVelocity("tt1", 0.0f);
    sys.update(1.0f);
    sys.fireShot("tt1");
    sys.fireShot("tt1");
    assertTrue(approxEqual(sys.getHitRate("tt1"), 1.0f), "100% hit rate");
}

static void testTurretTrackingSpeedClamp() {
    std::cout << "\n=== TurretTracking: SpeedClamp ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    sys.initialize("tt1", "turret_a");
    sys.setTrackingSpeed("tt1", 0.001f);
    sys.lockTarget("tt1", "enemy_1");
    sys.setTargetAngularVelocity("tt1", 0.0f);
    sys.update(1.0f);
    // tracking_speed clamped to 0.01 minimum
    assertTrue(sys.getAccuracy("tt1") > 0.0f, "Non-zero accuracy with clamped speed");
    sys.setOptimalRange("tt1", 0.5f);
    sys.setFalloffRange("tt1", -100.0f);
    // Ranges clamped to their min values
}

static void testTurretTrackingDamageMultiplier() {
    std::cout << "\n=== TurretTracking: DamageMultiplier ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    world.createEntity("tt1");
    sys.initialize("tt1", "turret_a");
    assertTrue(approxEqual(sys.getDamageMultiplier("tt1"), 0.0f), "0 damage mult without lock");
    sys.lockTarget("tt1", "enemy_1");
    sys.setTrackingSpeed("tt1", 2.0f);
    sys.setTargetAngularVelocity("tt1", 0.0f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getDamageMultiplier("tt1"), 1.0f), "Full damage on stationary");
}

static void testTurretTrackingMissing() {
    std::cout << "\n=== TurretTracking: Missing ===" << std::endl;
    ecs::World world;
    systems::TurretTrackingSystem sys(&world);
    assertTrue(!sys.lockTarget("nonexistent", "e1"), "Lock fails on missing");
    assertTrue(!sys.unlockTarget("nonexistent"), "Unlock fails on missing");
    assertTrue(!sys.setTrackingSpeed("nonexistent", 1.0f), "Speed fails on missing");
    assertTrue(!sys.setOptimalRange("nonexistent", 1000.0f), "Range fails on missing");
    assertTrue(!sys.setFalloffRange("nonexistent", 500.0f), "Falloff fails on missing");
    assertTrue(!sys.setTargetAngularVelocity("nonexistent", 1.0f), "Angular vel fails on missing");
    assertTrue(!sys.fireShot("nonexistent"), "Fire fails on missing");
    assertTrue(approxEqual(sys.getAccuracy("nonexistent"), 0.0f), "0 accuracy on missing");
    assertTrue(approxEqual(sys.getDamageMultiplier("nonexistent"), 0.0f), "0 damage on missing");
    assertTrue(approxEqual(sys.getHitRate("nonexistent"), 0.0f), "0 hit rate on missing");
    assertTrue(!sys.isLocked("nonexistent"), "Not locked on missing");
    assertTrue(sys.getTotalShots("nonexistent") == 0, "0 shots on missing");
    assertTrue(sys.getTotalHits("nonexistent") == 0, "0 hits on missing");
    assertTrue(sys.getTargetId("nonexistent").empty(), "Empty target on missing");
}

void run_turret_tracking_system_tests() {
    testTurretTrackingCreate();
    testTurretTrackingInitValidation();
    testTurretTrackingLock();
    testTurretTrackingUnlock();
    testTurretTrackingAccuracy();
    testTurretTrackingFireShot();
    testTurretTrackingHitRate();
    testTurretTrackingSpeedClamp();
    testTurretTrackingDamageMultiplier();
    testTurretTrackingMissing();
}
