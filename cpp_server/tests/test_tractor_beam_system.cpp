// Tests for: Tractor Beam System
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/tractor_beam_system.h"

using namespace atlas;

// ==================== Tractor Beam System Tests ====================

static void testTractorBeamCreate() {
    std::cout << "\n=== TractorBeam: Create ===" << std::endl;
    ecs::World world;
    systems::TractorBeamSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(!sys.isLocked("ship1"), "Not locked");
    assertTrue(sys.getItemsCollected("ship1") == 0, "0 collected");
    assertTrue(sys.getItemsFailed("ship1") == 0, "0 failed");
    assertTrue(sys.getTargetId("ship1").empty(), "No target");
}

static void testTractorBeamLockAndPull() {
    std::cout << "\n=== TractorBeam: LockAndPull ===" << std::endl;
    ecs::World world;
    systems::TractorBeamSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.lockTarget("ship1", "wreck_01", 5000.0f), "Lock target");
    assertTrue(sys.isLocked("ship1"), "Is locked");
    assertTrue(sys.getTargetId("ship1") == "wreck_01", "Target ID matches");

    // Can't lock a second target while one is active
    assertTrue(!sys.lockTarget("ship1", "wreck_02", 3000.0f), "Double lock rejected");

    // Pull the target — speed 500 m/s, 5000m distance
    // After 9 seconds at 500m/s = 4500m pulled → 500m remaining
    for (int i = 0; i < 90; i++) sys.update(0.1f);
    assertTrue(sys.isLocked("ship1"), "Still locked (not collected yet)");
    assertTrue(sys.getCurrentDistance("ship1") > 0.0f, "Distance > 0");

    // After 1 more second → fully pulled
    for (int i = 0; i < 10; i++) sys.update(0.1f);
    assertTrue(!sys.isLocked("ship1"), "Unlocked after collection");
    assertTrue(sys.getItemsCollected("ship1") == 1, "1 collected");
}

static void testTractorBeamOutOfRange() {
    std::cout << "\n=== TractorBeam: OutOfRange ===" << std::endl;
    ecs::World world;
    systems::TractorBeamSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Default range is 20000m — 25000m should fail
    assertTrue(!sys.lockTarget("ship1", "wreck_far", 25000.0f), "Out of range rejected");
    assertTrue(sys.getItemsFailed("ship1") == 1, "1 failed");
    assertTrue(!sys.isLocked("ship1"), "Not locked");
}

static void testTractorBeamUnlock() {
    std::cout << "\n=== TractorBeam: Unlock ===" << std::endl;
    ecs::World world;
    systems::TractorBeamSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.lockTarget("ship1", "wreck_01", 3000.0f);
    assertTrue(sys.unlockTarget("ship1"), "Manual unlock");
    assertTrue(!sys.isLocked("ship1"), "Not locked after unlock");
    assertTrue(sys.getItemsCollected("ship1") == 0, "0 collected (cancelled)");
}

static void testTractorBeamConfig() {
    std::cout << "\n=== TractorBeam: Config ===" << std::endl;
    ecs::World world;
    systems::TractorBeamSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.setRange("ship1", 30000.0f), "Set range");
    assertTrue(sys.setPullSpeed("ship1", 1000.0f), "Set pull speed");

    // Negative values rejected
    assertTrue(!sys.setRange("ship1", -1.0f), "Negative range rejected");
    assertTrue(!sys.setPullSpeed("ship1", 0.0f), "Zero speed rejected");

    // New range allows locking at 25000m
    assertTrue(sys.lockTarget("ship1", "wreck_far", 25000.0f), "Lock at 25000m with 30000m range");
}

static void testTractorBeamMissing() {
    std::cout << "\n=== TractorBeam: Missing ===" << std::endl;
    ecs::World world;
    systems::TractorBeamSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.lockTarget("nonexistent", "t", 100.0f), "Lock fails on missing");
    assertTrue(!sys.unlockTarget("nonexistent"), "Unlock fails on missing");
    assertTrue(!sys.setRange("nonexistent", 1000.0f), "SetRange fails on missing");
    assertTrue(!sys.setPullSpeed("nonexistent", 500.0f), "SetPullSpeed fails on missing");
    assertTrue(!sys.isLocked("nonexistent"), "Not locked on missing");
    assertTrue(sys.getCurrentDistance("nonexistent") == 0.0f, "0 distance on missing");
    assertTrue(sys.getItemsCollected("nonexistent") == 0, "0 collected on missing");
    assertTrue(sys.getItemsFailed("nonexistent") == 0, "0 failed on missing");
    assertTrue(sys.getTargetId("nonexistent").empty(), "Empty target on missing");
}

void run_tractor_beam_system_tests() {
    testTractorBeamCreate();
    testTractorBeamLockAndPull();
    testTractorBeamOutOfRange();
    testTractorBeamUnlock();
    testTractorBeamConfig();
    testTractorBeamMissing();
}
