// Tests for: LoadoutPersistenceSystem
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/loadout_persistence_system.h"

using namespace atlas;

// ==================== LoadoutPersistenceSystem Tests ====================

static void testLoadoutSave() {
    std::cout << "\n=== Loadout: Save ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    assertTrue(sys.saveLoadout("player_1", "lo_1", "PvE Fit", "Frigate"), "Save loadout succeeds");
    assertTrue(sys.getLoadoutCount("player_1") == 1, "1 loadout saved");
    assertTrue(sys.getLoadoutName("player_1", "lo_1") == "PvE Fit", "Name is PvE Fit");
    assertTrue(sys.getLoadoutShipClass("player_1", "lo_1") == "Frigate", "Ship class is Frigate");
}

static void testLoadoutAddModule() {
    std::cout << "\n=== Loadout: Add Module ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    sys.saveLoadout("player_1", "lo_1", "PvE Fit", "Frigate");
    assertTrue(sys.addModuleToLoadout("player_1", "lo_1", "gun_1", "200mm AutoCannon", 0, "high"), "Add high slot 0");
    assertTrue(sys.addModuleToLoadout("player_1", "lo_1", "shield_1", "Small Shield Booster", 0, "mid"), "Add mid slot 0");
    assertTrue(sys.getModuleCount("player_1", "lo_1") == 2, "2 modules in loadout");
    assertTrue(sys.hasModule("player_1", "lo_1", "gun_1"), "Has gun module");
}

static void testLoadoutDuplicateSlot() {
    std::cout << "\n=== Loadout: Duplicate Slot ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    sys.saveLoadout("player_1", "lo_1", "Test", "Frigate");
    sys.addModuleToLoadout("player_1", "lo_1", "gun_1", "AutoCannon", 0, "high");
    assertTrue(!sys.addModuleToLoadout("player_1", "lo_1", "gun_2", "Laser", 0, "high"), "Duplicate slot rejected");
    assertTrue(sys.getModuleCount("player_1", "lo_1") == 1, "Still 1 module");
}

static void testLoadoutDelete() {
    std::cout << "\n=== Loadout: Delete ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    sys.saveLoadout("player_1", "lo_1", "Fit A", "Frigate");
    sys.saveLoadout("player_1", "lo_2", "Fit B", "Cruiser");
    assertTrue(sys.deleteLoadout("player_1", "lo_1"), "Delete succeeds");
    assertTrue(sys.getLoadoutCount("player_1") == 1, "1 loadout remaining");
    assertTrue(!sys.hasLoadout("player_1", "lo_1"), "lo_1 gone");
    assertTrue(sys.hasLoadout("player_1", "lo_2"), "lo_2 still exists");
}

static void testLoadoutRename() {
    std::cout << "\n=== Loadout: Rename ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    sys.saveLoadout("player_1", "lo_1", "Old Name", "Frigate");
    assertTrue(sys.renameLoadout("player_1", "lo_1", "New Name"), "Rename succeeds");
    assertTrue(sys.getLoadoutName("player_1", "lo_1") == "New Name", "Name updated");
}

static void testLoadoutOverwrite() {
    std::cout << "\n=== Loadout: Overwrite ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    sys.saveLoadout("player_1", "lo_1", "First", "Frigate");
    sys.addModuleToLoadout("player_1", "lo_1", "gun_1", "Gun", 0, "high");
    sys.saveLoadout("player_1", "lo_1", "Second", "Cruiser");

    assertTrue(sys.getLoadoutCount("player_1") == 1, "Still 1 loadout (overwritten)");
    assertTrue(sys.getLoadoutName("player_1", "lo_1") == "Second", "Name overwritten");
    assertTrue(sys.getLoadoutShipClass("player_1", "lo_1") == "Cruiser", "Ship class overwritten");
    assertTrue(sys.getModuleCount("player_1", "lo_1") == 0, "Modules cleared on overwrite");
}

static void testLoadoutMaxCapacity() {
    std::cout << "\n=== Loadout: Max Capacity ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* sl = addComp<components::SavedLoadout>(e);
    sl->max_loadouts = 3;

    sys.saveLoadout("player_1", "lo_1", "A", "Frigate");
    sys.saveLoadout("player_1", "lo_2", "B", "Cruiser");
    sys.saveLoadout("player_1", "lo_3", "C", "Battleship");
    assertTrue(!sys.saveLoadout("player_1", "lo_4", "D", "Titan"), "Max loadouts enforced");
    assertTrue(sys.getLoadoutCount("player_1") == 3, "Still 3 loadouts");
}

static void testLoadoutSetActive() {
    std::cout << "\n=== Loadout: Set Active ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    sys.saveLoadout("player_1", "lo_1", "Fit A", "Frigate");
    sys.saveLoadout("player_1", "lo_2", "Fit B", "Cruiser");

    assertTrue(sys.setActiveLoadout("player_1", "lo_2"), "Set active succeeds");
    assertTrue(sys.getActiveLoadoutId("player_1") == "lo_2", "Active is lo_2");
}

static void testLoadoutDeleteClearsActive() {
    std::cout << "\n=== Loadout: Delete Clears Active ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    sys.saveLoadout("player_1", "lo_1", "Active Fit", "Frigate");
    sys.setActiveLoadout("player_1", "lo_1");
    sys.deleteLoadout("player_1", "lo_1");

    assertTrue(sys.getActiveLoadoutId("player_1") == "", "Active cleared after delete");
}

static void testLoadoutSetActiveNonexistent() {
    std::cout << "\n=== Loadout: Set Active Nonexistent ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::SavedLoadout>(e);

    assertTrue(!sys.setActiveLoadout("player_1", "nonexistent"), "Set active fails for nonexistent");
}

static void testLoadoutMissingEntity() {
    std::cout << "\n=== Loadout: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::LoadoutPersistenceSystem sys(&world);

    assertTrue(!sys.saveLoadout("nonexistent", "lo_1", "A", "Frigate"), "Save fails on missing");
    assertTrue(!sys.deleteLoadout("nonexistent", "lo_1"), "Delete fails on missing");
    assertTrue(!sys.renameLoadout("nonexistent", "lo_1", "B"), "Rename fails on missing");
    assertTrue(sys.getLoadoutCount("nonexistent") == 0, "0 loadouts on missing");
    assertTrue(sys.getActiveLoadoutId("nonexistent") == "", "Empty active on missing");
    assertTrue(!sys.hasLoadout("nonexistent", "lo_1"), "No loadout on missing");
    assertTrue(!sys.hasModule("nonexistent", "lo_1", "mod"), "No module on missing");
}

void run_loadout_persistence_system_tests() {
    testLoadoutSave();
    testLoadoutAddModule();
    testLoadoutDuplicateSlot();
    testLoadoutDelete();
    testLoadoutRename();
    testLoadoutOverwrite();
    testLoadoutMaxCapacity();
    testLoadoutSetActive();
    testLoadoutDeleteClearsActive();
    testLoadoutSetActiveNonexistent();
    testLoadoutMissingEntity();
}
