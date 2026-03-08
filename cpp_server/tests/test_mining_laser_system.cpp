// Tests for: MiningLaserSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/mining_laser_system.h"

using namespace atlas;

// ==================== MiningLaserSystem Tests ====================

static void testMiningLaserCreate() {
    std::cout << "\n=== MiningLaser: Create ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", "strip", 1.5f), "Init strip miner succeeds");
    assertTrue(approxEqual(sys.getMiningStrength("ship1"), 1.5f), "Strength 1.5");
    assertTrue(!sys.isMiningActive("ship1"), "Not mining initially");
    assertTrue(approxEqual(sys.getTotalOreMined("ship1"), 0.0f), "Zero ore mined");
    assertTrue(sys.getTotalCycles("ship1") == 0, "Zero cycles");
    assertTrue(sys.getFailedCycles("ship1") == 0, "Zero failed cycles");
    assertTrue(sys.getOreTypeCount("ship1") == 0, "Zero ore types");
}

static void testMiningLaserInvalidInit() {
    std::cout << "\n=== MiningLaser: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    assertTrue(!sys.initialize("missing", "strip", 1.0f), "Missing entity fails");
    world.createEntity("ship1");
    assertTrue(!sys.initialize("ship1", "", 1.0f), "Empty laser type fails");
    assertTrue(!sys.initialize("ship1", "strip", 0.0f), "Zero strength fails");
    assertTrue(!sys.initialize("ship1", "strip", -1.0f), "Negative strength fails");
}

static void testMiningLaserStartStop() {
    std::cout << "\n=== MiningLaser: StartStop ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 1.0f);

    assertTrue(sys.startMining("ship1", "asteroid_1"), "Start mining succeeds");
    assertTrue(sys.isMiningActive("ship1"), "Mining active");
    assertTrue(sys.getTargetAsteroid("ship1") == "asteroid_1", "Target set");

    // Can't start twice
    assertTrue(!sys.startMining("ship1", "asteroid_2"), "Double start rejected");

    assertTrue(sys.stopMining("ship1"), "Stop mining succeeds");
    assertTrue(!sys.isMiningActive("ship1"), "Mining stopped");

    // Can't stop when not mining
    assertTrue(!sys.stopMining("ship1"), "Double stop fails");
}

static void testMiningLaserInvalidStart() {
    std::cout << "\n=== MiningLaser: InvalidStart ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 1.0f);

    assertTrue(!sys.startMining("ship1", ""), "Empty target rejected");
    assertTrue(!sys.startMining("nonexistent", "asteroid_1"), "Missing entity rejected");
}

static void testMiningLaserCycleCompletion() {
    std::cout << "\n=== MiningLaser: CycleCompletion ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 1.0f);
    sys.setCycleDuration("ship1", 10.0f);

    sys.startMining("ship1", "asteroid_1");

    // Advance 5s → 50% cycle
    sys.update(5.0f);
    float progress = sys.getCycleProgress("ship1");
    assertTrue(progress > 0.49f && progress < 0.51f, "50% cycle at 5s");
    assertTrue(sys.getTotalCycles("ship1") == 0, "No completed cycles yet");

    // Complete the cycle at 10s
    sys.update(5.0f);
    assertTrue(sys.getTotalCycles("ship1") == 1, "1 completed cycle");
    assertTrue(sys.getTotalOreMined("ship1") > 0.0f, "Some ore mined");
}

static void testMiningLaserMultipleCycles() {
    std::cout << "\n=== MiningLaser: MultipleCycles ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 2.0f);
    sys.setCycleDuration("ship1", 5.0f);

    sys.startMining("ship1", "asteroid_1");
    sys.update(15.0f); // 15s / 5s per cycle = 3 cycles
    assertTrue(sys.getTotalCycles("ship1") == 3, "3 cycles after 15s");

    float oreMined = sys.getTotalOreMined("ship1");
    // Each cycle: strength(2) * 10 base = 20 units, 3 cycles = 60 total
    assertTrue(oreMined > 59.0f && oreMined < 61.0f, "~60 ore mined");
}

static void testMiningLaserFailedCycle() {
    std::cout << "\n=== MiningLaser: FailedCycle ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 1.0f);
    sys.setCycleDuration("ship1", 10.0f);

    sys.startMining("ship1", "asteroid_1");
    sys.update(5.0f); // 50% through cycle
    sys.stopMining("ship1"); // Interrupt → failed cycle

    assertTrue(sys.getFailedCycles("ship1") == 1, "1 failed cycle");
    assertTrue(sys.getTotalCycles("ship1") == 0, "0 completed cycles");
}

