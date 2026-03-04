// Tests for: ModManagerSystem Tests
#include "test_log.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/mod_manager_system.h"

using namespace atlas;

// ==================== ModManagerSystem Tests ====================

static void testModManagerCreate() {
    std::cout << "\n=== ModManager: Create ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    auto* e = world.createEntity("mgr1");
    assertTrue(sys.createManager("mgr1"), "Create manager succeeds");
    auto* m = e->getComponent<components::ModManager>();
    assertTrue(m != nullptr, "Component exists");
    assertTrue(m->max_mods == 50, "Default max_mods is 50");
    assertTrue(m->total_installed == 0, "Default total_installed is 0");
    assertTrue(m->active, "Manager is active by default");
}

static void testModManagerInstall() {
    std::cout << "\n=== ModManager: Install ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    assertTrue(sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev"), "Install succeeds");
    assertTrue(sys.isInstalled("mgr1", "mod_a"), "Mod is installed");
    assertTrue(sys.getModCount("mgr1") == 1, "Mod count is 1");
}

static void testModManagerUninstall() {
    std::cout << "\n=== ModManager: Uninstall ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev");
    assertTrue(sys.uninstallMod("mgr1", "mod_a"), "Uninstall succeeds");
    assertTrue(!sys.isInstalled("mgr1", "mod_a"), "Mod no longer installed");
    assertTrue(sys.getModCount("mgr1") == 0, "Mod count is 0");
}

static void testModManagerEnable() {
    std::cout << "\n=== ModManager: Enable/Disable ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev");
    assertTrue(sys.enableMod("mgr1", "mod_a"), "Enable succeeds");
    assertTrue(sys.getEnabledCount("mgr1") == 1, "Enabled count is 1");
    assertTrue(sys.disableMod("mgr1", "mod_a"), "Disable succeeds");
    assertTrue(sys.getEnabledCount("mgr1") == 0, "Enabled count is 0");
}

static void testModManagerDependency() {
    std::cout << "\n=== ModManager: Dependency ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "base", "Base", "1.0", "Dev");
    sys.installMod("mgr1", "ext", "Extension", "1.0", "Dev");
    assertTrue(sys.addDependency("mgr1", "ext", "base"), "Add dependency succeeds");
    sys.update(0.0f);
    auto order = sys.getLoadOrder("mgr1");
    assertTrue(order.size() == 2, "Load order has 2 entries");
    assertTrue(order[0] == "base", "Base comes first in load order");
}

static void testModManagerConflict() {
    std::cout << "\n=== ModManager: Conflict ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev");
    sys.installMod("mgr1", "mod_b", "Beta", "1.0", "Dev");
    sys.addConflict("mgr1", "mod_a", "mod_b");
    sys.enableMod("mgr1", "mod_b");
    assertTrue(sys.hasConflict("mgr1", "mod_a"), "mod_a conflicts with enabled mod_b");
    sys.disableMod("mgr1", "mod_b");
    assertTrue(!sys.hasConflict("mgr1", "mod_a"), "No conflict when mod_b disabled");
}

static void testModManagerMaxMods() {
    std::cout << "\n=== ModManager: MaxMods ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    auto* e = world.createEntity("mgr1");
    sys.createManager("mgr1");
    auto* m = e->getComponent<components::ModManager>();
    m->max_mods = 2;
    assertTrue(sys.installMod("mgr1", "m1", "M1", "1.0", "D"), "Install 1 succeeds");
    assertTrue(sys.installMod("mgr1", "m2", "M2", "1.0", "D"), "Install 2 succeeds");
    assertTrue(!sys.installMod("mgr1", "m3", "M3", "1.0", "D"), "Install 3 fails (max reached)");
}

static void testModManagerDuplicate() {
    std::cout << "\n=== ModManager: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    assertTrue(sys.installMod("mgr1", "mod_a", "Alpha", "1.0", "Dev"), "First install succeeds");
    assertTrue(!sys.installMod("mgr1", "mod_a", "Alpha2", "2.0", "Dev2"), "Duplicate install fails");
    assertTrue(sys.getModCount("mgr1") == 1, "Still only 1 mod");
}

static void testModManagerLoadOrder() {
    std::cout << "\n=== ModManager: LoadOrder ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    world.createEntity("mgr1");
    sys.createManager("mgr1");
    sys.installMod("mgr1", "c", "C", "1.0", "D");
    sys.installMod("mgr1", "b", "B", "1.0", "D");
    sys.installMod("mgr1", "a", "A", "1.0", "D");
    sys.addDependency("mgr1", "c", "b");
    sys.addDependency("mgr1", "b", "a");
    sys.update(0.0f);
    auto order = sys.getLoadOrder("mgr1");
    assertTrue(order.size() == 3, "Load order has 3 entries");
    // a has no deps (order 0), b depends on a (order 1), c depends on b (order 2)
    assertTrue(order[0] == "a", "a is first");
    assertTrue(order[2] == "c", "c is last");
}

static void testModManagerMissing() {
    std::cout << "\n=== ModManager: Missing ===" << std::endl;
    ecs::World world;
    systems::ModManagerSystem sys(&world);
    assertTrue(!sys.createManager("nonexistent"), "Create fails on missing");
    assertTrue(!sys.installMod("nonexistent", "m", "M", "1.0", "D"), "Install fails on missing");
    assertTrue(!sys.uninstallMod("nonexistent", "m"), "Uninstall fails on missing");
    assertTrue(!sys.enableMod("nonexistent", "m"), "Enable fails on missing");
    assertTrue(!sys.disableMod("nonexistent", "m"), "Disable fails on missing");
    assertTrue(sys.getModCount("nonexistent") == 0, "0 mods on missing");
    assertTrue(sys.getEnabledCount("nonexistent") == 0, "0 enabled on missing");
    assertTrue(!sys.isInstalled("nonexistent", "m"), "Not installed on missing");
    assertTrue(sys.getLoadOrder("nonexistent").empty(), "Empty load order on missing");
}


void run_mod_manager_system_tests() {
    testModManagerCreate();
    testModManagerInstall();
    testModManagerUninstall();
    testModManagerEnable();
    testModManagerDependency();
    testModManagerConflict();
    testModManagerMaxMods();
    testModManagerDuplicate();
    testModManagerLoadOrder();
    testModManagerMissing();
}
