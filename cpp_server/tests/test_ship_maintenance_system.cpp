// Tests for: Ship Maintenance System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/ship_maintenance_system.h"

using namespace atlas;

// ==================== Ship Maintenance System Tests ====================

static void testShipMaintenanceCreate() {
    std::cout << "\n=== ShipMaintenance: Create ===" << std::endl;
    ecs::World world;
    systems::ShipMaintenanceSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", "frigate_01"), "Init succeeds");
    assertTrue(sys.getCondition("ship1") == "Pristine", "Pristine initially");
    assertTrue(approxEqual(sys.getHullIntegrity("ship1"), 1.0f), "Full integrity");
    assertTrue(approxEqual(sys.getPerformancePenalty("ship1"), 0.0f), "No penalty");
    assertTrue(sys.getRepairQueueSize("ship1") == 0, "No repairs queued");
    assertTrue(sys.getTotalRepairsCompleted("ship1") == 0, "No repairs completed");
    assertTrue(sys.getTotalRepairCost("ship1") == 0.0, "No repair cost");
}

static void testShipMaintenanceWearOverTime() {
    std::cout << "\n=== ShipMaintenance: WearOverTime ===" << std::endl;
    ecs::World world;
    systems::ShipMaintenanceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01");
    sys.setWearRate("ship1", 0.01f, 0.05f);

    // Normal wear: 0.01 * 10 = 0.1 -> integrity = 0.9
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getHullIntegrity("ship1"), 0.9f), "Integrity after 10s normal wear");
    assertTrue(sys.getCondition("ship1") == "Pristine", "Still Pristine at 0.9");

    // More wear to degrade to Good
    sys.update(5.0f);  // 0.9 - 0.05 = 0.85
    assertTrue(sys.getCondition("ship1") == "Good", "Good at ~0.85");
}

static void testShipMaintenanceCombatWear() {
    std::cout << "\n=== ShipMaintenance: CombatWear ===" << std::endl;
    ecs::World world;
    systems::ShipMaintenanceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01");
    sys.setWearRate("ship1", 0.01f, 0.05f);
    sys.setCombatState("ship1", true);

    // Combat wear: (0.01 + 0.05) * 10 = 0.6 -> integrity = 0.4
    sys.update(10.0f);
    assertTrue(sys.getHullIntegrity("ship1") < 0.5f, "High wear in combat");
    assertTrue(sys.getCondition("ship1") == "Poor", "Poor condition after heavy combat wear");
}

static void testShipMaintenanceApplyDamage() {
    std::cout << "\n=== ShipMaintenance: ApplyDamage ===" << std::endl;
    ecs::World world;
    systems::ShipMaintenanceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01");

    assertTrue(sys.applyDamage("ship1", 0.3f), "Apply 30% damage");
    assertTrue(approxEqual(sys.getHullIntegrity("ship1"), 0.7f), "70% integrity");
    assertTrue(sys.getCondition("ship1") == "Good", "Good condition");

    assertTrue(sys.applyDamage("ship1", 0.5f), "Apply 50% more damage");
    assertTrue(approxEqual(sys.getHullIntegrity("ship1"), 0.2f), "20% integrity");
    assertTrue(sys.getCondition("ship1") == "Critical", "Critical condition");
    assertTrue(sys.getPerformancePenalty("ship1") > 0.0f, "Has performance penalty");
}

static void testShipMaintenanceRepairQueue() {
    std::cout << "\n=== ShipMaintenance: RepairQueue ===" << std::endl;
    ecs::World world;
    systems::ShipMaintenanceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01");
    sys.applyDamage("ship1", 0.5f);

    // Must be docked to queue repairs
    assertTrue(!sys.queueRepair("ship1", "hull_plates", 500.0, 10.0f), "Cannot repair while undocked");

    sys.setDockedState("ship1", true);
    assertTrue(sys.queueRepair("ship1", "hull_plates", 500.0, 10.0f), "Queue repair while docked");
    assertTrue(sys.getRepairQueueSize("ship1") == 1, "1 repair queued");

    // Run repair to completion
    sys.update(11.0f);
    assertTrue(sys.getTotalRepairsCompleted("ship1") == 1, "1 repair completed");
    assertTrue(sys.getTotalRepairCost("ship1") == 500.0, "Repair cost recorded");
    assertTrue(sys.getRepairQueueSize("ship1") == 0, "Queue empty after completion");
    assertTrue(sys.getHullIntegrity("ship1") > 0.5f, "Integrity improved after repair");
}

static void testShipMaintenanceConditionThresholds() {
    std::cout << "\n=== ShipMaintenance: ConditionThresholds ===" << std::endl;
    ecs::World world;
    systems::ShipMaintenanceSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01");

    assertTrue(sys.getCondition("ship1") == "Pristine", "1.0 = Pristine");

    sys.applyDamage("ship1", 0.15f);
    assertTrue(sys.getCondition("ship1") == "Good", "0.85 = Good");

    sys.applyDamage("ship1", 0.2f);
    assertTrue(sys.getCondition("ship1") == "Fair", "0.65 = Fair");

    sys.applyDamage("ship1", 0.2f);
    assertTrue(sys.getCondition("ship1") == "Poor", "0.45 = Poor");

    sys.applyDamage("ship1", 0.25f);
    assertTrue(sys.getCondition("ship1") == "Critical", "0.20 = Critical");
}

static void testShipMaintenanceMissing() {
    std::cout << "\n=== ShipMaintenance: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipMaintenanceSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "ship"), "Init fails on missing");
    assertTrue(!sys.setWearRate("nonexistent", 0.01f, 0.05f), "SetWearRate fails on missing");
    assertTrue(!sys.setCombatState("nonexistent", true), "SetCombatState fails on missing");
    assertTrue(!sys.setDockedState("nonexistent", true), "SetDockedState fails on missing");
    assertTrue(!sys.queueRepair("nonexistent", "hull", 100.0, 5.0f), "QueueRepair fails on missing");
    assertTrue(!sys.applyDamage("nonexistent", 0.1f), "ApplyDamage fails on missing");
    assertTrue(sys.getCondition("nonexistent") == "Unknown", "Unknown condition on missing");
    assertTrue(sys.getHullIntegrity("nonexistent") == 0.0f, "0 integrity on missing");
    assertTrue(sys.getPerformancePenalty("nonexistent") == 0.0f, "0 penalty on missing");
    assertTrue(sys.getRepairQueueSize("nonexistent") == 0, "0 queue on missing");
    assertTrue(sys.getTotalRepairsCompleted("nonexistent") == 0, "0 repairs on missing");
    assertTrue(sys.getTotalRepairCost("nonexistent") == 0.0, "0 cost on missing");
}


void run_ship_maintenance_system_tests() {
    testShipMaintenanceCreate();
    testShipMaintenanceWearOverTime();
    testShipMaintenanceCombatWear();
    testShipMaintenanceApplyDamage();
    testShipMaintenanceRepairQueue();
    testShipMaintenanceConditionThresholds();
    testShipMaintenanceMissing();
}
