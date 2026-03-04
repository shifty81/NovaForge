// Tests for: RigLocker Tests
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/rig_locker_system.h"

using namespace atlas;

// ==================== RigLocker Tests ====================

static void testRigLockerInit() {
    std::cout << "\n=== RigLocker: Init ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    assertTrue(sys.initializeLocker("locker_1", "player_1"), "Locker initialized");
    assertTrue(sys.getPresetCount("locker_1") == 0, "No presets initially");
    assertTrue(!sys.initializeLocker("locker_1", "player_1"), "Duplicate init fails");
}

static void testRigLockerSave() {
    std::cout << "\n=== RigLocker: Save Preset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    std::vector<std::string> modules = {"LifeSupport", "PowerCore", "JetpackTank"};
    assertTrue(sys.savePreset("locker_1", "Combat Suit", modules), "Preset saved");
    assertTrue(sys.getPresetCount("locker_1") == 1, "1 preset after save");
}

static void testRigLockerDelete() {
    std::cout << "\n=== RigLocker: Delete Preset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "Mining Suit", {"Sensor", "CargoPod"});
    assertTrue(sys.getPresetCount("locker_1") == 1, "1 preset before delete");
    assertTrue(sys.deletePreset("locker_1", "preset_0"), "Preset deleted");
    assertTrue(sys.getPresetCount("locker_1") == 0, "0 presets after delete");
    assertTrue(!sys.deletePreset("locker_1", "preset_0"), "Double delete fails");
}

static void testRigLockerRename() {
    std::cout << "\n=== RigLocker: Rename Preset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "Old Name", {"PowerCore"});
    assertTrue(sys.renamePreset("locker_1", "preset_0", "New Name"), "Renamed");
    assertTrue(!sys.renamePreset("locker_1", "nonexistent", "X"), "Rename nonexistent fails");
}

static void testRigLockerEquip() {
    std::cout << "\n=== RigLocker: Equip Preset ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "EVA Suit", {"LifeSupport", "JetpackTank"});
    assertTrue(sys.equipPreset("locker_1", "preset_0"), "Preset equipped");
    assertTrue(sys.getActivePreset("locker_1") == "preset_0", "Active preset matches");
    assertTrue(!sys.equipPreset("locker_1", "nonexistent"), "Equip nonexistent fails");
}

static void testRigLockerFavorite() {
    std::cout << "\n=== RigLocker: Favorite ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "Suit A", {"PowerCore"});
    sys.savePreset("locker_1", "Suit B", {"Shield"});
    assertTrue(sys.toggleFavorite("locker_1", "preset_0"), "Toggled favorite on");
    assertTrue(sys.getFavoriteCount("locker_1") == 1, "1 favorite");
    assertTrue(sys.toggleFavorite("locker_1", "preset_0"), "Toggled favorite off");
    assertTrue(sys.getFavoriteCount("locker_1") == 0, "0 favorites after toggle off");
}

static void testRigLockerMass() {
    std::cout << "\n=== RigLocker: Mass Calculation ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "Heavy Suit", {"A", "B", "C", "D"});
    float mass = sys.getPresetMass("locker_1", "preset_0");
    assertTrue(mass > 0.0f, "Mass is positive");
    assertTrue(approxEqual(mass, 6.0f), "4 modules × 1.5kg = 6kg");
}

static void testRigLockerMaxPresets() {
    std::cout << "\n=== RigLocker: Max Presets ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    for (int i = 0; i < 10; i++) {
        assertTrue(sys.savePreset("locker_1", "Preset " + std::to_string(i), {"mod"}),
                   "Save preset " + std::to_string(i));
    }
    assertTrue(sys.getPresetCount("locker_1") == 10, "10 presets at max");
    assertTrue(!sys.savePreset("locker_1", "Overflow", {"mod"}), "11th preset rejected");
}

static void testRigLockerEquipTracking() {
    std::cout << "\n=== RigLocker: Equip Tracking ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    world.createEntity("locker_1");
    sys.initializeLocker("locker_1", "player_1");
    sys.savePreset("locker_1", "A", {"mod_a"});
    sys.savePreset("locker_1", "B", {"mod_b"});
    sys.equipPreset("locker_1", "preset_0");
    sys.equipPreset("locker_1", "preset_1");
    assertTrue(sys.getActivePreset("locker_1") == "preset_1", "Active is last equipped");
    sys.deletePreset("locker_1", "preset_1");
    assertTrue(sys.getActivePreset("locker_1").empty(), "Active cleared after delete");
}

static void testRigLockerMissing() {
    std::cout << "\n=== RigLocker: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::RigLockerSystem sys(&world);
    assertTrue(!sys.initializeLocker("nonexistent", "p"), "Init fails on missing");
    assertTrue(!sys.savePreset("nonexistent", "x", {}), "Save fails on missing");
    assertTrue(sys.getPresetCount("nonexistent") == 0, "Count 0 on missing");
    assertTrue(sys.getActivePreset("nonexistent").empty(), "Empty active on missing");
    assertTrue(approxEqual(sys.getPresetMass("nonexistent", "x"), 0.0f), "0 mass on missing");
}


void run_rig_locker_tests() {
    testRigLockerInit();
    testRigLockerSave();
    testRigLockerDelete();
    testRigLockerRename();
    testRigLockerEquip();
    testRigLockerFavorite();
    testRigLockerMass();
    testRigLockerMaxPresets();
    testRigLockerEquipTracking();
    testRigLockerMissing();
}
