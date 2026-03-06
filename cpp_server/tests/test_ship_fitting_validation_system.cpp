// Tests for: Ship Fitting Validation System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/ship_fitting_validation_system.h"

using namespace atlas;

// ==================== Ship Fitting Validation System Tests ====================

static void testFittingValidationCreate() {
    std::cout << "\n=== FittingValidation: Create ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    world.createEntity("ship1");
    // Frigate: 200 CPU, 50 PG, 3 high, 3 mid, 2 low
    assertTrue(sys.initialize("ship1", 200.0f, 50.0f, 3, 3, 2), "Init succeeds");
    assertTrue(sys.getFittedModuleCount("ship1") == 0, "No modules");
    assertTrue(approxEqual(sys.getCpuUsed("ship1"), 0.0f), "0 CPU used");
    assertTrue(approxEqual(sys.getCpuRemaining("ship1"), 200.0f), "200 CPU remaining");
    assertTrue(approxEqual(sys.getPowerGridUsed("ship1"), 0.0f), "0 PG used");
    assertTrue(approxEqual(sys.getPowerGridRemaining("ship1"), 50.0f), "50 PG remaining");
    assertTrue(sys.getHighSlotsRemaining("ship1") == 3, "3 high slots free");
    assertTrue(sys.getMidSlotsRemaining("ship1") == 3, "3 mid slots free");
    assertTrue(sys.getLowSlotsRemaining("ship1") == 2, "2 low slots free");
    assertTrue(sys.isValidFit("ship1"), "Valid fit with no modules");
    assertTrue(!sys.isCpuOverloaded("ship1"), "Not CPU overloaded");
    assertTrue(!sys.isPowerGridOverloaded("ship1"), "Not PG overloaded");
}

static void testFittingValidationFitModules() {
    std::cout << "\n=== FittingValidation: FitModules ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 200.0f, 50.0f, 3, 3, 2);
    assertTrue(sys.fitModule("ship1", "laser_1", "high", 30.0f, 8.0f), "Fit laser");
    assertTrue(sys.fitModule("ship1", "shield_1", "mid", 25.0f, 10.0f), "Fit shield");
    assertTrue(sys.fitModule("ship1", "armor_1", "low", 15.0f, 5.0f), "Fit armor");
    assertTrue(sys.getFittedModuleCount("ship1") == 3, "3 modules");
    assertTrue(sys.hasFittedModule("ship1", "laser_1"), "Has laser");
    assertTrue(sys.hasFittedModule("ship1", "shield_1"), "Has shield");
    assertTrue(!sys.hasFittedModule("ship1", "missing"), "No missing module");
    assertTrue(approxEqual(sys.getCpuUsed("ship1"), 70.0f), "70 CPU");
    assertTrue(approxEqual(sys.getCpuRemaining("ship1"), 130.0f), "130 CPU remaining");
    assertTrue(approxEqual(sys.getPowerGridUsed("ship1"), 23.0f), "23 PG");
    assertTrue(approxEqual(sys.getPowerGridRemaining("ship1"), 27.0f), "27 PG remaining");
    assertTrue(sys.isValidFit("ship1"), "Still valid");
    // Duplicate rejected
    assertTrue(!sys.fitModule("ship1", "laser_1", "high", 30.0f, 8.0f), "Dup rejected");
}

static void testFittingValidationSlotLimits() {
    std::cout << "\n=== FittingValidation: SlotLimits ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f, 1000.0f, 2, 1, 1); // Small ship
    sys.fitModule("ship1", "h1", "high", 10.0f, 5.0f);
    sys.fitModule("ship1", "h2", "high", 10.0f, 5.0f);
    assertTrue(!sys.fitModule("ship1", "h3", "high", 10.0f, 5.0f), "High slots full");
    assertTrue(sys.getHighSlotsUsed("ship1") == 2, "2 high used");
    assertTrue(sys.getHighSlotsRemaining("ship1") == 0, "0 high remaining");
    sys.fitModule("ship1", "m1", "mid", 10.0f, 5.0f);
    assertTrue(!sys.fitModule("ship1", "m2", "mid", 10.0f, 5.0f), "Mid slots full");
    assertTrue(sys.getMidSlotsUsed("ship1") == 1, "1 mid used");
    sys.fitModule("ship1", "l1", "low", 10.0f, 5.0f);
    assertTrue(!sys.fitModule("ship1", "l2", "low", 10.0f, 5.0f), "Low slots full");
    assertTrue(sys.getLowSlotsUsed("ship1") == 1, "1 low used");
    // Unknown slot type
    assertTrue(!sys.fitModule("ship1", "x1", "rig", 1.0f, 1.0f), "Unknown slot rejected");
}

