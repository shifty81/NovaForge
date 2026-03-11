// Tests for: RemoteRepairSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/remote_repair_system.h"

using namespace atlas;

using RT = components::RemoteRepairState::RepairType;

// ==================== RemoteRepairSystem Tests ====================

static void testRepInit() {
    std::cout << "\n=== RemoteRepair: Init ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    assertTrue(sys.initialize("logi1"), "Init succeeds");
    assertTrue(sys.getModuleCount("logi1") == 0, "Zero modules initially");
    assertTrue(sys.getActiveModuleCount("logi1") == 0, "Zero active initially");
    assertTrue(approxEqual(sys.getTotalShieldRepaired("logi1"), 0.0f), "Zero shield repaired");
    assertTrue(approxEqual(sys.getTotalArmorRepaired("logi1"), 0.0f), "Zero armor repaired");
    assertTrue(approxEqual(sys.getTotalHullRepaired("logi1"), 0.0f), "Zero hull repaired");
    assertTrue(sys.getTotalCycles("logi1") == 0, "Zero cycles");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testRepAddModule() {
    std::cout << "\n=== RemoteRepair: AddModule ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");

    assertTrue(sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f),
               "Add shield remote rep");
    assertTrue(sys.getModuleCount("logi1") == 1, "One module");

    assertTrue(sys.addModule("logi1", "rar1", RT::Armor, 300.0f, 6000.0f, 5.0f),
               "Add armor remote rep");
    assertTrue(sys.getModuleCount("logi1") == 2, "Two modules");
}

static void testRepAddModuleValidation() {
    std::cout << "\n=== RemoteRepair: AddModuleValidation ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");

    assertTrue(!sys.addModule("logi1", "", RT::Shield, 200.0f, 7500.0f, 5.0f),
               "Empty module_id rejected");
    assertTrue(!sys.addModule("logi1", "srr1", RT::Shield, 0.0f, 7500.0f, 5.0f),
               "Zero rep_amount rejected");
    assertTrue(!sys.addModule("logi1", "srr1", RT::Shield, -100.0f, 7500.0f, 5.0f),
               "Negative rep_amount rejected");
    assertTrue(!sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 0.0f, 5.0f),
               "Zero range rejected");
    assertTrue(!sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 0.0f),
               "Zero cycle_time rejected");

    // Duplicate
    sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f);
    assertTrue(!sys.addModule("logi1", "srr1", RT::Armor, 300.0f, 6000.0f, 5.0f),
               "Duplicate module_id rejected");
}

static void testRepRemoveModule() {
    std::cout << "\n=== RemoteRepair: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");
    sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f);
    sys.addModule("logi1", "rar1", RT::Armor,  300.0f, 6000.0f, 5.0f);

    assertTrue(sys.removeModule("logi1", "srr1"), "Remove first module");
    assertTrue(sys.getModuleCount("logi1") == 1, "One remaining");
    assertTrue(!sys.removeModule("logi1", "nonexistent"), "Remove unknown fails");
    assertTrue(sys.removeModule("logi1", "rar1"), "Remove last module");
    assertTrue(sys.getModuleCount("logi1") == 0, "Zero modules");
}

static void testRepActivate() {
    std::cout << "\n=== RemoteRepair: Activate ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");
    sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f);

    assertTrue(sys.activateModule("logi1", "srr1", "target1"), "Activate module");
    assertTrue(sys.isModuleActive("logi1", "srr1"), "Module is active");
    assertTrue(sys.getActiveModuleCount("logi1") == 1, "One active module");
    assertTrue(sys.getTargetId("logi1", "srr1") == "target1", "Target ID stored");

    // Double activation rejected
    assertTrue(!sys.activateModule("logi1", "srr1", "target2"), "Double activation rejected");

    // Empty target rejected
    sys.addModule("logi1", "srr2", RT::Shield, 200.0f, 7500.0f, 5.0f);
    assertTrue(!sys.activateModule("logi1", "srr2", ""), "Empty target rejected");
}

static void testRepDeactivate() {
    std::cout << "\n=== RemoteRepair: Deactivate ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");
    sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f);
    sys.activateModule("logi1", "srr1", "target1");

    assertTrue(sys.deactivateModule("logi1", "srr1"), "Deactivate module");
    assertTrue(!sys.isModuleActive("logi1", "srr1"), "Module inactive");
    assertTrue(sys.getActiveModuleCount("logi1") == 0, "Zero active");
    assertTrue(sys.getTargetId("logi1", "srr1").empty(), "Target cleared on deactivate");

    // Double deactivation rejected
    assertTrue(!sys.deactivateModule("logi1", "srr1"), "Double deactivation rejected");
}

static void testRepCycleRepairs() {
    std::cout << "\n=== RemoteRepair: CycleRepairs ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");
    sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f);
    sys.activateModule("logi1", "srr1", "target1");

    // After one full cycle, shield should be repaired
    sys.update(5.0f);
    assertTrue(sys.getTotalCycles("logi1") == 1, "One cycle completed");
    assertTrue(approxEqual(sys.getTotalShieldRepaired("logi1"), 200.0f, 1.0f),
               "200 shield repaired after one cycle");
    assertTrue(sys.getModuleReps("logi1", "srr1") == 1, "Module shows 1 rep");

    // After second cycle
    sys.update(5.0f);
    assertTrue(sys.getTotalCycles("logi1") == 2, "Two cycles completed");
    assertTrue(approxEqual(sys.getTotalShieldRepaired("logi1"), 400.0f, 1.0f),
               "400 shield repaired after two cycles");
}

