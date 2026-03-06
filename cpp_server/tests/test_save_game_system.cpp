// Tests for: Save Game System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/save_game_system.h"

using namespace atlas;

// ==================== Save Game System Tests ====================

static void testSaveGameCreate() {
    std::cout << "\n=== SaveGame: Create ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    assertTrue(sys.initialize("sg1", "player_001"), "Init succeeds");
    assertTrue(sys.getSlotCount("sg1") == 0, "No slots initially");
    assertTrue(sys.getOccupiedSlotCount("sg1") == 0, "No occupied slots");
    assertTrue(sys.getSaveStatus("sg1") == 0, "Status is Idle");
    assertTrue(sys.getTotalSaves("sg1") == 0, "No saves");
    assertTrue(sys.getTotalLoads("sg1") == 0, "No loads");
    assertTrue(sys.getSaveErrors("sg1") == 0, "No errors");
    assertTrue(approxEqual(sys.getLastSaveTime("sg1"), 0.0f), "No last save time");
}

static void testSaveGameCreateSlot() {
    std::cout << "\n=== SaveGame: CreateSlot ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    assertTrue(sys.createSaveSlot("sg1", "slot1", "Pilot Alpha", "Jita", "Rifter", 50000.0, 1000, 3600.0f), "Create slot 1");
    assertTrue(sys.createSaveSlot("sg1", "slot2", "Pilot Beta", "Amarr", "Merlin", 25000.0, 500, 1800.0f), "Create slot 2");
    assertTrue(sys.getSlotCount("sg1") == 2, "2 slots");
    assertTrue(sys.getOccupiedSlotCount("sg1") == 2, "2 occupied");
    assertTrue(!sys.createSaveSlot("sg1", "slot1", "Dup", "X", "X", 0, 0, 0), "Duplicate rejected");
}

static void testSaveGameDeleteSlot() {
    std::cout << "\n=== SaveGame: DeleteSlot ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    sys.createSaveSlot("sg1", "slot1", "Pilot", "Jita", "Rifter", 50000.0, 1000, 3600.0f);
    assertTrue(sys.deleteSaveSlot("sg1", "slot1"), "Delete succeeds");
    assertTrue(sys.getSlotCount("sg1") == 0, "0 slots after delete");
    assertTrue(!sys.deleteSaveSlot("sg1", "slot1"), "Double delete fails");
}

static void testSaveGameOverwrite() {
    std::cout << "\n=== SaveGame: Overwrite ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    sys.createSaveSlot("sg1", "slot1", "Pilot", "Jita", "Rifter", 50000.0, 1000, 3600.0f);
    assertTrue(sys.overwriteSaveSlot("sg1", "slot1", "Pilot Updated", "Amarr", "Merlin", 75000.0, 2000, 7200.0f), "Overwrite succeeds");
    assertTrue(sys.getSlotCount("sg1") == 1, "Still 1 slot");
    assertTrue(!sys.overwriteSaveSlot("sg1", "nonexistent", "X", "X", "X", 0, 0, 0), "Overwrite nonexistent fails");
}

static void testSaveGameMaxSlots() {
    std::cout << "\n=== SaveGame: MaxSlots ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    auto* entity = world.getEntity("sg1");
    auto* state = entity->getComponent<components::SaveGameState>();
    state->max_slots = 2;
    sys.createSaveSlot("sg1", "s1", "A", "J", "R", 100, 10, 100);
    sys.createSaveSlot("sg1", "s2", "B", "A", "M", 200, 20, 200);
    assertTrue(!sys.createSaveSlot("sg1", "s3", "C", "D", "K", 300, 30, 300), "Max slots enforced");
}

static void testSaveGameLoadSlot() {
    std::cout << "\n=== SaveGame: LoadSlot ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    sys.createSaveSlot("sg1", "slot1", "Pilot", "Jita", "Rifter", 50000.0, 1000, 3600.0f);
    assertTrue(sys.loadSaveSlot("sg1", "slot1"), "Load succeeds");
    assertTrue(!sys.loadSaveSlot("sg1", "nonexistent"), "Load nonexistent fails");
}

static void testSaveGameCorruptedSlot() {
    std::cout << "\n=== SaveGame: CorruptedSlot ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    sys.createSaveSlot("sg1", "slot1", "Pilot", "Jita", "Rifter", 50000.0, 1000, 3600.0f);
    auto* entity = world.getEntity("sg1");
    auto* state = entity->getComponent<components::SaveGameState>();
    state->slots[0].corrupted = true;
    assertTrue(!sys.loadSaveSlot("sg1", "slot1"), "Load corrupted slot fails");
}

static void testSaveGameDirtyFlags() {
    std::cout << "\n=== SaveGame: DirtyFlags ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    assertTrue(!sys.isDirty("sg1"), "Not dirty initially");
    assertTrue(sys.markDirty("sg1", "ship"), "Mark ship dirty");
    assertTrue(sys.markDirty("sg1", "wallet"), "Mark wallet dirty");
    assertTrue(sys.isDirty("sg1"), "Now dirty");
    assertTrue(!sys.markDirty("sg1", "invalid"), "Invalid category rejected");
    assertTrue(sys.clearDirty("sg1"), "Clear dirty");
    assertTrue(!sys.isDirty("sg1"), "Not dirty after clear");
}

