// Tests for: ModulePowerGrid System Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/module_power_grid_system.h"

using namespace atlas;

// ==================== ModulePowerGrid System Tests ====================

static void testPowerGridCreate() {
    std::cout << "\n=== ModulePowerGrid: Create ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializePowerGrid("ship1", 100.0f, 200.0f), "Init power grid succeeds");
    assertTrue(approxEqual(sys.getCpuUsed("ship1"), 0.0f), "0 CPU used");
    assertTrue(approxEqual(sys.getPgUsed("ship1"), 0.0f), "0 PG used");
    assertTrue(approxEqual(sys.getCpuFree("ship1"), 100.0f), "100 CPU free");
    assertTrue(approxEqual(sys.getPgFree("ship1"), 200.0f), "200 PG free");
    assertTrue(sys.getModuleCount("ship1") == 0, "0 modules");
}

static void testPowerGridFitModule() {
    std::cout << "\n=== ModulePowerGrid: FitModule ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    sys.initializePowerGrid("ship1", 100.0f, 200.0f);

    assertTrue(sys.fitModule("ship1", "gun1", "Laser", 25.0f, 50.0f), "Fit laser");
    assertTrue(sys.getModuleCount("ship1") == 1, "1 module");
    assertTrue(approxEqual(sys.getCpuUsed("ship1"), 25.0f), "25 CPU used");
    assertTrue(approxEqual(sys.getPgUsed("ship1"), 50.0f), "50 PG used");
    assertTrue(sys.getOnlineCount("ship1") == 1, "1 online");
}

static void testPowerGridBudgetEnforcement() {
    std::cout << "\n=== ModulePowerGrid: BudgetEnforcement ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    sys.initializePowerGrid("ship1", 50.0f, 100.0f);

    assertTrue(sys.fitModule("ship1", "m1", "Shield", 30.0f, 60.0f), "First module fits");
    assertTrue(!sys.fitModule("ship1", "m2", "Armor", 30.0f, 60.0f), "Second exceeds budget");
    assertTrue(sys.getModuleCount("ship1") == 1, "Only 1 module fitted");
}

static void testPowerGridOnlineOffline() {
    std::cout << "\n=== ModulePowerGrid: OnlineOffline ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    sys.initializePowerGrid("ship1", 100.0f, 200.0f);

    sys.fitModule("ship1", "m1", "Gun", 50.0f, 100.0f);
    assertTrue(sys.setModuleOnline("ship1", "m1", false), "Take offline");
    assertTrue(approxEqual(sys.getCpuUsed("ship1"), 0.0f), "0 CPU when offline");
    assertTrue(sys.getOnlineCount("ship1") == 0, "0 online");

    assertTrue(sys.setModuleOnline("ship1", "m1", true), "Bring online");
    assertTrue(approxEqual(sys.getCpuUsed("ship1"), 50.0f), "50 CPU when online");
}

static void testPowerGridOnlineBudgetCheck() {
    std::cout << "\n=== ModulePowerGrid: OnlineBudgetCheck ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    sys.initializePowerGrid("ship1", 60.0f, 120.0f);

    sys.fitModule("ship1", "m1", "Gun", 40.0f, 80.0f);
    sys.fitModule("ship1", "m2", "Shield", 20.0f, 40.0f);
    sys.setModuleOnline("ship1", "m2", false);

    // m1 uses 40 CPU, bringing m2 online needs 20 more = 60 total, fits
    assertTrue(sys.setModuleOnline("ship1", "m2", true), "m2 fits within budget");

    // Now at 60/60 CPU - take m2 offline to test exceeded budget scenario
    sys.setModuleOnline("ship1", "m2", false);
    // Reduce capacity
    sys.setCapacity("ship1", 30.0f, 60.0f);
    // m2 needs 20 CPU, but only 30-40=-10 free (m1 exceeds new budget)
    // Force enforcement on next tick
    sys.update(0.1f);
    // m1 should be forced offline since it exceeds new budget
    assertTrue(sys.getModulesForcedOffline("ship1") > 0, "Module forced offline after capacity drop");
}

