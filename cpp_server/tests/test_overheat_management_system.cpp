// Tests for: OverheatManagementSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/overheat_management_system.h"

using namespace atlas;

// ==================== OverheatManagementSystem Tests ====================

static void testOverheatCreate() {
    std::cout << "\n=== OverheatManagement: Create ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getModuleCount("ship1") == 0, "Zero modules");
    assertTrue(approxEqual(sys.getGlobalHeat("ship1"), 0.0f), "Zero global heat");
    assertTrue(approxEqual(sys.getDissipationRate("ship1"), 2.0f), "Default dissipation");
    assertTrue(sys.getTotalOverheats("ship1") == 0, "Zero overheats");
    assertTrue(sys.getTotalBurnouts("ship1") == 0, "Zero burnouts");
}

static void testOverheatInvalidInit() {
    std::cout << "\n=== OverheatManagement: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    assertTrue(!sys.initialize("missing"), "Missing entity fails");
}

static void testOverheatAddModule() {
    std::cout << "\n=== OverheatManagement: AddModule ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.addModule("ship1", "gun1", 10.0f, 100.0f), "Add gun1");
    assertTrue(sys.addModule("ship1", "gun2", 15.0f, 80.0f), "Add gun2");
    assertTrue(sys.getModuleCount("ship1") == 2, "2 modules");

    assertTrue(!sys.addModule("ship1", "gun1", 5.0f, 50.0f), "Duplicate rejected");
    assertTrue(!sys.addModule("ship1", "", 10.0f, 100.0f), "Empty ID rejected");
    assertTrue(!sys.addModule("ship1", "x", 0.0f, 100.0f), "Zero heat gen rejected");
    assertTrue(!sys.addModule("ship1", "x", 10.0f, 0.0f), "Zero max heat rejected");
    assertTrue(!sys.addModule("nonexistent", "x", 10.0f, 100.0f), "Missing entity rejected");
}

static void testOverheatRemoveModule() {
    std::cout << "\n=== OverheatManagement: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "gun1", 10.0f, 100.0f);
    sys.addModule("ship1", "gun2", 15.0f, 80.0f);

    assertTrue(sys.removeModule("ship1", "gun1"), "Remove gun1 succeeds");
    assertTrue(sys.getModuleCount("ship1") == 1, "1 module remaining");
    assertTrue(!sys.removeModule("ship1", "gun1"), "Double remove fails");
    assertTrue(!sys.removeModule("ship1", "nonexistent"), "Remove nonexistent fails");
}

static void testOverheatActivation() {
    std::cout << "\n=== OverheatManagement: Activation ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "gun1", 25.0f, 100.0f);

    assertTrue(sys.activateModule("ship1", "gun1"), "Activate gun1");
    float heat = sys.getModuleHeat("ship1", "gun1");
    assertTrue(heat > 24.0f && heat < 26.0f, "Heat increased by generation");

    assertTrue(sys.activateModule("ship1", "gun1"), "Activate again adds more heat");
    heat = sys.getModuleHeat("ship1", "gun1");
    assertTrue(heat > 49.0f && heat < 51.0f, "Heat doubled");

    assertTrue(!sys.activateModule("ship1", "nonexistent"), "Activate nonexistent fails");
    assertTrue(!sys.activateModule("missing", "gun1"), "Missing entity fails");
}

static void testOverheatInvalidOperations() {
    std::cout << "\n=== OverheatManagement: InvalidOperations ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(!sys.setDissipationRate("ship1", -1.0f), "Negative rate rejected");
    assertTrue(sys.setDissipationRate("ship1", 0.0f), "Zero rate accepted");
    assertTrue(!sys.setDissipationRate("missing", 1.0f), "Missing entity rejected");
    assertTrue(!sys.resetModule("ship1", "nonexistent"), "Reset nonexistent fails");
    assertTrue(!sys.resetModule("missing", "x"), "Reset missing entity fails");
}

static void testOverheatUpdateDissipation() {
    std::cout << "\n=== OverheatManagement: UpdateDissipation ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "gun1", 10.0f, 100.0f);
    sys.activateModule("ship1", "gun1"); // heat = 10

    // Dissipation = 2.0/s, after 3s: 10 - 6 = 4
    sys.update(3.0f);
    float heat = sys.getModuleHeat("ship1", "gun1");
    assertTrue(heat > 3.5f && heat < 4.5f, "Heat dissipated to ~4");
}

