// Tests for: Power Grid Management System
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/power_grid_management_system.h"

using namespace atlas;

// ==================== Power Grid Management System Tests ====================

static void testPowerGridCreate() {
    std::cout << "\n=== PowerGrid: Create ===" << std::endl;
    ecs::World world;
    systems::PowerGridManagementSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 1000.0f), "Init with 1000 MW");
    assertTrue(sys.getModuleCount("ship1") == 0, "0 modules");
    assertTrue(sys.getOnlineCount("ship1") == 0, "0 online");
    assertTrue(sys.getTotalOutput("ship1") == 1000.0f, "1000 MW output");
    assertTrue(sys.getTotalDraw("ship1") == 0.0f, "0 MW draw");
    assertTrue(sys.getAvailablePower("ship1") == 1000.0f, "1000 MW available");
    assertTrue(!sys.isOverloaded("ship1"), "Not overloaded");
}

static void testPowerGridAddAndOnline() {
    std::cout << "\n=== PowerGrid: AddAndOnline ===" << std::endl;
    ecs::World world;
    systems::PowerGridManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 500.0f);

    assertTrue(sys.addModule("ship1", "gun_01", 200.0f, 5), "Add gun_01");
    assertTrue(sys.addModule("ship1", "shield_01", 150.0f, 8), "Add shield_01");
    assertTrue(sys.getModuleCount("ship1") == 2, "2 modules");

    // Duplicate rejected
    assertTrue(!sys.addModule("ship1", "gun_01", 100.0f, 3), "Duplicate rejected");

    assertTrue(sys.onlineModule("ship1", "gun_01"), "Online gun_01");
    assertTrue(sys.getOnlineCount("ship1") == 1, "1 online");
    assertTrue(approxEqual(sys.getTotalDraw("ship1"), 200.0f), "200 MW draw");
    assertTrue(approxEqual(sys.getAvailablePower("ship1"), 300.0f), "300 MW available");

    assertTrue(sys.onlineModule("ship1", "shield_01"), "Online shield_01");
    assertTrue(sys.getOnlineCount("ship1") == 2, "2 online");
    assertTrue(approxEqual(sys.getTotalDraw("ship1"), 350.0f), "350 MW draw");
}

static void testPowerGridInsufficientPower() {
    std::cout << "\n=== PowerGrid: InsufficientPower ===" << std::endl;
    ecs::World world;
    systems::PowerGridManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 300.0f);

    sys.addModule("ship1", "gun_01", 200.0f, 5);
    sys.addModule("ship1", "gun_02", 200.0f, 5);
    sys.onlineModule("ship1", "gun_01");  // 200 MW — fits

    // gun_02 needs 200 MW but only 100 available
    assertTrue(!sys.onlineModule("ship1", "gun_02"), "Insufficient power rejected");
    assertTrue(sys.getOnlineCount("ship1") == 1, "Still only 1 online");
}

static void testPowerGridOffline() {
    std::cout << "\n=== PowerGrid: Offline ===" << std::endl;
    ecs::World world;
    systems::PowerGridManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 500.0f);

    sys.addModule("ship1", "gun_01", 200.0f, 5);
    sys.onlineModule("ship1", "gun_01");
    assertTrue(sys.offlineModule("ship1", "gun_01"), "Offline gun_01");
    assertTrue(sys.getOnlineCount("ship1") == 0, "0 online");
    assertTrue(approxEqual(sys.getTotalDraw("ship1"), 0.0f), "0 MW draw after offline");

    // Can't offline again
    assertTrue(!sys.offlineModule("ship1", "gun_01"), "Double offline rejected");
}

static void testPowerGridRemoveModule() {
    std::cout << "\n=== PowerGrid: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::PowerGridManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 500.0f);

    sys.addModule("ship1", "gun_01", 200.0f, 5);
    sys.onlineModule("ship1", "gun_01");

    assertTrue(sys.removeModule("ship1", "gun_01"), "Remove gun_01");
    assertTrue(sys.getModuleCount("ship1") == 0, "0 modules after remove");
    assertTrue(approxEqual(sys.getTotalDraw("ship1"), 0.0f), "0 MW draw after remove");
}

static void testPowerGridAutoOfflineOverload() {
    std::cout << "\n=== PowerGrid: AutoOfflineOverload ===" << std::endl;
    ecs::World world;
    systems::PowerGridManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 500.0f);

    sys.addModule("ship1", "gun_01", 200.0f, 3);     // priority 3 (lowest)
    sys.addModule("ship1", "shield_01", 200.0f, 8);   // priority 8 (highest)
    sys.onlineModule("ship1", "gun_01");
    sys.onlineModule("ship1", "shield_01");

    // Reduce output to cause overload
    sys.setTotalOutput("ship1", 300.0f);

    // Total draw = 400, output = 300 → overloaded
    // Update should auto-offline lowest priority (gun_01, pri=3)
    sys.update(0.1f);
    assertTrue(sys.getTotalOverloads("ship1") >= 1, "Overload detected");
    assertTrue(sys.getOnlineCount("ship1") == 1, "1 module still online");
    assertTrue(approxEqual(sys.getTotalDraw("ship1"), 200.0f), "200 MW draw after auto-offline");
}

static void testPowerGridMaxModules() {
    std::cout << "\n=== PowerGrid: MaxModules ===" << std::endl;
    ecs::World world;
    systems::PowerGridManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 10000.0f);

    // Fill to max (12)
    for (int i = 0; i < 12; i++) {
        assertTrue(sys.addModule("ship1", "mod_" + std::to_string(i), 10.0f, 5),
                   "Add module " + std::to_string(i));
    }
    assertTrue(sys.getModuleCount("ship1") == 12, "12 modules at max");
    assertTrue(!sys.addModule("ship1", "mod_12", 10.0f, 5), "13th module rejected");
}

static void testPowerGridMissing() {
    std::cout << "\n=== PowerGrid: Missing ===" << std::endl;
    ecs::World world;
    systems::PowerGridManagementSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 500.0f), "Init fails on missing");
    assertTrue(!sys.addModule("nonexistent", "m1", 100.0f, 5), "AddModule fails on missing");
    assertTrue(!sys.onlineModule("nonexistent", "m1"), "Online fails on missing");
    assertTrue(!sys.offlineModule("nonexistent", "m1"), "Offline fails on missing");
    assertTrue(!sys.removeModule("nonexistent", "m1"), "Remove fails on missing");
    assertTrue(!sys.setTotalOutput("nonexistent", 500.0f), "SetOutput fails on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "0 modules on missing");
    assertTrue(sys.getOnlineCount("nonexistent") == 0, "0 online on missing");
    assertTrue(sys.getTotalDraw("nonexistent") == 0.0f, "0 draw on missing");
    assertTrue(sys.getTotalOutput("nonexistent") == 0.0f, "0 output on missing");
    assertTrue(sys.getAvailablePower("nonexistent") == 0.0f, "0 available on missing");
    assertTrue(!sys.isOverloaded("nonexistent"), "Not overloaded on missing");
    assertTrue(sys.getTotalOverloads("nonexistent") == 0, "0 overloads on missing");
}

void run_power_grid_management_system_tests() {
    testPowerGridCreate();
    testPowerGridAddAndOnline();
    testPowerGridInsufficientPower();
    testPowerGridOffline();
    testPowerGridRemoveModule();
    testPowerGridAutoOfflineOverload();
    testPowerGridMaxModules();
    testPowerGridMissing();
}
