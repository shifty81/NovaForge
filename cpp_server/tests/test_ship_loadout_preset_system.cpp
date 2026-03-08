// Tests for: Ship Loadout Preset System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/ship_loadout_preset_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Ship Loadout Preset System Tests ====================

static void testShipLoadoutPresetCreate() {
    std::cout << "\n=== ShipLoadoutPreset: Create ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    assertTrue(sys.initialize("slp1"), "Init succeeds");
    assertTrue(sys.getPresetCount("slp1") == 0, "No presets initially");
    assertTrue(sys.getTotalPresetsSaved("slp1") == 0, "0 total presets saved");
}

static void testShipLoadoutPresetSave() {
    std::cout << "\n=== ShipLoadoutPreset: Save ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");
    assertTrue(sys.savePreset("slp1", "PvP Fit", "Rifter"), "Save PvP preset");
    assertTrue(sys.savePreset("slp1", "Mining Fit", "Venture"), "Save Mining preset");
    assertTrue(sys.getPresetCount("slp1") == 2, "2 presets saved");
    assertTrue(sys.hasPreset("slp1", "PvP Fit"), "Has PvP Fit");
    assertTrue(sys.hasPreset("slp1", "Mining Fit"), "Has Mining Fit");
    assertTrue(!sys.hasPreset("slp1", "Ratting Fit"), "Does not have Ratting Fit");
    assertTrue(sys.getTotalPresetsSaved("slp1") == 2, "2 total presets saved");
}

static void testShipLoadoutPresetDuplicate() {
    std::cout << "\n=== ShipLoadoutPreset: Duplicate ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");
    sys.savePreset("slp1", "PvP Fit", "Rifter");
    assertTrue(!sys.savePreset("slp1", "PvP Fit", "Hurricane"), "Duplicate name rejected");
    assertTrue(sys.getPresetCount("slp1") == 1, "Still 1 preset");
}

static void testShipLoadoutPresetAddModules() {
    std::cout << "\n=== ShipLoadoutPreset: AddModules ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");
    sys.savePreset("slp1", "PvP Fit", "Rifter");
    assertTrue(sys.addModuleToPreset("slp1", "PvP Fit", "200mm AutoCannon II", "high_1"), "Add autocannon");
    assertTrue(sys.addModuleToPreset("slp1", "PvP Fit", "Warp Scrambler II", "mid_1"), "Add scram");
    assertTrue(sys.addModuleToPreset("slp1", "PvP Fit", "Damage Control II", "low_1"), "Add DC");
    assertTrue(sys.getModuleCount("slp1", "PvP Fit") == 3, "3 modules in PvP Fit");
    assertTrue(!sys.addModuleToPreset("slp1", "Nonexistent", "Module", "high_1"), "Add to missing preset fails");
}

static void testShipLoadoutPresetShipType() {
    std::cout << "\n=== ShipLoadoutPreset: ShipType ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");
    sys.savePreset("slp1", "PvP Fit", "Rifter");
    assertTrue(sys.getPresetShipType("slp1", "PvP Fit") == "Rifter", "Ship type is Rifter");
    assertTrue(sys.getPresetShipType("slp1", "Nonexistent") == "", "Empty for missing preset");
}

static void testShipLoadoutPresetRemove() {
    std::cout << "\n=== ShipLoadoutPreset: Remove ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");
    sys.savePreset("slp1", "PvP Fit", "Rifter");
    assertTrue(sys.removePreset("slp1", "PvP Fit"), "Remove preset");
    assertTrue(sys.getPresetCount("slp1") == 0, "0 presets after remove");
    assertTrue(!sys.removePreset("slp1", "PvP Fit"), "Double remove fails");
}

static void testShipLoadoutPresetRename() {
    std::cout << "\n=== ShipLoadoutPreset: Rename ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");
    sys.savePreset("slp1", "PvP Fit", "Rifter");
    assertTrue(sys.renamePreset("slp1", "PvP Fit", "Solo PvP"), "Rename succeeds");
    assertTrue(sys.hasPreset("slp1", "Solo PvP"), "Has new name");
    assertTrue(!sys.hasPreset("slp1", "PvP Fit"), "Old name removed");
    assertTrue(!sys.renamePreset("slp1", "Nonexistent", "New"), "Rename missing fails");
}