static void testOverheatThresholds() {
    std::cout << "\n=== OverheatManagement: Thresholds ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    // damage_threshold defaults to 80, max_heat = 100
    sys.addModule("ship1", "gun1", 85.0f, 100.0f);

    sys.activateModule("ship1", "gun1"); // heat = 85 (>= 80 threshold)
    sys.update(0.0f); // trigger threshold check
    assertTrue(sys.isOverheated("ship1", "gun1"), "Module overheated at 85");
    assertTrue(sys.getTotalOverheats("ship1") == 1, "1 overheat counted");
    assertTrue(!sys.isBurnedOut("ship1", "gun1"), "Not burned out yet");

    // Push to max
    sys.setDissipationRate("ship1", 0.0f);
    sys.activateModule("ship1", "gun1"); // heat capped at 100
    sys.update(0.0f);
    assertTrue(sys.isBurnedOut("ship1", "gun1"), "Module burned out at 100");
    assertTrue(sys.getTotalBurnouts("ship1") == 1, "1 burnout counted");

    // Can't activate burned out module
    assertTrue(!sys.activateModule("ship1", "gun1"), "Activate burned out rejected");
    // Can't reset burned out module
    assertTrue(!sys.resetModule("ship1", "gun1"), "Reset burned out rejected");
}

static void testOverheatMultipleModules() {
    std::cout << "\n=== OverheatManagement: MultipleModules ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    for (int i = 0; i < 4; i++) {
        std::string id = "mod_" + std::to_string(i);
        sys.addModule("ship1", id, 10.0f, 100.0f);
        sys.activateModule("ship1", id); // each gets 10 heat
    }
    assertTrue(sys.getModuleCount("ship1") == 4, "4 modules");

    // Global heat = avg(10, 10, 10, 10) = 10
    sys.update(0.0f);
    float global = sys.getGlobalHeat("ship1");
    assertTrue(global > 9.0f && global < 11.0f, "Global heat ~10");
}

static void testOverheatBoundary() {
    std::cout << "\n=== OverheatManagement: Boundary ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "gun1", 200.0f, 100.0f);

    // Activation generates 200 but clamps to max_heat 100
    sys.activateModule("ship1", "gun1");
    float heat = sys.getModuleHeat("ship1", "gun1");
    assertTrue(heat > 99.0f && heat < 101.0f, "Heat clamped to 100");
}

static void testOverheatMissing() {
    std::cout << "\n=== OverheatManagement: Missing ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    assertTrue(sys.getModuleCount("x") == 0, "Default modules on missing");
    assertTrue(approxEqual(sys.getModuleHeat("x", "m"), 0.0f), "Default heat on missing");
    assertTrue(!sys.isOverheated("x", "m"), "Default overheated on missing");
    assertTrue(!sys.isBurnedOut("x", "m"), "Default burned out on missing");
    assertTrue(approxEqual(sys.getGlobalHeat("x"), 0.0f), "Default global heat on missing");
    assertTrue(approxEqual(sys.getDissipationRate("x"), 0.0f), "Default dissipation on missing");
    assertTrue(sys.getTotalOverheats("x") == 0, "Default overheats on missing");
    assertTrue(sys.getTotalBurnouts("x") == 0, "Default burnouts on missing");
}

static void testOverheatReset() {
    std::cout << "\n=== OverheatManagement: Reset ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "gun1", 50.0f, 100.0f);
    sys.activateModule("ship1", "gun1"); // heat = 50

    assertTrue(sys.resetModule("ship1", "gun1"), "Reset succeeds");
    assertTrue(approxEqual(sys.getModuleHeat("ship1", "gun1"), 0.0f), "Heat reset to 0");
    assertTrue(!sys.isOverheated("ship1", "gun1"), "No longer overheated");
}

static void testOverheatCombined() {
    std::cout << "\n=== OverheatManagement: Combined ===" << std::endl;
    ecs::World world;
    systems::OverheatManagementSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "gun1", 20.0f, 100.0f);
    sys.addModule("ship1", "gun2", 30.0f, 100.0f);
    sys.setDissipationRate("ship1", 5.0f);

    sys.activateModule("ship1", "gun1"); // 20 heat
    sys.activateModule("ship1", "gun2"); // 30 heat

    // After 2s: gun1 = 20 - 10 = 10, gun2 = 30 - 10 = 20
    sys.update(2.0f);
    float h1 = sys.getModuleHeat("ship1", "gun1");
    float h2 = sys.getModuleHeat("ship1", "gun2");
    assertTrue(h1 > 9.0f && h1 < 11.0f, "gun1 heat ~10");
    assertTrue(h2 > 19.0f && h2 < 21.0f, "gun2 heat ~20");

    // Global = (10 + 20) / 2 = 15
    float global = sys.getGlobalHeat("ship1");
    assertTrue(global > 14.0f && global < 16.0f, "Global heat ~15");
}

void run_overheat_management_system_tests() {
    testOverheatCreate();
    testOverheatInvalidInit();
    testOverheatAddModule();
    testOverheatRemoveModule();
    testOverheatActivation();
    testOverheatInvalidOperations();
    testOverheatUpdateDissipation();
    testOverheatThresholds();
    testOverheatMultipleModules();
    testOverheatBoundary();
    testOverheatMissing();
    testOverheatReset();
    testOverheatCombined();
}