static void testFittingValidationUnfit() {
    std::cout << "\n=== FittingValidation: Unfit ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 200.0f, 50.0f, 3, 3, 2);
    sys.fitModule("ship1", "laser_1", "high", 30.0f, 8.0f);
    sys.fitModule("ship1", "shield_1", "mid", 25.0f, 10.0f);
    assertTrue(sys.unfitModule("ship1", "laser_1"), "Unfit laser");
    assertTrue(sys.getFittedModuleCount("ship1") == 1, "1 module left");
    assertTrue(!sys.hasFittedModule("ship1", "laser_1"), "Laser gone");
    assertTrue(approxEqual(sys.getCpuUsed("ship1"), 25.0f), "25 CPU after unfit");
    assertTrue(approxEqual(sys.getPowerGridUsed("ship1"), 10.0f), "10 PG after unfit");
    assertTrue(sys.getHighSlotsUsed("ship1") == 0, "0 high slots used");
    assertTrue(!sys.unfitModule("ship1", "laser_1"), "Double unfit fails");
    assertTrue(!sys.unfitModule("ship1", "nonexistent"), "Nonexistent fails");
}

static void testFittingValidationOverload() {
    std::cout << "\n=== FittingValidation: Overload ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f, 30.0f, 5, 5, 5);
    // Fit modules that exceed CPU
    sys.fitModule("ship1", "h1", "high", 60.0f, 10.0f);
    sys.fitModule("ship1", "h2", "high", 50.0f, 10.0f); // 110 CPU > 100 max
    assertTrue(!sys.isValidFit("ship1"), "Invalid — CPU overloaded");
    assertTrue(sys.isCpuOverloaded("ship1"), "CPU overloaded");
    assertTrue(!sys.isPowerGridOverloaded("ship1"), "PG not overloaded");
    assertTrue(sys.getValidationErrorCount("ship1") == 1, "1 error");
    // Also overload PG
    sys.fitModule("ship1", "m1", "mid", 0.0f, 15.0f); // 35 PG > 30 max
    assertTrue(sys.isPowerGridOverloaded("ship1"), "PG now overloaded");
    assertTrue(sys.getValidationErrorCount("ship1") == 2, "2 errors");
    // Unfit to fix
    sys.unfitModule("ship1", "h2");
    sys.unfitModule("ship1", "m1");
    assertTrue(sys.isValidFit("ship1"), "Valid again after unfit");
    assertTrue(sys.getValidationErrorCount("ship1") == 0, "0 errors");
}

static void testFittingValidationUtilization() {
    std::cout << "\n=== FittingValidation: Utilization ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 200.0f, 100.0f, 5, 5, 5);
    sys.fitModule("ship1", "h1", "high", 100.0f, 50.0f);
    assertTrue(approxEqual(sys.getCpuUtilization("ship1"), 0.5f), "50% CPU util");
    assertTrue(approxEqual(sys.getPowerGridUtilization("ship1"), 0.5f), "50% PG util");
    sys.fitModule("ship1", "h2", "high", 100.0f, 50.0f);
    assertTrue(approxEqual(sys.getCpuUtilization("ship1"), 1.0f), "100% CPU util");
    assertTrue(approxEqual(sys.getPowerGridUtilization("ship1"), 1.0f), "100% PG util");
}