static void testSaveGameDirtyAllCategories() {
    std::cout << "\n=== SaveGame: DirtyAllCategories ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    assertTrue(sys.markDirty("sg1", "ship"), "ship");
    assertTrue(sys.markDirty("sg1", "wallet"), "wallet");
    assertTrue(sys.markDirty("sg1", "skills"), "skills");
    assertTrue(sys.markDirty("sg1", "standings"), "standings");
    assertTrue(sys.markDirty("sg1", "cargo"), "cargo");
    assertTrue(sys.markDirty("sg1", "missions"), "missions");
    assertTrue(sys.isDirty("sg1"), "All categories dirty");
    sys.clearDirty("sg1");
    assertTrue(!sys.isDirty("sg1"), "All cleared");
}

static void testSaveGameSaveLifecycle() {
    std::cout << "\n=== SaveGame: SaveLifecycle ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    assertTrue(sys.beginSave("sg1"), "Begin save");
    assertTrue(sys.getSaveStatus("sg1") == 1, "Status is Saving");
    assertTrue(!sys.beginSave("sg1"), "Double begin fails");
    assertTrue(sys.completeSave("sg1", 100.0f), "Complete save");
    assertTrue(sys.getSaveStatus("sg1") == 0, "Status is Idle");
    assertTrue(sys.getTotalSaves("sg1") == 1, "1 total save");
    assertTrue(approxEqual(sys.getLastSaveTime("sg1"), 100.0f), "Last save at 100");
}

static void testSaveGameLoadLifecycle() {
    std::cout << "\n=== SaveGame: LoadLifecycle ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    assertTrue(sys.beginLoad("sg1"), "Begin load");
    assertTrue(sys.getSaveStatus("sg1") == 2, "Status is Loading");
    assertTrue(!sys.beginLoad("sg1"), "Double begin load fails");
    assertTrue(sys.completeLoad("sg1"), "Complete load");
    assertTrue(sys.getSaveStatus("sg1") == 0, "Status is Idle");
    assertTrue(sys.getTotalLoads("sg1") == 1, "1 total load");
}

static void testSaveGameError() {
    std::cout << "\n=== SaveGame: Error ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    sys.beginSave("sg1");
    assertTrue(sys.reportError("sg1"), "Report error");
    assertTrue(sys.getSaveStatus("sg1") == 3, "Status is Error");
    assertTrue(sys.getSaveErrors("sg1") == 1, "1 error");
}

static void testSaveGameUpdate() {
    std::cout << "\n=== SaveGame: Update ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    world.createEntity("sg1");
    sys.initialize("sg1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("sg1");
    auto* state = entity->getComponent<components::SaveGameState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
    assertTrue(approxEqual(state->auto_save_timer, 3.5f), "Auto-save timer 3.5s");
}

static void testSaveGameMissing() {
    std::cout << "\n=== SaveGame: Missing ===" << std::endl;
    ecs::World world;
    systems::SaveGameSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails on missing");
    assertTrue(!sys.createSaveSlot("nonexistent", "s", "n", "l", "t", 0, 0, 0), "createSaveSlot fails");
    assertTrue(!sys.deleteSaveSlot("nonexistent", "s"), "deleteSaveSlot fails");
    assertTrue(!sys.overwriteSaveSlot("nonexistent", "s", "n", "l", "t", 0, 0, 0), "overwriteSaveSlot fails");
    assertTrue(!sys.loadSaveSlot("nonexistent", "s"), "loadSaveSlot fails");
    assertTrue(!sys.markDirty("nonexistent", "ship"), "markDirty fails");
    assertTrue(!sys.clearDirty("nonexistent"), "clearDirty fails");
    assertTrue(!sys.isDirty("nonexistent"), "isDirty returns false");
    assertTrue(!sys.beginSave("nonexistent"), "beginSave fails");
    assertTrue(!sys.completeSave("nonexistent", 0), "completeSave fails");
    assertTrue(!sys.beginLoad("nonexistent"), "beginLoad fails");
    assertTrue(!sys.completeLoad("nonexistent"), "completeLoad fails");
    assertTrue(!sys.reportError("nonexistent"), "reportError fails");
    assertTrue(sys.getSlotCount("nonexistent") == 0, "0 slots");
    assertTrue(sys.getOccupiedSlotCount("nonexistent") == 0, "0 occupied");
    assertTrue(sys.getSaveStatus("nonexistent") == -1, "-1 status");
    assertTrue(sys.getTotalSaves("nonexistent") == 0, "0 saves");
    assertTrue(sys.getTotalLoads("nonexistent") == 0, "0 loads");
    assertTrue(sys.getSaveErrors("nonexistent") == 0, "0 errors");
    assertTrue(approxEqual(sys.getLastSaveTime("nonexistent"), 0.0f), "0 last save time");
}

void run_save_game_system_tests() {
    testSaveGameCreate();
    testSaveGameCreateSlot();
    testSaveGameDeleteSlot();
    testSaveGameOverwrite();
    testSaveGameMaxSlots();
    testSaveGameLoadSlot();
    testSaveGameCorruptedSlot();
    testSaveGameDirtyFlags();
    testSaveGameDirtyAllCategories();
    testSaveGameSaveLifecycle();
    testSaveGameLoadLifecycle();
    testSaveGameError();
    testSaveGameUpdate();
    testSaveGameMissing();
}
