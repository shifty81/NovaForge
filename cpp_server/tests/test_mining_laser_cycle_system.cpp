// Tests for: MiningLaserCycle System Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/mining_laser_cycle_system.h"

using namespace atlas;

// ==================== MiningLaserCycle System Tests ====================

static void testMiningLaserCycleCreate() {
    std::cout << "\n=== MiningLaserCycle: Create ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    assertTrue(sys.initializeMining("miner1", "miner1", 5000.0f), "Init mining succeeds");
    assertTrue(sys.getActiveLaserCount("miner1") == 0, "No active lasers initially");
    assertTrue(sys.getTotalCyclesCompleted("miner1") == 0, "0 cycles completed");
    assertTrue(approxEqual(sys.getTotalOreMined("miner1"), 0.0f), "0 ore mined");
    assertTrue(approxEqual(sys.getCargoUsed("miner1"), 0.0f), "0 cargo used");
    assertTrue(approxEqual(sys.getCargoRemaining("miner1"), 5000.0f), "5000 cargo remaining");
}

static void testMiningLaserCycleStartLaser() {
    std::cout << "\n=== MiningLaserCycle: StartLaser ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    sys.initializeMining("miner1", "miner1", 5000.0f);
    assertTrue(sys.startLaser("miner1", "laser1", "ast1", "Veldspar", 10.0f, 100.0f), "Start laser succeeds");
    assertTrue(sys.getActiveLaserCount("miner1") == 1, "1 active laser");
    assertTrue(approxEqual(sys.getLaserProgress("miner1", "laser1"), 0.0f), "0 progress initially");
}

static void testMiningLaserCycleDuplicate() {
    std::cout << "\n=== MiningLaserCycle: Duplicate ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    sys.initializeMining("miner1", "miner1", 5000.0f);
    sys.startLaser("miner1", "laser1", "ast1", "Veldspar", 10.0f, 100.0f);
    assertTrue(!sys.startLaser("miner1", "laser1", "ast2", "Scordite", 10.0f, 50.0f), "Duplicate laser rejected");
    assertTrue(sys.getActiveLaserCount("miner1") == 1, "Still 1 laser");
}

static void testMiningLaserCycleProgress() {
    std::cout << "\n=== MiningLaserCycle: Progress ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    sys.initializeMining("miner1", "miner1", 5000.0f);
    sys.startLaser("miner1", "laser1", "ast1", "Veldspar", 10.0f, 100.0f);
    sys.update(5.0f);  // 50% progress
    assertTrue(approxEqual(sys.getLaserProgress("miner1", "laser1"), 0.5f), "50% progress after 5s");
}

static void testMiningLaserCycleComplete() {
    std::cout << "\n=== MiningLaserCycle: Complete ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    sys.initializeMining("miner1", "miner1", 5000.0f);
    sys.startLaser("miner1", "laser1", "ast1", "Veldspar", 10.0f, 100.0f);
    sys.update(10.0f);  // cycle completes
    assertTrue(sys.getTotalCyclesCompleted("miner1") == 1, "1 cycle completed");
    assertTrue(approxEqual(sys.getTotalOreMined("miner1"), 100.0f), "100 ore mined");
    assertTrue(approxEqual(sys.getCargoUsed("miner1"), 100.0f), "100 cargo used");
    assertTrue(sys.getActiveLaserCount("miner1") == 0, "Laser removed after completion");
}

static void testMiningLaserCycleMultipleLasers() {
    std::cout << "\n=== MiningLaserCycle: MultipleLasers ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    sys.initializeMining("miner1", "miner1", 5000.0f);
    sys.startLaser("miner1", "laser1", "ast1", "Veldspar", 10.0f, 100.0f);
    sys.startLaser("miner1", "laser2", "ast2", "Scordite", 10.0f, 80.0f);
    assertTrue(sys.getActiveLaserCount("miner1") == 2, "2 active lasers");
    sys.update(10.0f);
    assertTrue(sys.getTotalCyclesCompleted("miner1") == 2, "2 cycles completed");
    assertTrue(approxEqual(sys.getTotalOreMined("miner1"), 180.0f), "180 total ore");
}

