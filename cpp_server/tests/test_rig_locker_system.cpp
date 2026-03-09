// Tests for: RigLockerSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/rig_locker_system.h"

using namespace atlas;

// ==================== RigLockerSystem Tests ====================

static void testRigLockerInitialize() {
    std::cout << "\n=== RigLocker: Initialize ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker1");

    assertTrue(sys.initializeLocker("locker1", "player1"), "Initialize locker");
    assertTrue(sys.getPresetCount("locker1") == 0, "No presets initially");
    assertTrue(sys.getActivePreset("locker1").empty(), "No active preset initially");
    assertTrue(sys.getFavoriteCount("locker1") == 0, "No favorites initially");
}

static void testRigLockerDuplicateInitRejected() {
    std::cout << "\n=== RigLocker: DuplicateInitRejected ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker1");

    assertTrue(sys.initializeLocker("locker1", "player1"), "First init ok");
    assertTrue(!sys.initializeLocker("locker1", "player2"), "Duplicate init rejected");
}

static void testRigLockerSavePreset() {
    std::cout << "\n=== RigLocker: SavePreset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    std::vector<std::string> mods = {"helmet_mk1", "suit_basic", "boots_mag"};
    assertTrue(sys.savePreset("locker1", "Combat Rig", mods), "Save preset");
    assertTrue(sys.getPresetCount("locker1") == 1, "Preset count is 1");
}

static void testRigLockerSaveMultiplePresets() {
    std::cout << "\n=== RigLocker: SaveMultiplePresets ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    sys.savePreset("locker1", "Combat Rig", {"helmet_mk1", "suit_basic"});
    sys.savePreset("locker1", "Mining Rig", {"helmet_mining", "suit_heavy"});
    sys.savePreset("locker1", "Explorer Rig", {"helmet_scanner", "suit_light"});

    assertTrue(sys.getPresetCount("locker1") == 3, "Preset count is 3");
}

static void testRigLockerMaxPresetsRespected() {
    std::cout << "\n=== RigLocker: MaxPresetsRespected ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    auto* e = world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    auto* locker = e->getComponent<components::RigLockerPreset>();
    locker->max_presets = 2;

    sys.savePreset("locker1", "Rig A", {"mod_a"});
    sys.savePreset("locker1", "Rig B", {"mod_b"});
    assertTrue(!sys.savePreset("locker1", "Rig C", {"mod_c"}), "Cannot exceed max presets");
    assertTrue(sys.getPresetCount("locker1") == 2, "Still 2 presets");
}

static void testRigLockerDeletePreset() {
    std::cout << "\n=== RigLocker: DeletePreset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    auto* e = world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    sys.savePreset("locker1", "Combat Rig", {"helmet_mk1"});
    auto* locker = e->getComponent<components::RigLockerPreset>();
    std::string preset_id = locker->presets[0].preset_id;

    assertTrue(sys.deletePreset("locker1", preset_id), "Delete preset");
    assertTrue(sys.getPresetCount("locker1") == 0, "Preset count is 0");
}

static void testRigLockerDeleteActivePresetClearsActive() {
    std::cout << "\n=== RigLocker: DeleteActivePresetClearsActive ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    auto* e = world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    sys.savePreset("locker1", "Combat Rig", {"helmet_mk1"});
    auto* locker = e->getComponent<components::RigLockerPreset>();
    std::string preset_id = locker->presets[0].preset_id;

    sys.equipPreset("locker1", preset_id);
    assertTrue(sys.getActivePreset("locker1") == preset_id, "Active is set");

    sys.deletePreset("locker1", preset_id);
    assertTrue(sys.getActivePreset("locker1").empty(), "Active cleared after delete");
}

static void testRigLockerDeleteNonexistent() {
    std::cout << "\n=== RigLocker: DeleteNonexistent ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    assertTrue(!sys.deletePreset("locker1", "ghost_preset"), "Cannot delete nonexistent preset");
}

static void testRigLockerRenamePreset() {
    std::cout << "\n=== RigLocker: RenamePreset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    auto* e = world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    sys.savePreset("locker1", "Old Name", {"mod_a"});
    auto* locker = e->getComponent<components::RigLockerPreset>();
    std::string preset_id = locker->presets[0].preset_id;

    assertTrue(sys.renamePreset("locker1", preset_id, "New Name"), "Rename preset");
    assertTrue(locker->presets[0].name == "New Name", "Name updated");
}

