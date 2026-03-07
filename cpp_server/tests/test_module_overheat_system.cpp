// Tests for: Module Overheat System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/module_overheat_system.h"

using namespace atlas;

// ==================== Module Overheat System Tests ====================

static void testModuleOverheatCreate() {
    std::cout << "\n=== ModuleOverheat: Create ===" << std::endl;
    ecs::World world;
    systems::ModuleOverheatSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getModuleCount("ship1") == 0, "0 modules");
    assertTrue(sys.getTotalBurnouts("ship1") == 0, "0 burnouts");
}

static void testModuleOverheatAddModule() {
    std::cout << "\n=== ModuleOverheat: AddModule ===" << std::endl;
    ecs::World world;
    systems::ModuleOverheatSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.addModule("ship1", "gun_01", 10.0f, 3.0f, 80.0f), "Add gun_01");
    assertTrue(sys.addModule("ship1", "gun_02", 10.0f, 3.0f, 80.0f), "Add gun_02");
    assertTrue(sys.getModuleCount("ship1") == 2, "2 modules");

    // Duplicate rejected
    assertTrue(!sys.addModule("ship1", "gun_01", 5.0f, 2.0f, 70.0f), "Duplicate rejected");

    // Fill to max (8)
    for (int i = 3; i <= 8; i++) {
        assertTrue(sys.addModule("ship1", "mod_" + std::to_string(i), 5.0f, 2.0f, 80.0f),
                   "Add module " + std::to_string(i));
    }
    assertTrue(sys.getModuleCount("ship1") == 8, "8 modules");
    assertTrue(!sys.addModule("ship1", "mod_9", 5.0f, 2.0f, 80.0f), "9th module rejected");
}

static void testModuleOverheatCycleAndDissipate() {
    std::cout << "\n=== ModuleOverheat: CycleAndDissipate ===" << std::endl;
    ecs::World world;
    systems::ModuleOverheatSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addModule("ship1", "gun_01", 10.0f, 5.0f, 80.0f);

    // Overheat + cycle → heat goes up
    sys.setOverheating("ship1", "gun_01", true);
    sys.cycleModule("ship1", "gun_01");
    assertTrue(sys.getHeatLevel("ship1", "gun_01") == 10.0f, "Heat 10 after 1 cycle");

    sys.cycleModule("ship1", "gun_01");
    assertTrue(sys.getHeatLevel("ship1", "gun_01") == 20.0f, "Heat 20 after 2 cycles");

    // Disengage overheat → heat dissipates over time (5/s)
    sys.setOverheating("ship1", "gun_01", false);
    sys.update(2.0f);  // 2s × 5/s = 10 dissipated → 20 - 10 = 10
    assertTrue(approxEqual(sys.getHeatLevel("ship1", "gun_01"), 10.0f), "Heat 10 after 2s dissipation");

    sys.update(2.0f);  // another 10 → 0
    assertTrue(approxEqual(sys.getHeatLevel("ship1", "gun_01"), 0.0f), "Heat 0 after full dissipation");
}

static void testModuleOverheatBurnout() {
    std::cout << "\n=== ModuleOverheat: Burnout ===" << std::endl;
    ecs::World world;
    systems::ModuleOverheatSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Module with 20 heat/cycle, high damage threshold to focus on burnout
    sys.addModule("ship1", "gun_01", 20.0f, 0.0f, 90.0f);
    sys.setOverheating("ship1", "gun_01", true);

    // Cycle 5 times → 100 heat
    for (int i = 0; i < 5; i++) sys.cycleModule("ship1", "gun_01");
    assertTrue(sys.getHeatLevel("ship1", "gun_01") == 100.0f, "Heat at 100");

    // Update to trigger burnout
    sys.update(0.1f);
    assertTrue(sys.isBurnedOut("ship1", "gun_01"), "Module burned out");
    assertTrue(!sys.isOverheating("ship1", "gun_01"), "Overheating disabled on burnout");
    assertTrue(sys.getTotalBurnouts("ship1") == 1, "1 burnout");

    // Can't cycle or overheat a burned-out module
    assertTrue(!sys.cycleModule("ship1", "gun_01"), "Cycle rejected on burned out");
    assertTrue(!sys.setOverheating("ship1", "gun_01", true), "Overheat rejected on burned out");
}

static void testModuleOverheatDamage() {
    std::cout << "\n=== ModuleOverheat: Damage ===" << std::endl;
    ecs::World world;
    systems::ModuleOverheatSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Module with threshold at 50
    sys.addModule("ship1", "rep_01", 15.0f, 0.0f, 50.0f);
    sys.setOverheating("ship1", "rep_01", true);

    // 4 cycles → 60 heat (above 50 threshold)
    for (int i = 0; i < 4; i++) sys.cycleModule("ship1", "rep_01");
    assertTrue(sys.getHeatLevel("ship1", "rep_01") == 60.0f, "Heat 60");

    // Update — excess is 10, damage += 10 * 0.1 * dt
    sys.update(1.0f);
    float dmg = sys.getDamageAccumulated("ship1", "rep_01");
    assertTrue(dmg > 0.0f, "Damage accumulated above threshold");
}

static void testModuleOverheatMissing() {
    std::cout << "\n=== ModuleOverheat: Missing ===" << std::endl;
    ecs::World world;
    systems::ModuleOverheatSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addModule("nonexistent", "m1", 5.0f, 2.0f, 80.0f), "AddModule fails on missing");
    assertTrue(!sys.setOverheating("nonexistent", "m1", true), "SetOverheating fails on missing");
    assertTrue(!sys.cycleModule("nonexistent", "m1"), "CycleModule fails on missing");
    assertTrue(sys.getModuleCount("nonexistent") == 0, "0 modules on missing");
    assertTrue(sys.getHeatLevel("nonexistent", "m1") == 0.0f, "0 heat on missing");
    assertTrue(!sys.isBurnedOut("nonexistent", "m1"), "Not burned out on missing");
    assertTrue(!sys.isOverheating("nonexistent", "m1"), "Not overheating on missing");
    assertTrue(sys.getTotalBurnouts("nonexistent") == 0, "0 burnouts on missing");
    assertTrue(sys.getDamageAccumulated("nonexistent", "m1") == 0.0f, "0 damage on missing");
}

void run_module_overheat_system_tests() {
    testModuleOverheatCreate();
    testModuleOverheatAddModule();
    testModuleOverheatCycleAndDissipate();
    testModuleOverheatBurnout();
    testModuleOverheatDamage();
    testModuleOverheatMissing();
}