static void testMiningLaserCycleStop() {
    std::cout << "\n=== MiningLaserCycle: Stop ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    sys.initializeMining("miner1", "miner1", 5000.0f);
    sys.startLaser("miner1", "laser1", "ast1", "Veldspar", 10.0f, 100.0f);
    sys.update(3.0f);  // partial progress
    assertTrue(sys.stopLaser("miner1", "laser1"), "Stop laser succeeds");
    assertTrue(sys.getActiveLaserCount("miner1") == 0, "0 lasers after stop");
    assertTrue(sys.getTotalCyclesCompleted("miner1") == 0, "0 completed (stopped mid-cycle)");
}

static void testMiningLaserCycleCargoFull() {
    std::cout << "\n=== MiningLaserCycle: CargoFull ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    sys.initializeMining("miner1", "miner1", 50.0f);  // small cargo
    sys.startLaser("miner1", "laser1", "ast1", "Veldspar", 5.0f, 50.0f);
    sys.update(5.0f);  // fills cargo
    assertTrue(sys.isCargoFull("miner1"), "Cargo is full");
    assertTrue(!sys.startLaser("miner1", "laser2", "ast2", "Scordite", 5.0f, 50.0f), "Cannot start laser with full cargo");
}

static void testMiningLaserCycleMaxLasers() {
    std::cout << "\n=== MiningLaserCycle: MaxLasers ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    world.createEntity("miner1");
    sys.initializeMining("miner1", "miner1", 50000.0f);
    auto* entity = world.getEntity("miner1");
    auto* mlc = entity->getComponent<components::MiningLaserCycle>();
    mlc->max_active_lasers = 2;

    sys.startLaser("miner1", "l1", "a1", "Veldspar", 10.0f, 100.0f);
    sys.startLaser("miner1", "l2", "a2", "Scordite", 10.0f, 100.0f);
    assertTrue(!sys.startLaser("miner1", "l3", "a3", "Pyroxeres", 10.0f, 100.0f), "Max lasers enforced");
    assertTrue(sys.getActiveLaserCount("miner1") == 2, "Still 2 lasers");
}

static void testMiningLaserCycleMissing() {
    std::cout << "\n=== MiningLaserCycle: Missing ===" << std::endl;
    ecs::World world;
    systems::MiningLaserCycleSystem sys(&world);
    assertTrue(!sys.initializeMining("nonexistent", "c1", 5000.0f), "Init fails on missing");
    assertTrue(!sys.startLaser("nonexistent", "l1", "a1", "Ore", 10.0f, 100.0f), "Start fails on missing");
    assertTrue(!sys.stopLaser("nonexistent", "l1"), "Stop fails on missing");
    assertTrue(sys.getActiveLaserCount("nonexistent") == 0, "0 lasers on missing");
    assertTrue(approxEqual(sys.getLaserProgress("nonexistent", "l1"), 0.0f), "0 progress on missing");
    assertTrue(sys.getTotalCyclesCompleted("nonexistent") == 0, "0 cycles on missing");
    assertTrue(approxEqual(sys.getTotalOreMined("nonexistent"), 0.0f), "0 ore on missing");
    assertTrue(approxEqual(sys.getCargoUsed("nonexistent"), 0.0f), "0 cargo on missing");
    assertTrue(approxEqual(sys.getCargoRemaining("nonexistent"), 0.0f), "0 remaining on missing");
    assertTrue(!sys.isCargoFull("nonexistent"), "Not full on missing");
}

void run_mining_laser_cycle_system_tests() {
    testMiningLaserCycleCreate();
    testMiningLaserCycleStartLaser();
    testMiningLaserCycleDuplicate();
    testMiningLaserCycleProgress();
    testMiningLaserCycleComplete();
    testMiningLaserCycleMultipleLasers();
    testMiningLaserCycleStop();
    testMiningLaserCycleCargoFull();
    testMiningLaserCycleMaxLasers();
    testMiningLaserCycleMissing();
}