static void testRigLockerRenameNonexistent() {
    std::cout << "\n=== RigLocker: RenameNonexistent ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    assertTrue(!sys.renamePreset("locker1", "ghost", "New"), "Cannot rename nonexistent preset");
}

static void testRigLockerEquipPreset() {
    std::cout << "\n=== RigLocker: EquipPreset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    auto* e = world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    sys.savePreset("locker1", "Combat Rig", {"helmet_mk1", "suit_basic"});
    auto* locker = e->getComponent<components::RigLockerPreset>();
    std::string preset_id = locker->presets[0].preset_id;

    assertTrue(sys.equipPreset("locker1", preset_id), "Equip preset");
    assertTrue(sys.getActivePreset("locker1") == preset_id, "Active preset set");
    assertTrue(locker->total_equips == 1, "Total equips is 1");
}

static void testRigLockerEquipNonexistent() {
    std::cout << "\n=== RigLocker: EquipNonexistent ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    assertTrue(!sys.equipPreset("locker1", "ghost"), "Cannot equip nonexistent preset");
}

static void testRigLockerToggleFavorite() {
    std::cout << "\n=== RigLocker: ToggleFavorite ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    auto* e = world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    sys.savePreset("locker1", "Combat Rig", {"helmet_mk1"});
    auto* locker = e->getComponent<components::RigLockerPreset>();
    std::string preset_id = locker->presets[0].preset_id;

    assertTrue(sys.getFavoriteCount("locker1") == 0, "No favorites initially");

    assertTrue(sys.toggleFavorite("locker1", preset_id), "Toggle favorite on");
    assertTrue(sys.getFavoriteCount("locker1") == 1, "1 favorite");

    assertTrue(sys.toggleFavorite("locker1", preset_id), "Toggle favorite off");
    assertTrue(sys.getFavoriteCount("locker1") == 0, "0 favorites again");
}

static void testRigLockerPresetMass() {
    std::cout << "\n=== RigLocker: PresetMass ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    auto* e = world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    // 3 modules * 1.5kg each = 4.5kg
    sys.savePreset("locker1", "Heavy Rig", {"mod_a", "mod_b", "mod_c"});
    auto* locker = e->getComponent<components::RigLockerPreset>();
    std::string preset_id = locker->presets[0].preset_id;

    assertTrue(approxEqual(sys.getPresetMass("locker1", preset_id), 4.5f), "Preset mass is 4.5kg");
}

static void testRigLockerPresetMassNonexistent() {
    std::cout << "\n=== RigLocker: PresetMassNonexistent ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker1");
    sys.initializeLocker("locker1", "player1");

    assertTrue(sys.getPresetMass("locker1", "ghost") == 0.0f, "Mass 0 for nonexistent preset");
}

static void testRigLockerMissingEntity() {
    std::cout << "\n=== RigLocker: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);

    assertTrue(!sys.initializeLocker("ghost", "p1"), "Init fails for missing entity");
    assertTrue(!sys.savePreset("ghost", "r", {"m"}), "savePreset fails for missing");
    assertTrue(!sys.deletePreset("ghost", "p"), "deletePreset fails for missing");
    assertTrue(!sys.renamePreset("ghost", "p", "n"), "renamePreset fails for missing");
    assertTrue(!sys.equipPreset("ghost", "p"), "equipPreset fails for missing");
    assertTrue(!sys.toggleFavorite("ghost", "p"), "toggleFavorite fails for missing");
    assertTrue(sys.getPresetCount("ghost") == 0, "preset count 0 for missing");
    assertTrue(sys.getActivePreset("ghost").empty(), "active empty for missing");
    assertTrue(sys.getFavoriteCount("ghost") == 0, "favorite count 0 for missing");
    assertTrue(sys.getPresetMass("ghost", "p") == 0.0f, "mass 0 for missing");
}

void run_rig_locker_system_tests() {
    testRigLockerInitialize();
    testRigLockerDuplicateInitRejected();
    testRigLockerSavePreset();
    testRigLockerSaveMultiplePresets();
    testRigLockerMaxPresetsRespected();
    testRigLockerDeletePreset();
    testRigLockerDeleteActivePresetClearsActive();
    testRigLockerDeleteNonexistent();
    testRigLockerRenamePreset();
    testRigLockerRenameNonexistent();
    testRigLockerEquipPreset();
    testRigLockerEquipNonexistent();
    testRigLockerToggleFavorite();
    testRigLockerPresetMass();
    testRigLockerPresetMassNonexistent();
    testRigLockerMissingEntity();
}