static void testPowerGridRemoveModule() {
    std::cout << "\n=== ModulePowerGrid: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    sys.initializePowerGrid("ship1", 100.0f, 200.0f);

    sys.fitModule("ship1", "m1", "Gun", 25.0f, 50.0f);
    assertTrue(sys.removeModule("ship1", "m1"), "Remove succeeds");
    assertTrue(sys.getModuleCount("ship1") == 0, "0 modules after remove");
    assertTrue(approxEqual(sys.getCpuUsed("ship1"), 0.0f), "0 CPU after remove");
}

static void testPowerGridCapacityDrop() {
    std::cout << "\n=== ModulePowerGrid: CapacityDrop ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    sys.initializePowerGrid("ship1", 100.0f, 200.0f);

    sys.fitModule("ship1", "m1", "Gun", 40.0f, 80.0f);
    sys.fitModule("ship1", "m2", "Shield", 40.0f, 80.0f);
    // Reduce capacity below current usage
    sys.setCapacity("ship1", 50.0f, 100.0f);
    sys.update(0.1f);

    // Should force at least one module offline
    assertTrue(sys.getOnlineCount("ship1") < 2, "At least one forced offline");
    assertTrue(sys.getModulesForcedOffline("ship1") > 0, "Forced offline count > 0");
}

static void testPowerGridDuplicateFit() {
    std::cout << "\n=== ModulePowerGrid: DuplicateFit ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    sys.initializePowerGrid("ship1", 100.0f, 200.0f);

    assertTrue(sys.fitModule("ship1", "m1", "Gun", 25.0f, 50.0f), "First fit succeeds");
    assertTrue(!sys.fitModule("ship1", "m1", "Gun2", 25.0f, 50.0f), "Duplicate module_id rejected");
}

static void testPowerGridDuplicateInit() {
    std::cout << "\n=== ModulePowerGrid: DuplicateInit ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializePowerGrid("ship1"), "First init succeeds");
    assertTrue(!sys.initializePowerGrid("ship1"), "Duplicate init rejected");
}

static void testPowerGridMissing() {
    std::cout << "\n=== ModulePowerGrid: Missing ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    assertTrue(!sys.initializePowerGrid("nonexistent"), "Init fails on missing");
    assertTrue(!sys.fitModule("nonexistent", "m1", "Gun", 10.0f, 20.0f), "Fit fails on missing");
    assertTrue(approxEqual(sys.getCpuUsed("nonexistent"), 0.0f), "0 CPU on missing");
    assertTrue(approxEqual(sys.getPgUsed("nonexistent"), 0.0f), "0 PG on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "0 modules on missing");
    assertTrue(!sys.removeModule("nonexistent", "m1"), "Remove fails on missing");
}

static void testPowerGridInvalidInput() {
    std::cout << "\n=== ModulePowerGrid: InvalidInput ===" << std::endl;
    ecs::World world;
    systems::ModulePowerGridSystem sys(&world);
    world.createEntity("ship1");
    sys.initializePowerGrid("ship1", 100.0f, 200.0f);

    assertTrue(!sys.fitModule("ship1", "", "Gun", 10.0f, 20.0f), "Empty module_id rejected");
    assertTrue(!sys.fitModule("ship1", "m1", "Gun", -10.0f, 20.0f), "Negative CPU rejected");
    assertTrue(!sys.fitModule("ship1", "m1", "Gun", 10.0f, -20.0f), "Negative PG rejected");
    assertTrue(!sys.setCapacity("ship1", -10.0f, 200.0f), "Negative CPU capacity rejected");
}

void run_module_power_grid_system_tests() {
    testPowerGridCreate();
    testPowerGridFitModule();
    testPowerGridBudgetEnforcement();
    testPowerGridOnlineOffline();
    testPowerGridOnlineBudgetCheck();
    testPowerGridRemoveModule();
    testPowerGridCapacityDrop();
    testPowerGridDuplicateFit();
    testPowerGridDuplicateInit();
    testPowerGridMissing();
    testPowerGridInvalidInput();
}
