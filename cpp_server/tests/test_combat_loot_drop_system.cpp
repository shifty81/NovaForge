// Tests for: Combat Loot Drop System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/combat_loot_drop_system.h"

using namespace atlas;

// ==================== Combat Loot Drop System Tests ====================

static void testCombatLootDropCreate() {
    std::cout << "\n=== CombatLootDrop: Create ===" << std::endl;
    ecs::World world;
    systems::CombatLootDropSystem sys(&world);
    world.createEntity("table1");
    assertTrue(sys.initialize("table1"), "Init succeeds");
    assertTrue(sys.getDropEntryCount("table1") == 0, "0 entries");
    assertTrue(sys.getTotalDropsTriggered("table1") == 0, "0 drops triggered");
    assertTrue(sys.getTotalItemsDropped("table1") == 0, "0 items dropped");
    assertTrue(sys.getPendingDropCount("table1") == 0, "0 pending");
    assertTrue(sys.getLastDropSource("table1") == "", "No last source");
}

static void testCombatLootDropAddEntry() {
    std::cout << "\n=== CombatLootDrop: AddEntry ===" << std::endl;
    ecs::World world;
    systems::CombatLootDropSystem sys(&world);
    world.createEntity("table1");
    sys.initialize("table1");

    assertTrue(sys.addDropEntry("table1", "tritanium", 0.8f, 10, 50, "common"), "Add common");
    assertTrue(sys.addDropEntry("table1", "module_01", 0.2f, 1, 1, "rare"), "Add rare");
    assertTrue(sys.getDropEntryCount("table1") == 2, "2 entries");

    // Duplicate rejected
    assertTrue(!sys.addDropEntry("table1", "tritanium", 0.5f, 5, 10, "common"), "Dup rejected");

    // Invalid qty
    assertTrue(!sys.addDropEntry("table1", "bad1", 0.5f, -1, 5, "common"), "Neg min rejected");
    assertTrue(!sys.addDropEntry("table1", "bad2", 0.5f, 10, 5, "common"), "Min > max rejected");
}

static void testCombatLootDropRemoveEntry() {
    std::cout << "\n=== CombatLootDrop: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::CombatLootDropSystem sys(&world);
    world.createEntity("table1");
    sys.initialize("table1");

    sys.addDropEntry("table1", "tritanium", 0.8f, 10, 50, "common");
    sys.addDropEntry("table1", "module_01", 0.2f, 1, 1, "rare");

    assertTrue(sys.removeDropEntry("table1", "tritanium"), "Remove tritanium");
    assertTrue(sys.getDropEntryCount("table1") == 1, "1 entry");
    assertTrue(!sys.removeDropEntry("table1", "tritanium"), "Can't remove twice");
    assertTrue(!sys.removeDropEntry("table1", "nonexistent"), "Can't remove missing");
}

static void testCombatLootDropTrigger() {
    std::cout << "\n=== CombatLootDrop: Trigger ===" << std::endl;
    ecs::World world;
    systems::CombatLootDropSystem sys(&world);
    world.createEntity("table1");
    sys.initialize("table1");

    // Can't trigger with empty table
    assertTrue(!sys.triggerDrop("table1", "npc_pirate"), "Empty table rejects trigger");

    sys.addDropEntry("table1", "tritanium", 1.0f, 10, 10, "common");
    assertTrue(sys.triggerDrop("table1", "npc_pirate"), "Trigger from pirate");
    assertTrue(sys.getPendingDropCount("table1") == 1, "1 pending");
    assertTrue(sys.getLastDropSource("table1") == "npc_pirate", "Source is pirate");

    // Process the drop
    sys.update(0.5f);
    assertTrue(sys.getTotalDropsTriggered("table1") == 1, "1 triggered after tick");
    assertTrue(sys.getPendingDropCount("table1") == 0, "0 pending after tick");
    assertTrue(sys.getTotalItemsDropped("table1") > 0, "Items dropped");
}

static void testCombatLootDropMultiple() {
    std::cout << "\n=== CombatLootDrop: Multiple ===" << std::endl;
    ecs::World world;
    systems::CombatLootDropSystem sys(&world);
    world.createEntity("table1");
    sys.initialize("table1");

    sys.addDropEntry("table1", "tritanium", 1.0f, 5, 5, "common");
    sys.addDropEntry("table1", "pyerite", 1.0f, 3, 3, "common");

    sys.triggerDrop("table1", "npc_01");
    sys.triggerDrop("table1", "npc_02");
    assertTrue(sys.getPendingDropCount("table1") == 2, "2 pending");

    sys.update(0.5f);
    assertTrue(sys.getTotalDropsTriggered("table1") == 2, "2 triggered after tick");
    assertTrue(sys.getPendingDropCount("table1") == 0, "0 pending");
}

static void testCombatLootDropChanceClamped() {
    std::cout << "\n=== CombatLootDrop: ChanceClamped ===" << std::endl;
    ecs::World world;
    systems::CombatLootDropSystem sys(&world);
    world.createEntity("table1");
    sys.initialize("table1");

    // Out-of-range chance values are clamped
    assertTrue(sys.addDropEntry("table1", "item1", 2.0f, 1, 1, "common"), "Over 1.0 accepted (clamped)");
    assertTrue(sys.addDropEntry("table1", "item2", -0.5f, 1, 1, "common"), "Negative accepted (clamped)");
    assertTrue(sys.getDropEntryCount("table1") == 2, "2 entries added");
}

static void testCombatLootDropMaxEntries() {
    std::cout << "\n=== CombatLootDrop: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::CombatLootDropSystem sys(&world);
    world.createEntity("table1");
    sys.initialize("table1");

    // Fill to max (20)
    for (int i = 0; i < 20; i++) {
        assertTrue(sys.addDropEntry("table1", "item_" + std::to_string(i),
                   0.5f, 1, 5, "common"), "Add item_" + std::to_string(i));
    }
    assertTrue(sys.getDropEntryCount("table1") == 20, "20 entries at max");
    assertTrue(!sys.addDropEntry("table1", "item_20", 0.5f, 1, 5, "common"), "21st rejected");
}

static void testCombatLootDropMissing() {
    std::cout << "\n=== CombatLootDrop: Missing ===" << std::endl;
    ecs::World world;
    systems::CombatLootDropSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addDropEntry("nonexistent", "t", 0.5f, 1, 5, "c"), "Add fails on missing");
    assertTrue(!sys.removeDropEntry("nonexistent", "t"), "Remove fails on missing");
    assertTrue(!sys.triggerDrop("nonexistent", "src"), "Trigger fails on missing");
    assertTrue(sys.getDropEntryCount("nonexistent") == 0, "0 entries on missing");
    assertTrue(sys.getTotalDropsTriggered("nonexistent") == 0, "0 triggered on missing");
    assertTrue(sys.getTotalItemsDropped("nonexistent") == 0, "0 items on missing");
    assertTrue(sys.getPendingDropCount("nonexistent") == 0, "0 pending on missing");
    assertTrue(sys.getLastDropSource("nonexistent") == "", "Empty source on missing");
}

void run_combat_loot_drop_system_tests() {
    testCombatLootDropCreate();
    testCombatLootDropAddEntry();
    testCombatLootDropRemoveEntry();
    testCombatLootDropTrigger();
    testCombatLootDropMultiple();
    testCombatLootDropChanceClamped();
    testCombatLootDropMaxEntries();
    testCombatLootDropMissing();
}
