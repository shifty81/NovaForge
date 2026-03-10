// Tests for: SensorDampeningSystem
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/sensor_dampening_system.h"

using namespace atlas;

// ==================== SensorDampeningSystem Tests ====================

static void testSensorDampeningInit() {
    std::cout << "\n=== SensorDampening: Init ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 80.0f, 300.0f), "Init succeeds");
    assertTrue(sys.getDampenerCount("ship1") == 0, "Zero dampeners initially");
    assertTrue(!sys.isDampened("ship1"), "Not dampened initially");
    assertTrue(approxEqual(sys.getBaseLockRange("ship1"), 80.0f), "Base lock range set");
    assertTrue(approxEqual(sys.getBaseScanResolution("ship1"), 300.0f), "Base scan res set");
    assertTrue(approxEqual(sys.getEffectiveLockRange("ship1"), 80.0f), "Effective == base initially");
    assertTrue(approxEqual(sys.getEffectiveScanResolution("ship1"), 300.0f), "Effective scan res == base");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testSensorDampeningApplyDampener() {
    std::cout << "\n=== SensorDampening: ApplyDampener ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);

    // 25% range reduction, 20% scan res reduction
    assertTrue(sys.applyDampener("ship1", "enemy1", 0.25f, 0.20f, 5.0f),
               "Apply dampener succeeds");
    assertTrue(sys.getDampenerCount("ship1") == 1, "1 dampener after apply");
    assertTrue(sys.isDampened("ship1"), "Dampened after apply");

    // Effective lock range: 100 * (1 - 0.25) = 75
    assertTrue(approxEqual(sys.getEffectiveLockRange("ship1"), 75.0f),
               "Lock range reduced to 75");
    // Effective scan res: 400 * (1 - 0.20) = 320
    assertTrue(approxEqual(sys.getEffectiveScanResolution("ship1"), 320.0f),
               "Scan res reduced to 320");
    assertTrue(sys.getTotalDampenersApplied("ship1") == 1, "1 total dampener applied");
}

static void testSensorDampeningMultipleDampeners() {
    std::cout << "\n=== SensorDampening: MultipleDampeners ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);

    sys.applyDampener("ship1", "e1", 0.25f, 0.20f, 5.0f);
    sys.applyDampener("ship1", "e2", 0.20f, 0.15f, 5.0f);

    assertTrue(sys.getDampenerCount("ship1") == 2, "2 dampeners");
    // Multiplicative: 100 * (1-0.25) * (1-0.20) = 100 * 0.75 * 0.80 = 60
    assertTrue(approxEqual(sys.getEffectiveLockRange("ship1"), 60.0f),
               "Lock range multiplicatively reduced to 60");
    // 400 * (1-0.20) * (1-0.15) = 400 * 0.80 * 0.85 = 272
    assertTrue(approxEqual(sys.getEffectiveScanResolution("ship1"), 272.0f),
               "Scan res reduced to 272");
}

static void testSensorDampeningDuplicate() {
    std::cout << "\n=== SensorDampening: Duplicate ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);

    sys.applyDampener("ship1", "enemy1", 0.25f, 0.20f, 5.0f);
    assertTrue(!sys.applyDampener("ship1", "enemy1", 0.10f, 0.10f, 5.0f),
               "Duplicate source rejected");
    assertTrue(sys.getDampenerCount("ship1") == 1, "Still 1 dampener after duplicate");
}

static void testSensorDampeningValidation() {
    std::cout << "\n=== SensorDampening: Validation ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);

    assertTrue(!sys.applyDampener("ship1", "",      0.25f, 0.20f, 5.0f), "Empty source rejected");
    assertTrue(!sys.applyDampener("ship1", "e1", -0.1f,  0.20f, 5.0f),  "Negative range red rejected");
    assertTrue(!sys.applyDampener("ship1", "e1",  1.0f,  0.20f, 5.0f),  "Range red >= 1 rejected");
    assertTrue(!sys.applyDampener("ship1", "e1",  0.25f, -0.1f, 5.0f),  "Negative scan red rejected");
    assertTrue(!sys.applyDampener("ship1", "e1",  0.25f,  1.0f, 5.0f),  "Scan red >= 1 rejected");
    assertTrue(!sys.applyDampener("ship1", "e1",  0.25f, 0.20f, 0.0f),  "Zero cycle time rejected");
    assertTrue(sys.getDampenerCount("ship1") == 0, "No dampeners after all failed validates");
}

static void testSensorDampeningRemoveDampener() {
    std::cout << "\n=== SensorDampening: RemoveDampener ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);
    sys.applyDampener("ship1", "e1", 0.25f, 0.20f, 5.0f);
    sys.applyDampener("ship1", "e2", 0.20f, 0.15f, 5.0f);

    assertTrue(sys.removeDampener("ship1", "e1"), "Remove first dampener succeeds");
    assertTrue(sys.getDampenerCount("ship1") == 1, "1 dampener remaining");
    // After removing e1: 100*(1-0.20)=80, 400*(1-0.15)=340
    assertTrue(approxEqual(sys.getEffectiveLockRange("ship1"), 80.0f),
               "Lock range recalculated to 80 after remove");
    assertTrue(approxEqual(sys.getEffectiveScanResolution("ship1"), 340.0f),
               "Scan res recalculated to 340 after remove");

    assertTrue(!sys.removeDampener("ship1", "nonexistent"), "Remove nonexistent fails");
}

static void testSensorDampeningClearDampeners() {
    std::cout << "\n=== SensorDampening: ClearDampeners ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);
    sys.applyDampener("ship1", "e1", 0.25f, 0.20f, 5.0f);
    sys.applyDampener("ship1", "e2", 0.10f, 0.10f, 5.0f);

    assertTrue(sys.clearDampeners("ship1"), "Clear succeeds");
    assertTrue(sys.getDampenerCount("ship1") == 0, "0 dampeners after clear");
    assertTrue(!sys.isDampened("ship1"), "Not dampened after clear");
    assertTrue(approxEqual(sys.getEffectiveLockRange("ship1"), 100.0f),
               "Lock range restored to base after clear");
    assertTrue(approxEqual(sys.getEffectiveScanResolution("ship1"), 400.0f),
               "Scan res restored to base after clear");
}

static void testSensorDampeningSetBaseValues() {
    std::cout << "\n=== SensorDampening: SetBaseValues ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);
    sys.applyDampener("ship1", "e1", 0.25f, 0.20f, 5.0f);

    assertTrue(sys.setBaseLockRange("ship1", 120.0f), "Set base lock range succeeds");
    assertTrue(approxEqual(sys.getBaseLockRange("ship1"), 120.0f), "Base range updated");
    assertTrue(approxEqual(sys.getEffectiveLockRange("ship1"), 90.0f),
               "Effective range recalculated: 120*(1-0.25)=90");

    assertTrue(sys.setBaseScanResolution("ship1", 500.0f), "Set base scan res succeeds");
    assertTrue(approxEqual(sys.getBaseScanResolution("ship1"), 500.0f), "Base scan res updated");
    assertTrue(approxEqual(sys.getEffectiveScanResolution("ship1"), 400.0f),
               "Effective scan res: 500*(1-0.20)=400");

    assertTrue(!sys.setBaseLockRange("ship1", 0.0f),  "Zero range rejected");
    assertTrue(!sys.setBaseLockRange("ship1", -10.0f),"Negative range rejected");
    assertTrue(!sys.setBaseScanResolution("ship1", 0.0f), "Zero scan res rejected");
}

static void testSensorDampeningCycleTimer() {
    std::cout << "\n=== SensorDampening: CycleTimer ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);
    sys.applyDampener("ship1", "e1", 0.25f, 0.20f, 3.0f);

    sys.update(3.1f);
    assertTrue(sys.getTotalDampenerCycles("ship1") == 1, "1 cycle completed after 3.1s");

    sys.update(3.1f);
    assertTrue(sys.getTotalDampenerCycles("ship1") == 2, "2 cycles after second tick");
}

static void testSensorDampeningMaxDampeners() {
    std::cout << "\n=== SensorDampening: MaxDampeners ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 400.0f);

    auto* comp = world.getEntity("ship1")->getComponent<components::SensorDampeningState>();
    comp->max_dampeners = 2;

    assertTrue(sys.applyDampener("ship1", "e1", 0.1f, 0.1f, 5.0f), "First dampener applied");
    assertTrue(sys.applyDampener("ship1", "e2", 0.1f, 0.1f, 5.0f), "Second dampener applied");
    assertTrue(!sys.applyDampener("ship1", "e3", 0.1f, 0.1f, 5.0f),"Third dampener rejected at max");
    assertTrue(sys.getDampenerCount("ship1") == 2, "Dampener count capped at 2");
}

static void testSensorDampeningMissing() {
    std::cout << "\n=== SensorDampening: Missing ===" << std::endl;
    ecs::World world;
    systems::SensorDampeningSystem sys(&world);

    assertTrue(!sys.applyDampener("nx", "e1", 0.25f, 0.20f, 5.0f), "ApplyDampener fails on missing");
    assertTrue(!sys.removeDampener("nx", "e1"),                      "RemoveDampener fails on missing");
    assertTrue(!sys.clearDampeners("nx"),                            "ClearDampeners fails on missing");
    assertTrue(!sys.setBaseLockRange("nx", 100.0f),                  "SetBaseLockRange fails on missing");
    assertTrue(!sys.setBaseScanResolution("nx", 400.0f),             "SetBaseScanRes fails on missing");
    assertTrue(!sys.isDampened("nx"),                                "isDampened false on missing");
    assertTrue(sys.getDampenerCount("nx") == 0,                      "0 dampeners on missing");
    assertTrue(approxEqual(sys.getEffectiveLockRange("nx"), 0.0f),   "0 lock range on missing");
    assertTrue(approxEqual(sys.getEffectiveScanResolution("nx"), 0.0f), "0 scan res on missing");
    assertTrue(sys.getTotalDampenersApplied("nx") == 0,              "0 total applied on missing");
}

void run_sensor_dampening_system_tests() {
    testSensorDampeningInit();
    testSensorDampeningApplyDampener();
    testSensorDampeningMultipleDampeners();
    testSensorDampeningDuplicate();
    testSensorDampeningValidation();
    testSensorDampeningRemoveDampener();
    testSensorDampeningClearDampeners();
    testSensorDampeningSetBaseValues();
    testSensorDampeningCycleTimer();
    testSensorDampeningMaxDampeners();
    testSensorDampeningMissing();
}