static void testRepMultiType() {
    std::cout << "\n=== RemoteRepair: MultiType ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");
    sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f);
    sys.addModule("logi1", "rar1", RT::Armor,  300.0f, 6000.0f, 5.0f);
    sys.addModule("logi1", "hrr1", RT::Hull,   150.0f, 5000.0f, 5.0f);
    sys.activateModule("logi1", "srr1", "t1");
    sys.activateModule("logi1", "rar1", "t2");
    sys.activateModule("logi1", "hrr1", "t3");

    sys.update(5.0f);
    assertTrue(sys.getTotalCycles("logi1") == 3, "Three cycles (one per module)");
    assertTrue(approxEqual(sys.getTotalShieldRepaired("logi1"), 200.0f, 1.0f), "Shield repaired");
    assertTrue(approxEqual(sys.getTotalArmorRepaired("logi1"), 300.0f, 1.0f), "Armor repaired");
    assertTrue(approxEqual(sys.getTotalHullRepaired("logi1"), 150.0f, 1.0f), "Hull repaired");
}

static void testRepInactiveModuleDoesNotRep() {
    std::cout << "\n=== RemoteRepair: InactiveNoRep ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");
    sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f);
    // Do NOT activate the module

    sys.update(10.0f);
    assertTrue(approxEqual(sys.getTotalShieldRepaired("logi1"), 0.0f), "No repair when inactive");
    assertTrue(sys.getTotalCycles("logi1") == 0, "No cycles when inactive");
}

static void testRepSetRepAmount() {
    std::cout << "\n=== RemoteRepair: SetRepAmount ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");
    sys.addModule("logi1", "srr1", RT::Shield, 200.0f, 7500.0f, 5.0f);

    assertTrue(sys.setRepAmount("logi1", "srr1", 500.0f), "Set rep amount");
    sys.activateModule("logi1", "srr1", "target1");
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getTotalShieldRepaired("logi1"), 500.0f, 1.0f),
               "New rep amount used after cycle");

    assertTrue(!sys.setRepAmount("logi1", "srr1", 0.0f), "Zero amount rejected");
    assertTrue(!sys.setRepAmount("logi1", "srr1", -100.0f), "Negative amount rejected");
    assertTrue(!sys.setRepAmount("logi1", "nonexistent", 200.0f), "Unknown module rejected");
}

static void testRepCapacity() {
    std::cout << "\n=== RemoteRepair: Capacity ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);
    world.createEntity("logi1");
    sys.initialize("logi1");
    auto* comp = world.getEntity("logi1")->getComponent<components::RemoteRepairState>();
    comp->max_modules = 2;

    assertTrue(sys.addModule("logi1", "m1", RT::Shield, 200.0f, 7500.0f, 5.0f), "First ok");
    assertTrue(sys.addModule("logi1", "m2", RT::Armor,  300.0f, 6000.0f, 5.0f), "Second ok");
    assertTrue(!sys.addModule("logi1", "m3", RT::Hull,  150.0f, 5000.0f, 5.0f), "Third rejected at cap");
    assertTrue(sys.getModuleCount("logi1") == 2, "Still at 2");
}

static void testRepMissing() {
    std::cout << "\n=== RemoteRepair: Missing ===" << std::endl;
    ecs::World world;
    systems::RemoteRepairSystem sys(&world);

    assertTrue(!sys.addModule("nonexistent", "m1", RT::Shield, 200.0f, 7500.0f, 5.0f),
               "Add fails on missing");
    assertTrue(!sys.removeModule("nonexistent", "m1"), "Remove fails on missing");
    assertTrue(!sys.activateModule("nonexistent", "m1", "t1"), "Activate fails on missing");
    assertTrue(!sys.deactivateModule("nonexistent", "m1"), "Deactivate fails on missing");
    assertTrue(!sys.setRepAmount("nonexistent", "m1", 200.0f), "SetRepAmount fails on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "Zero count on missing");
    assertTrue(sys.getActiveModuleCount("nonexistent") == 0, "Zero active on missing");
    assertTrue(!sys.isModuleActive("nonexistent", "m1"), "Not active on missing");
    assertTrue(approxEqual(sys.getTotalShieldRepaired("nonexistent"), 0.0f), "Zero shield on missing");
    assertTrue(approxEqual(sys.getTotalArmorRepaired("nonexistent"), 0.0f), "Zero armor on missing");
    assertTrue(approxEqual(sys.getTotalHullRepaired("nonexistent"), 0.0f), "Zero hull on missing");
    assertTrue(sys.getTotalCycles("nonexistent") == 0, "Zero cycles on missing");
}

void run_remote_repair_system_tests() {
    testRepInit();
    testRepAddModule();
    testRepAddModuleValidation();
    testRepRemoveModule();
    testRepActivate();
    testRepDeactivate();
    testRepCycleRepairs();
    testRepMultiType();
    testRepInactiveModuleDoesNotRep();
    testRepSetRepAmount();
    testRepCapacity();
    testRepMissing();
}