static void testShipLoadoutPresetRenameDuplicate() {
    std::cout << "\n=== ShipLoadoutPreset: RenameDuplicate ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");
    sys.savePreset("slp1", "PvP Fit", "Rifter");
    sys.savePreset("slp1", "Mining Fit", "Venture");
    assertTrue(!sys.renamePreset("slp1", "PvP Fit", "Mining Fit"), "Rename to existing name fails");
}

static void testShipLoadoutPresetMaxPresets() {
    std::cout << "\n=== ShipLoadoutPreset: MaxPresets ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");

    auto* entity = world.getEntity("slp1");
    auto* slp = entity->getComponent<components::ShipLoadoutPresets>();
    slp->max_presets = 2;

    sys.savePreset("slp1", "Fit1", "Rifter");
    sys.savePreset("slp1", "Fit2", "Venture");
    assertTrue(!sys.savePreset("slp1", "Fit3", "Hurricane"), "Max presets enforced");
    assertTrue(sys.getPresetCount("slp1") == 2, "Still 2 presets");
}

static void testShipLoadoutPresetMaxModules() {
    std::cout << "\n=== ShipLoadoutPreset: MaxModules ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");

    auto* entity = world.getEntity("slp1");
    auto* slp = entity->getComponent<components::ShipLoadoutPresets>();
    slp->max_modules_per_preset = 2;

    sys.savePreset("slp1", "Fit1", "Rifter");
    sys.addModuleToPreset("slp1", "Fit1", "Gun1", "high_1");
    sys.addModuleToPreset("slp1", "Fit1", "Gun2", "high_2");
    assertTrue(!sys.addModuleToPreset("slp1", "Fit1", "Gun3", "high_3"), "Max modules enforced");
    assertTrue(sys.getModuleCount("slp1", "Fit1") == 2, "Still 2 modules");
}

static void testShipLoadoutPresetValidation() {
    std::cout << "\n=== ShipLoadoutPreset: Validation ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    world.createEntity("slp1");
    sys.initialize("slp1");
    assertTrue(!sys.savePreset("slp1", "", "Rifter"), "Empty name rejected");
    assertTrue(!sys.savePreset("slp1", "Fit1", ""), "Empty ship type rejected");
    sys.savePreset("slp1", "Fit1", "Rifter");
    assertTrue(!sys.addModuleToPreset("slp1", "Fit1", "", "high_1"), "Empty module name rejected");
    assertTrue(!sys.addModuleToPreset("slp1", "Fit1", "Gun1", ""), "Empty slot rejected");
    assertTrue(!sys.renamePreset("slp1", "Fit1", ""), "Empty rename rejected");
}

static void testShipLoadoutPresetMissing() {
    std::cout << "\n=== ShipLoadoutPreset: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipLoadoutPresetSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.savePreset("nonexistent", "Fit", "Ship"), "Save fails on missing");
    assertTrue(!sys.addModuleToPreset("nonexistent", "Fit", "Mod", "slot"), "AddModule fails on missing");
    assertTrue(!sys.removePreset("nonexistent", "Fit"), "Remove fails on missing");
    assertTrue(!sys.renamePreset("nonexistent", "Old", "New"), "Rename fails on missing");
    assertTrue(sys.getPresetCount("nonexistent") == 0, "0 presets on missing");
    assertTrue(sys.getModuleCount("nonexistent", "Fit") == 0, "0 modules on missing");
    assertTrue(!sys.hasPreset("nonexistent", "Fit"), "No preset on missing");
    assertTrue(sys.getPresetShipType("nonexistent", "Fit") == "", "Empty ship type on missing");
    assertTrue(sys.getTotalPresetsSaved("nonexistent") == 0, "0 total saved on missing");
}


void run_ship_loadout_preset_system_tests() {
    testShipLoadoutPresetCreate();
    testShipLoadoutPresetSave();
    testShipLoadoutPresetDuplicate();
    testShipLoadoutPresetAddModules();
    testShipLoadoutPresetShipType();
    testShipLoadoutPresetRemove();
    testShipLoadoutPresetRename();
    testShipLoadoutPresetRenameDuplicate();
    testShipLoadoutPresetMaxPresets();
    testShipLoadoutPresetMaxModules();
    testShipLoadoutPresetValidation();
    testShipLoadoutPresetMissing();
}
