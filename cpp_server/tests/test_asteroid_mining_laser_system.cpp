// Tests for: Asteroid Mining Laser System
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/asteroid_mining_laser_system.h"

using namespace atlas;

// ==================== Asteroid Mining Laser System Tests ====================

static void testMiningLaserCreate() {
    std::cout << "\n=== MiningLaser: Create ===" << std::endl;
    ecs::World world;
    systems::AsteroidMiningLaserSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getLaserCount("ship1") == 0, "0 lasers");
    assertTrue(sys.getTotalCyclesCompleted("ship1") == 0, "0 cycles");
    assertTrue(sys.getOreHoldCurrent("ship1") == 0.0, "0 ore in hold");
    assertTrue(!sys.isHoldFull("ship1"), "Hold not full");
}

static void testMiningLaserAddAndDuplicate() {
    std::cout << "\n=== MiningLaser: AddAndDuplicate ===" << std::endl;
    ecs::World world;
    systems::AsteroidMiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.addLaser("ship1", "laser_01", 10.0f, 60.0f), "Add laser_01");
    assertTrue(sys.addLaser("ship1", "laser_02", 10.0f, 60.0f), "Add laser_02");
    assertTrue(sys.getLaserCount("ship1") == 2, "2 lasers");

    // Duplicate rejected
    assertTrue(!sys.addLaser("ship1", "laser_01", 5.0f, 30.0f), "Duplicate rejected");

    // Fill to max (3)
    assertTrue(sys.addLaser("ship1", "laser_03", 10.0f, 60.0f), "Add laser_03");
    assertTrue(!sys.addLaser("ship1", "laser_04", 10.0f, 60.0f), "4th laser rejected");
    assertTrue(sys.getLaserCount("ship1") == 3, "3 lasers at max");
}

static void testMiningLaserCycleYield() {
    std::cout << "\n=== MiningLaser: CycleYield ===" << std::endl;
    ecs::World world;
    systems::AsteroidMiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addLaser("ship1", "laser_01", 10.0f, 10.0f);  // 10 m3 per 10s cycle
    sys.setTarget("ship1", "asteroid_42");
    assertTrue(sys.startCycle("ship1", "laser_01"), "Start cycle");
    assertTrue(sys.isCycling("ship1", "laser_01"), "Is cycling");

    // Tick 10 seconds — one full cycle
    sys.update(10.0f);
    assertTrue(sys.getTotalCyclesCompleted("ship1") == 1, "1 cycle completed");
    assertTrue(approxEqual(sys.getOreHoldCurrent("ship1"), 10.0), "10 m3 ore in hold");
    assertTrue(approxEqual(sys.getTotalOreMined("ship1"), 10.0), "10 m3 total mined");
}

static void testMiningLaserCrystalBonus() {
    std::cout << "\n=== MiningLaser: CrystalBonus ===" << std::endl;
    ecs::World world;
    systems::AsteroidMiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addLaser("ship1", "laser_01", 10.0f, 10.0f);
    sys.loadCrystal("ship1", "laser_01", "veldspar_t2", 0.75f);  // +75% bonus
    sys.setTarget("ship1", "asteroid_42");
    sys.startCycle("ship1", "laser_01");

    // 1 cycle: 10 * (1 + 0.75) = 17.5 m3
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getOreHoldCurrent("ship1"), 17.5), "17.5 m3 with crystal");
}

static void testMiningLaserHoldFull() {
    std::cout << "\n=== MiningLaser: HoldFull ===" << std::endl;
    ecs::World world;
    systems::AsteroidMiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Set tiny hold: 15 m3
    auto* comp = world.getEntity("ship1")->getComponent<components::AsteroidMiningLaser>();
    comp->ore_hold_capacity = 15.0;

    sys.addLaser("ship1", "laser_01", 10.0f, 10.0f);
    sys.setTarget("ship1", "asteroid_42");
    sys.startCycle("ship1", "laser_01");

    // 1 cycle yields 10 → hold = 10
    sys.update(10.0f);
    assertTrue(!sys.isHoldFull("ship1"), "Not full after 1 cycle");

    // 2nd cycle yields 10, but only 5 fits → hold = 15
    sys.update(10.0f);
    assertTrue(sys.isHoldFull("ship1"), "Hold full after 2 cycles");
    assertTrue(approxEqual(sys.getOreHoldCurrent("ship1"), 15.0), "Capped at capacity");

    // Can't start another cycle when full
    assertTrue(!sys.startCycle("ship1", "laser_01"), "Start rejected when full");
}

static void testMiningLaserNoTarget() {
    std::cout << "\n=== MiningLaser: NoTarget ===" << std::endl;
    ecs::World world;
    systems::AsteroidMiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addLaser("ship1", "laser_01", 10.0f, 10.0f);
    // No target set — can't start
    assertTrue(!sys.startCycle("ship1", "laser_01"), "Start rejected with no target");
}

static void testMiningLaserStopCycle() {
    std::cout << "\n=== MiningLaser: StopCycle ===" << std::endl;
    ecs::World world;
    systems::AsteroidMiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addLaser("ship1", "laser_01", 10.0f, 10.0f);
    sys.setTarget("ship1", "asteroid_42");
    sys.startCycle("ship1", "laser_01");
    assertTrue(sys.stopCycle("ship1", "laser_01"), "Stop cycle");
    assertTrue(!sys.isCycling("ship1", "laser_01"), "No longer cycling");
    assertTrue(!sys.stopCycle("ship1", "laser_01"), "Can't stop already stopped");
}

static void testMiningLaserMissing() {
    std::cout << "\n=== MiningLaser: Missing ===" << std::endl;
    ecs::World world;
    systems::AsteroidMiningLaserSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addLaser("nonexistent", "l1", 10.0f, 10.0f), "AddLaser fails on missing");
    assertTrue(!sys.startCycle("nonexistent", "l1"), "StartCycle fails on missing");
    assertTrue(!sys.stopCycle("nonexistent", "l1"), "StopCycle fails on missing");
    assertTrue(!sys.setTarget("nonexistent", "a1"), "SetTarget fails on missing");
    assertTrue(!sys.loadCrystal("nonexistent", "l1", "c1", 0.5f), "LoadCrystal fails on missing");
    assertTrue(sys.getLaserCount("nonexistent") == 0, "0 lasers on missing");
    assertTrue(!sys.isCycling("nonexistent", "l1"), "Not cycling on missing");
    assertTrue(sys.getOreHoldCurrent("nonexistent") == 0.0, "0 ore on missing");
    assertTrue(sys.getTotalOreMined("nonexistent") == 0.0, "0 total mined on missing");
    assertTrue(sys.getTotalCyclesCompleted("nonexistent") == 0, "0 cycles on missing");
    assertTrue(!sys.isHoldFull("nonexistent"), "Not full on missing");
}

void run_asteroid_mining_laser_system_tests() {
    testMiningLaserCreate();
    testMiningLaserAddAndDuplicate();
    testMiningLaserCycleYield();
    testMiningLaserCrystalBonus();
    testMiningLaserHoldFull();
    testMiningLaserNoTarget();
    testMiningLaserStopCycle();
    testMiningLaserMissing();
}