static void testMiningLaserSetRange() {
    std::cout << "\n=== MiningLaser: SetRange ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 1.0f);

    assertTrue(sys.setRange("ship1", 20.0f, 15.0f), "Set range succeeds");
    assertTrue(!sys.setRange("ship1", 10.0f, 15.0f), "Optimal > range rejected");
    assertTrue(!sys.setRange("ship1", 0.0f, 0.0f), "Zero range rejected");
    assertTrue(!sys.setRange("nonexistent", 20.0f, 15.0f), "Missing entity rejected");
}

static void testMiningLaserOreYield() {
    std::cout << "\n=== MiningLaser: OreYield ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 1.0f);

    assertTrue(sys.addOreYield("ship1", "tritanium", 100.0f), "Add tritanium yield");
    assertTrue(sys.addOreYield("ship1", "pyerite", 50.0f), "Add pyerite yield");
    assertTrue(sys.getOreTypeCount("ship1") == 2, "2 ore types");

    assertTrue(approxEqual(sys.getOreYield("ship1", "tritanium"), 100.0f), "Tritanium 100");
    assertTrue(approxEqual(sys.getOreYield("ship1", "pyerite"), 50.0f), "Pyerite 50");

    // Accumulate
    assertTrue(sys.addOreYield("ship1", "tritanium", 25.0f), "Add more tritanium");
    assertTrue(approxEqual(sys.getOreYield("ship1", "tritanium"), 125.0f), "Tritanium 125");
    assertTrue(sys.getOreTypeCount("ship1") == 2, "Still 2 ore types");

    // Invalid
    assertTrue(!sys.addOreYield("ship1", "", 10.0f), "Empty ore type rejected");
    assertTrue(!sys.addOreYield("ship1", "veldspar", 0.0f), "Zero amount rejected");
    assertTrue(!sys.addOreYield("ship1", "veldspar", -5.0f), "Negative amount rejected");
    assertTrue(approxEqual(sys.getOreYield("ship1", "nonexistent"), 0.0f), "Unknown ore returns 0");
}

static void testMiningLaserAsteroidDepletion() {
    std::cout << "\n=== MiningLaser: AsteroidDepletion ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 1.0f);
    sys.setCycleDuration("ship1", 1.0f); // Fast cycles for testing

    sys.startMining("ship1", "asteroid_1");
    // Each cycle depletes 10*1.0*0.1 = 1.0% of asteroid
    sys.update(10.0f); // 10 cycles → 10% depleted
    float remaining = sys.getAsteroidRemaining("ship1");
    assertTrue(remaining < 100.0f, "Asteroid partially depleted");
    assertTrue(remaining > 0.0f, "Asteroid not fully depleted");
}

static void testMiningLaserUpdate() {
    std::cout << "\n=== MiningLaser: Update ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "strip", 1.0f);
    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testMiningLaserMissing() {
    std::cout << "\n=== MiningLaser: Missing ===" << std::endl;
    ecs::World world;
    systems::MiningLaserSystem sys(&world);
    assertTrue(!sys.isMiningActive("x"), "Default mining on missing");
    assertTrue(approxEqual(sys.getCycleProgress("x"), 0.0f), "Default progress on missing");
    assertTrue(approxEqual(sys.getTotalOreMined("x"), 0.0f), "Default ore on missing");
    assertTrue(sys.getTotalCycles("x") == 0, "Default cycles on missing");
    assertTrue(sys.getFailedCycles("x") == 0, "Default failed on missing");
    assertTrue(approxEqual(sys.getAsteroidRemaining("x"), 0.0f), "Default remaining on missing");
    assertTrue(sys.getTargetAsteroid("x").empty(), "Default target on missing");
    assertTrue(approxEqual(sys.getMiningStrength("x"), 0.0f), "Default strength on missing");
    assertTrue(sys.getOreTypeCount("x") == 0, "Default ore types on missing");
    assertTrue(approxEqual(sys.getOreYield("x", "t"), 0.0f), "Default yield on missing");
}

void run_mining_laser_system_tests() {
    testMiningLaserCreate();
    testMiningLaserInvalidInit();
    testMiningLaserStartStop();
    testMiningLaserInvalidStart();
    testMiningLaserCycleCompletion();
    testMiningLaserMultipleCycles();
    testMiningLaserFailedCycle();
    testMiningLaserSetRange();
    testMiningLaserOreYield();
    testMiningLaserAsteroidDepletion();
    testMiningLaserUpdate();
    testMiningLaserMissing();
}