static void testFittingValidationSlotCounts() {
    std::cout << "\n=== FittingValidation: SlotCounts ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 1000.0f, 1000.0f, 4, 3, 2);
    sys.fitModule("ship1", "h1", "high", 10.0f, 5.0f);
    sys.fitModule("ship1", "h2", "high", 10.0f, 5.0f);
    sys.fitModule("ship1", "m1", "mid", 10.0f, 5.0f);
    sys.fitModule("ship1", "l1", "low", 10.0f, 5.0f);
    assertTrue(sys.getHighSlotsUsed("ship1") == 2, "2 high used");
    assertTrue(sys.getHighSlotsRemaining("ship1") == 2, "2 high remaining");
    assertTrue(sys.getMidSlotsUsed("ship1") == 1, "1 mid used");
    assertTrue(sys.getMidSlotsRemaining("ship1") == 2, "2 mid remaining");
    assertTrue(sys.getLowSlotsUsed("ship1") == 1, "1 low used");
    assertTrue(sys.getLowSlotsRemaining("ship1") == 1, "1 low remaining");
    assertTrue(sys.getFittedModuleCount("ship1") == 4, "4 total modules");
}

static void testFittingValidationUpdate() {
    std::cout << "\n=== FittingValidation: Update ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 200.0f, 50.0f, 3, 3, 2);
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("ship1");
    auto* state = entity->getComponent<components::ShipFittingValidationState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testFittingValidationMissing() {
    std::cout << "\n=== FittingValidation: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipFittingValidationSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 100.0f, 50.0f, 3, 3, 2), "Init fails");
    assertTrue(!sys.fitModule("nonexistent", "m", "high", 1.0f, 1.0f), "fit fails");
    assertTrue(!sys.unfitModule("nonexistent", "m"), "unfit fails");
    assertTrue(!sys.hasFittedModule("nonexistent", "m"), "hasFitted false");
    assertTrue(sys.getFittedModuleCount("nonexistent") == 0, "0 modules");
    assertTrue(approxEqual(sys.getCpuUsed("nonexistent"), 0.0f), "0 CPU");
    assertTrue(approxEqual(sys.getCpuRemaining("nonexistent"), 0.0f), "0 CPU remaining");
    assertTrue(approxEqual(sys.getPowerGridUsed("nonexistent"), 0.0f), "0 PG");
    assertTrue(approxEqual(sys.getPowerGridRemaining("nonexistent"), 0.0f), "0 PG remaining");
    assertTrue(sys.getHighSlotsUsed("nonexistent") == 0, "0 high");
    assertTrue(sys.getMidSlotsUsed("nonexistent") == 0, "0 mid");
    assertTrue(sys.getLowSlotsUsed("nonexistent") == 0, "0 low");
    assertTrue(sys.getHighSlotsRemaining("nonexistent") == 0, "0 high remaining");
    assertTrue(sys.getMidSlotsRemaining("nonexistent") == 0, "0 mid remaining");
    assertTrue(sys.getLowSlotsRemaining("nonexistent") == 0, "0 low remaining");
    assertTrue(!sys.isValidFit("nonexistent"), "Not valid");
    assertTrue(!sys.isCpuOverloaded("nonexistent"), "Not overloaded");
    assertTrue(!sys.isPowerGridOverloaded("nonexistent"), "Not PG overloaded");
    assertTrue(sys.getValidationErrorCount("nonexistent") == 0, "0 errors");
    assertTrue(approxEqual(sys.getCpuUtilization("nonexistent"), 0.0f), "0 CPU util");
    assertTrue(approxEqual(sys.getPowerGridUtilization("nonexistent"), 0.0f), "0 PG util");
}

void run_ship_fitting_validation_system_tests() {
    testFittingValidationCreate();
    testFittingValidationFitModules();
    testFittingValidationSlotLimits();
    testFittingValidationUnfit();
    testFittingValidationOverload();
    testFittingValidationUtilization();
    testFittingValidationSlotCounts();
    testFittingValidationUpdate();
    testFittingValidationMissing();
}
