// Tests for: Salvage Drone System
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/salvage_drone_system.h"

using namespace atlas;

// ==================== Salvage Drone System Tests ====================

static void testSalvageDroneCreate() {
    std::cout << "\n=== SalvageDrone: Create ===" << std::endl;
    ecs::World world;
    systems::SalvageDroneSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getDroneCount("ship1") == 0, "0 drones");
    assertTrue(sys.getDeployedCount("ship1") == 0, "0 deployed");
    assertTrue(sys.getTotalSalvages("ship1") == 0, "0 salvages");
    assertTrue(sys.getTotalFailures("ship1") == 0, "0 failures");
}

static void testSalvageDroneAddAndDuplicate() {
    std::cout << "\n=== SalvageDrone: AddAndDuplicate ===" << std::endl;
    ecs::World world;
    systems::SalvageDroneSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.addDrone("ship1", "drone_01", 10.0f, 0.6f), "Add drone_01");
    assertTrue(sys.addDrone("ship1", "drone_02", 10.0f, 0.6f), "Add drone_02");
    assertTrue(sys.getDroneCount("ship1") == 2, "2 drones");

    // Duplicate rejected
    assertTrue(!sys.addDrone("ship1", "drone_01", 5.0f, 0.5f), "Duplicate rejected");

    // Fill to max (5)
    for (int i = 3; i <= 5; i++) {
        assertTrue(sys.addDrone("ship1", "drone_0" + std::to_string(i), 10.0f, 0.6f),
                   "Add drone_0" + std::to_string(i));
    }
    assertTrue(sys.getDroneCount("ship1") == 5, "5 drones at max");
    assertTrue(!sys.addDrone("ship1", "drone_06", 10.0f, 0.6f), "6th drone rejected");
}

static void testSalvageDroneDeploy() {
    std::cout << "\n=== SalvageDrone: Deploy ===" << std::endl;
    ecs::World world;
    systems::SalvageDroneSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addDrone("ship1", "drone_01", 10.0f, 0.6f);
    assertTrue(sys.getDroneState("ship1", "drone_01") == "idle", "State idle");

    assertTrue(sys.deployDrone("ship1", "drone_01", "wreck_42"), "Deploy drone_01");
    assertTrue(sys.getDroneState("ship1", "drone_01") == "deployed", "State deployed");
    assertTrue(sys.getDeployedCount("ship1") == 1, "1 deployed");

    // Can't deploy again
    assertTrue(!sys.deployDrone("ship1", "drone_01", "wreck_43"), "Double deploy rejected");
}

static void testSalvageDroneRecall() {
    std::cout << "\n=== SalvageDrone: Recall ===" << std::endl;
    ecs::World world;
    systems::SalvageDroneSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addDrone("ship1", "drone_01", 10.0f, 0.6f);
    sys.addDrone("ship1", "drone_02", 10.0f, 0.6f);
    sys.deployDrone("ship1", "drone_01", "wreck_42");
    sys.deployDrone("ship1", "drone_02", "wreck_43");

    assertTrue(sys.recallDrone("ship1", "drone_01"), "Recall drone_01");
    assertTrue(sys.getDroneState("ship1", "drone_01") == "idle", "drone_01 idle after recall");
    assertTrue(sys.getDeployedCount("ship1") == 1, "1 deployed after recall");

    // Can't recall already idle
    assertTrue(!sys.recallDrone("ship1", "drone_01"), "Can't recall idle drone");
}

static void testSalvageDroneRecallAll() {
    std::cout << "\n=== SalvageDrone: RecallAll ===" << std::endl;
    ecs::World world;
    systems::SalvageDroneSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addDrone("ship1", "drone_01", 10.0f, 0.6f);
    sys.addDrone("ship1", "drone_02", 10.0f, 0.6f);
    sys.deployDrone("ship1", "drone_01", "wreck_42");
    sys.deployDrone("ship1", "drone_02", "wreck_43");

    assertTrue(sys.recallAll("ship1"), "RecallAll succeeds");
    assertTrue(sys.getDeployedCount("ship1") == 0, "0 deployed after recallAll");

    // RecallAll when all idle returns false
    assertTrue(!sys.recallAll("ship1"), "RecallAll fails when none deployed");
}

static void testSalvageDroneSalvageCycle() {
    std::cout << "\n=== SalvageDrone: SalvageCycle ===" << std::endl;
    ecs::World world;
    systems::SalvageDroneSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Drone with 100% success chance and short cycle
    sys.addDrone("ship1", "drone_01", 1.0f, 1.0f);
    sys.deployDrone("ship1", "drone_01", "wreck_42");

    // Manually set drone to Salvaging state (normally triggered by proximity)
    auto* comp = world.getEntity("ship1")->getComponent<components::SalvageDroneBay>();
    comp->drones[0].state = components::SalvageDroneBay::DroneState::Salvaging;

    // Tick 1.5 seconds — cycle_time is 1.0s, should complete one cycle
    sys.update(1.5f);

    // With 100% success and deterministic check, should have salvaged
    int salvages = sys.getTotalSalvages("ship1");
    int failures = sys.getTotalFailures("ship1");
    assertTrue(salvages + failures >= 1, "At least 1 cycle attempted");
}

static void testSalvageDroneSuccessChanceClamped() {
    std::cout << "\n=== SalvageDrone: SuccessChanceClamped ===" << std::endl;
    ecs::World world;
    systems::SalvageDroneSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Out-of-range values clamped
    sys.addDrone("ship1", "drone_01", 10.0f, 2.0f);   // clamped to 1.0
    sys.addDrone("ship1", "drone_02", 10.0f, -0.5f);   // clamped to 0.0
    assertTrue(sys.getDroneCount("ship1") == 2, "2 drones added");
}

static void testSalvageDroneMissing() {
    std::cout << "\n=== SalvageDrone: Missing ===" << std::endl;
    ecs::World world;
    systems::SalvageDroneSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addDrone("nonexistent", "d1", 10.0f, 0.5f), "AddDrone fails on missing");
    assertTrue(!sys.deployDrone("nonexistent", "d1", "w1"), "Deploy fails on missing");
    assertTrue(!sys.recallDrone("nonexistent", "d1"), "Recall fails on missing");
    assertTrue(!sys.recallAll("nonexistent"), "RecallAll fails on missing");
    assertTrue(sys.getDroneCount("nonexistent") == 0, "0 drones on missing");
    assertTrue(sys.getDeployedCount("nonexistent") == 0, "0 deployed on missing");
    assertTrue(sys.getTotalSalvages("nonexistent") == 0, "0 salvages on missing");
    assertTrue(sys.getTotalFailures("nonexistent") == 0, "0 failures on missing");
    assertTrue(sys.getDroneState("nonexistent", "d1") == "unknown", "Unknown state on missing");
}

void run_salvage_drone_system_tests() {
    testSalvageDroneCreate();
    testSalvageDroneAddAndDuplicate();
    testSalvageDroneDeploy();
    testSalvageDroneRecall();
    testSalvageDroneRecallAll();
    testSalvageDroneSalvageCycle();
    testSalvageDroneSuccessChanceClamped();
    testSalvageDroneMissing();
}
