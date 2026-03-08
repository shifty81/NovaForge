// Tests for: CapacitorWarfareSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/capacitor_warfare_system.h"

using namespace atlas;

// ==================== CapacitorWarfareSystem Tests ====================

static void testCapWarfareCreate() {
    std::cout << "\n=== CapacitorWarfare: Create ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getModuleCount("ship1") == 0, "Zero modules");
    assertTrue(sys.getActiveModuleCount("ship1") == 0, "Zero active");
    assertTrue(approxEqual(sys.getDrainResistance("ship1"), 0.0f), "Zero drain resistance");
    assertTrue(approxEqual(sys.getTotalEnergyDrained("ship1"), 0.0f), "Zero drained");
    assertTrue(approxEqual(sys.getTotalEnergyReceived("ship1"), 0.0f), "Zero received");
    assertTrue(sys.getTotalCyclesCompleted("ship1") == 0, "Zero cycles");
    assertTrue(sys.getTotalTargetsCapped("ship1") == 0, "Zero targets capped");
}

static void testCapWarfareInvalidInit() {
    std::cout << "\n=== CapacitorWarfare: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    assertTrue(!sys.initialize("missing"), "Missing entity fails");
}

static void testCapWarfareAddModule() {
    std::cout << "\n=== CapacitorWarfare: AddModule ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.addModule("ship1", "neut1", "neutralizer", 10.0f, 10.0f, 12.0f),
               "Add neutralizer");
    assertTrue(sys.addModule("ship1", "nos1", "nosferatu", 8.0f, 8.0f, 10.0f),
               "Add nosferatu");
    assertTrue(sys.getModuleCount("ship1") == 2, "2 modules");

    // Duplicate rejected
    assertTrue(!sys.addModule("ship1", "neut1", "neutralizer", 10.0f, 10.0f, 12.0f),
               "Duplicate module rejected");

    // Invalid inputs
    assertTrue(!sys.addModule("ship1", "", "neutralizer", 10.0f, 10.0f, 12.0f),
               "Empty ID rejected");
    assertTrue(!sys.addModule("ship1", "x", "invalid_type", 10.0f, 10.0f, 12.0f),
               "Invalid type rejected");
    assertTrue(!sys.addModule("ship1", "x", "neutralizer", 0.0f, 10.0f, 12.0f),
               "Zero drain rejected");
    assertTrue(!sys.addModule("ship1", "x", "neutralizer", 10.0f, 0.0f, 12.0f),
               "Zero range rejected");
    assertTrue(!sys.addModule("ship1", "x", "neutralizer", 10.0f, 10.0f, 0.0f),
               "Zero cycle time rejected");
    assertTrue(!sys.addModule("nonexistent", "x", "neutralizer", 10.0f, 10.0f, 12.0f),
               "Missing entity rejected");
}

static void testCapWarfareRemoveModule() {
    std::cout << "\n=== CapacitorWarfare: RemoveModule ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "neut1", "neutralizer", 10.0f, 10.0f, 12.0f);
    sys.addModule("ship1", "nos1", "nosferatu", 8.0f, 8.0f, 10.0f);

    assertTrue(sys.removeModule("ship1", "neut1"), "Remove neut1 succeeds");
    assertTrue(sys.getModuleCount("ship1") == 1, "1 module remaining");
    assertTrue(!sys.removeModule("ship1", "neut1"), "Double remove fails");
    assertTrue(!sys.removeModule("ship1", "nonexistent"), "Remove nonexistent fails");
}

static void testCapWarfareActivateDeactivate() {
    std::cout << "\n=== CapacitorWarfare: ActivateDeactivate ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "neut1", "neutralizer", 10.0f, 10.0f, 12.0f);

    assertTrue(sys.activateModule("ship1", "neut1", "enemy_1"), "Activate neut1");
    assertTrue(sys.isModuleActive("ship1", "neut1"), "Module is active");
    assertTrue(sys.getActiveModuleCount("ship1") == 1, "1 active module");

    // Can't activate twice
    assertTrue(!sys.activateModule("ship1", "neut1", "enemy_2"), "Double activate rejected");

    assertTrue(sys.deactivateModule("ship1", "neut1"), "Deactivate neut1");
    assertTrue(!sys.isModuleActive("ship1", "neut1"), "Module no longer active");
    assertTrue(sys.getActiveModuleCount("ship1") == 0, "0 active modules");

    // Can't deactivate twice
    assertTrue(!sys.deactivateModule("ship1", "neut1"), "Double deactivate rejected");

    // Invalid activation
    assertTrue(!sys.activateModule("ship1", "neut1", ""), "Empty target rejected");
    assertTrue(!sys.activateModule("ship1", "nonexistent", "enemy_1"), "Missing module rejected");
}

static void testCapWarfareNeutralizerCycle() {
    std::cout << "\n=== CapacitorWarfare: NeutralizerCycle ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "neut1", "neutralizer", 10.0f, 10.0f, 12.0f);
    sys.activateModule("ship1", "neut1", "enemy_1");

    // Run for one full cycle (12s)
    sys.update(12.0f);
    assertTrue(sys.getTotalCyclesCompleted("ship1") == 1, "1 cycle completed");

    float drained = sys.getTotalEnergyDrained("ship1");
    // drain_rate(10) * cycle_time(12) = 120 GJ
    assertTrue(drained > 119.0f && drained < 121.0f, "~120 GJ drained");

    // Neutralizer gives no energy back
    assertTrue(approxEqual(sys.getTotalEnergyReceived("ship1"), 0.0f),
               "Neut gives 0 energy back");
}

