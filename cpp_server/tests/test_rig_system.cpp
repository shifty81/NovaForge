// Tests for: Rig System Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/rig_system.h"

using namespace atlas;

// ==================== Rig System Tests ====================

static void testRigLoadoutDefaults() {
    std::cout << "\n=== Rig Loadout Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig1");
    auto* loadout = addComp<components::RigLoadout>(e);
    assertTrue(loadout->rack_width == 2, "Default rack width 2");
    assertTrue(loadout->rack_height == 2, "Default rack height 2");
    assertTrue(loadout->max_slots() == 4, "Default max slots 4");
}

static void testRigInstallModule() {
    std::cout << "\n=== Rig Install Module ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig2");
    addComp<components::RigLoadout>(e);
    auto* mod = world.createEntity("mod1");
    addComp<components::RigModule>(mod);

    systems::RigSystem sys(&world);
    assertTrue(sys.installModule("rig2", "mod1"), "Install module succeeds");
    assertTrue(sys.getInstalledCount("rig2") == 1, "One module installed");
}

static void testRigModuleFull() {
    std::cout << "\n=== Rig Module Full ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig3");
    auto* loadout = addComp<components::RigLoadout>(e);
    loadout->rack_width = 1;
    loadout->rack_height = 1;

    systems::RigSystem sys(&world);
    auto* m1 = world.createEntity("m1");
    addComp<components::RigModule>(m1);
    assertTrue(sys.installModule("rig3", "m1"), "First install succeeds");
    assertTrue(!sys.installModule("rig3", "m2"), "Second install fails (full)");
}

static void testRigDerivedStats() {
    std::cout << "\n=== Rig Derived Stats ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig4");
    addComp<components::RigLoadout>(e);
    auto* mod = world.createEntity("mod_ls");
    auto* rm = addComp<components::RigModule>(mod);
    rm->type = components::RigModule::ModuleType::LifeSupport;
    rm->tier = 2;
    rm->efficiency = 1.0f;

    systems::RigSystem sys(&world);
    sys.installModule("rig4", "mod_ls");
    sys.update(0.0f);

    auto* loadout = e->getComponent<components::RigLoadout>();
    assertTrue(approxEqual(loadout->total_oxygen, 200.0f), "LifeSupport tier 2 = 200 oxygen");
}

static void testRigRemoveModule() {
    std::cout << "\n=== Rig Remove Module ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rig5");
    addComp<components::RigLoadout>(e);

    systems::RigSystem sys(&world);
    auto* m1 = world.createEntity("rm1");
    addComp<components::RigModule>(m1);
    sys.installModule("rig5", "rm1");
    assertTrue(sys.getInstalledCount("rig5") == 1, "One module before remove");
    assertTrue(sys.removeModule("rig5", "rm1"), "Remove succeeds");
    assertTrue(sys.getInstalledCount("rig5") == 0, "Zero modules after remove");
}


void run_rig_system_tests() {
    testRigLoadoutDefaults();
    testRigInstallModule();
    testRigModuleFull();
    testRigDerivedStats();
    testRigRemoveModule();
}