static void testCapWarfareNosferatuCycle() {
    std::cout << "\n=== CapacitorWarfare: NosferatuCycle ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "nos1", "nosferatu", 10.0f, 10.0f, 10.0f);
    sys.activateModule("ship1", "nos1", "enemy_1");

    // Run for one full cycle (10s)
    sys.update(10.0f);
    assertTrue(sys.getTotalCyclesCompleted("ship1") == 1, "1 cycle completed");

    float drained = sys.getTotalEnergyDrained("ship1");
    assertTrue(drained > 99.0f && drained < 101.0f, "~100 GJ drained");

    float received = sys.getTotalEnergyReceived("ship1");
    // nos receives drain * (1 - resistance), resistance is 0
    assertTrue(received > 99.0f && received < 101.0f, "~100 GJ received (no resistance)");
}

static void testCapWarfareDrainResistance() {
    std::cout << "\n=== CapacitorWarfare: DrainResistance ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(sys.setDrainResistance("ship1", 0.5f), "Set 50% resistance");
    assertTrue(approxEqual(sys.getDrainResistance("ship1"), 0.5f), "Resistance is 0.5");

    // With 50% resistance, nosferatu receives only 50% energy
    sys.addModule("ship1", "nos1", "nosferatu", 10.0f, 10.0f, 10.0f);
    sys.activateModule("ship1", "nos1", "enemy_1");
    sys.update(10.0f);

    float received = sys.getTotalEnergyReceived("ship1");
    assertTrue(received > 49.0f && received < 51.0f, "~50 GJ received with 50% resistance");

    // Invalid resistance
    assertTrue(!sys.setDrainResistance("ship1", -0.1f), "Negative resistance rejected");
    assertTrue(!sys.setDrainResistance("ship1", 1.1f), "Resistance > 1 rejected");
    assertTrue(!sys.setDrainResistance("nonexistent", 0.5f), "Missing entity rejected");
}

static void testCapWarfareMultipleCycles() {
    std::cout << "\n=== CapacitorWarfare: MultipleCycles ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "neut1", "neutralizer", 10.0f, 10.0f, 5.0f);
    sys.activateModule("ship1", "neut1", "enemy_1");

    // 3 cycles at 5s each = 15s
    sys.update(15.0f);
    assertTrue(sys.getTotalCyclesCompleted("ship1") == 3, "3 cycles after 15s");
}

static void testCapWarfareModuleCycleProgress() {
    std::cout << "\n=== CapacitorWarfare: ModuleCycleProgress ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.addModule("ship1", "neut1", "neutralizer", 10.0f, 10.0f, 10.0f);
    sys.activateModule("ship1", "neut1", "enemy_1");

    sys.update(5.0f); // 50% through cycle
    float progress = sys.getModuleCycleProgress("ship1", "neut1");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% cycle progress");

    assertTrue(approxEqual(sys.getModuleCycleProgress("ship1", "nonexistent"), 0.0f),
               "Unknown module returns 0");
}

static void testCapWarfareUpdate() {
    std::cout << "\n=== CapacitorWarfare: Update ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testCapWarfareMissing() {
    std::cout << "\n=== CapacitorWarfare: Missing ===" << std::endl;
    ecs::World world;
    systems::CapacitorWarfareSystem sys(&world);
    assertTrue(sys.getModuleCount("x") == 0, "Default modules on missing");
    assertTrue(sys.getActiveModuleCount("x") == 0, "Default active on missing");
    assertTrue(approxEqual(sys.getDrainResistance("x"), 0.0f), "Default resistance on missing");
    assertTrue(approxEqual(sys.getTotalEnergyDrained("x"), 0.0f), "Default drained on missing");
    assertTrue(approxEqual(sys.getTotalEnergyReceived("x"), 0.0f), "Default received on missing");
    assertTrue(approxEqual(sys.getTotalEnergyLost("x"), 0.0f), "Default lost on missing");
    assertTrue(sys.getTotalCyclesCompleted("x") == 0, "Default cycles on missing");
    assertTrue(sys.getTotalTargetsCapped("x") == 0, "Default targets on missing");
    assertTrue(!sys.isModuleActive("x", "m"), "Default module active on missing");
    assertTrue(approxEqual(sys.getModuleCycleProgress("x", "m"), 0.0f), "Default progress on missing");
}

void run_capacitor_warfare_system_tests() {
    testCapWarfareCreate();
    testCapWarfareInvalidInit();
    testCapWarfareAddModule();
    testCapWarfareRemoveModule();
    testCapWarfareActivateDeactivate();
    testCapWarfareNeutralizerCycle();
    testCapWarfareNosferatuCycle();
    testCapWarfareDrainResistance();
    testCapWarfareMultipleCycles();
    testCapWarfareModuleCycleProgress();
    testCapWarfareUpdate();
    testCapWarfareMissing();
}
