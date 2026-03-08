// Tests for: Loot Table System
#include "test_log.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/loot_table_system.h"

using namespace atlas;

// ==================== Loot Table System Tests ====================

static void testLootTableCreate() {
    std::cout << "\n=== LootTable: Create ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    assertTrue(sys.initialize("loot1", "pirate_wreck"), "Init succeeds");
    assertTrue(sys.getTableId("loot1") == "pirate_wreck", "Table ID matches");
    assertTrue(sys.getEntryCount("loot1") == 0, "0 entries initially");
    assertTrue(approxEqual(sys.getTotalWeight("loot1"), 0.0f), "0 weight initially");
    assertTrue(sys.getTotalRolls("loot1") == 0, "0 rolls initially");
    assertTrue(sys.getTotalDrops("loot1") == 0, "0 drops initially");
    assertTrue(approxEqual(sys.getLuckModifier("loot1"), 1.0f), "Default luck 1.0");
}

static void testLootTableInitValidation() {
    std::cout << "\n=== LootTable: InitValidation ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    assertTrue(!sys.initialize("loot1", ""), "Empty table_id rejected");
    assertTrue(!sys.initialize("nonexistent", "table"), "Missing entity rejected");
}

static void testLootTableAddEntry() {
    std::cout << "\n=== LootTable: AddEntry ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    assertTrue(sys.addEntry("loot1", "iron_ore", "common", 10.0f, 5, 20), "Add iron_ore");
    assertTrue(sys.addEntry("loot1", "gold_ore", "rare", 2.0f, 1, 3), "Add gold_ore");
    assertTrue(sys.getEntryCount("loot1") == 2, "2 entries");
    assertTrue(approxEqual(sys.getTotalWeight("loot1"), 12.0f), "Total weight 12.0");
}

static void testLootTableAddEntryValidation() {
    std::cout << "\n=== LootTable: AddEntryValidation ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    assertTrue(!sys.addEntry("loot1", "", "common", 1.0f, 1, 1), "Empty item_id rejected");
    assertTrue(!sys.addEntry("loot1", "item", "", 1.0f, 1, 1), "Empty rarity rejected");
    assertTrue(!sys.addEntry("loot1", "item", "common", 0.0f, 1, 1), "Zero weight rejected");
    assertTrue(!sys.addEntry("loot1", "item", "common", -1.0f, 1, 1), "Negative weight rejected");
    assertTrue(!sys.addEntry("loot1", "item", "common", 1.0f, 0, 1), "Zero min_qty rejected");
    assertTrue(!sys.addEntry("loot1", "item", "common", 1.0f, 5, 2), "max < min rejected");
    assertTrue(!sys.addEntry("nonexistent", "item", "common", 1.0f, 1, 1), "Missing entity rejected");
}

static void testLootTableDuplicateEntry() {
    std::cout << "\n=== LootTable: DuplicateEntry ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    assertTrue(sys.addEntry("loot1", "iron", "common", 5.0f, 1, 10), "Add iron");
    assertTrue(!sys.addEntry("loot1", "iron", "rare", 1.0f, 1, 1), "Duplicate rejected");
}

static void testLootTableRemoveEntry() {
    std::cout << "\n=== LootTable: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    sys.addEntry("loot1", "iron", "common", 5.0f, 1, 10);
    sys.addEntry("loot1", "gold", "rare", 2.0f, 1, 3);
    assertTrue(sys.removeEntry("loot1", "iron"), "Remove iron");
    assertTrue(sys.getEntryCount("loot1") == 1, "1 entry left");
    assertTrue(!sys.removeEntry("loot1", "nonexistent"), "Cannot remove nonexistent");
}

static void testLootTableLuckModifier() {
    std::cout << "\n=== LootTable: LuckModifier ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    assertTrue(sys.setLuckModifier("loot1", 2.5f), "Set luck 2.5");
    assertTrue(approxEqual(sys.getLuckModifier("loot1"), 2.5f), "Luck is 2.5");
    assertTrue(sys.setLuckModifier("loot1", 0.0f), "Set luck 0 clamped");
    assertTrue(approxEqual(sys.getLuckModifier("loot1"), 0.1f), "Clamped to 0.1");
    assertTrue(sys.setLuckModifier("loot1", 99.0f), "Set luck 99 clamped");
    assertTrue(approxEqual(sys.getLuckModifier("loot1"), 10.0f), "Clamped to 10.0");
    assertTrue(!sys.setLuckModifier("nonexistent", 1.0f), "Missing entity rejected");
}

static void testLootTableRollLoot() {
    std::cout << "\n=== LootTable: RollLoot ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    sys.addEntry("loot1", "iron", "common", 10.0f, 1, 5);
    sys.addEntry("loot1", "gold", "rare", 2.0f, 1, 1);

    // Roll with seed 0.0 → should hit first entry (iron, weight 10)
    std::string result = sys.rollLoot("loot1", 0.0f);
    assertTrue(result == "iron", "Seed 0 → iron (common)");
    assertTrue(sys.getTotalRolls("loot1") == 1, "1 roll");
    assertTrue(sys.getTotalDrops("loot1") == 1, "1 drop");

    // Roll with seed just under total weight → should hit gold
    // Total weight = 10 + 2 = 12, so seed 11.0 should reach gold
    result = sys.rollLoot("loot1", 11.0f);
    assertTrue(result == "gold", "Seed 11 → gold (rare)");
    assertTrue(sys.getTotalRolls("loot1") == 2, "2 rolls");
}

static void testLootTableRollEmpty() {
    std::cout << "\n=== LootTable: RollEmpty ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    assertTrue(sys.rollLoot("loot1", 5.0f).empty(), "Roll empty table returns empty");
    assertTrue(sys.rollLoot("nonexistent", 5.0f).empty(), "Roll missing returns empty");
}

static void testLootTableGetByRarity() {
    std::cout << "\n=== LootTable: GetByRarity ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    sys.addEntry("loot1", "iron", "common", 10.0f, 1, 5);
    sys.addEntry("loot1", "copper", "common", 8.0f, 1, 5);
    sys.addEntry("loot1", "gold", "rare", 2.0f, 1, 1);
    sys.addEntry("loot1", "diamond", "legendary", 0.5f, 1, 1);

    auto commons = sys.getEntriesByRarity("loot1", "common");
    assertTrue(static_cast<int>(commons.size()) == 2, "2 common entries");
    auto rares = sys.getEntriesByRarity("loot1", "rare");
    assertTrue(static_cast<int>(rares.size()) == 1, "1 rare entry");
    auto epics = sys.getEntriesByRarity("loot1", "epic");
    assertTrue(epics.empty(), "0 epic entries");
}

static void testLootTableUpdate() {
    std::cout << "\n=== LootTable: Update ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    world.createEntity("loot1");
    sys.initialize("loot1", "table_1");
    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testLootTableMissing() {
    std::cout << "\n=== LootTable: Missing ===" << std::endl;
    ecs::World world;
    systems::LootTableSystem sys(&world);
    assertTrue(sys.getEntryCount("x") == 0, "0 entries on missing");
    assertTrue(approxEqual(sys.getTotalWeight("x"), 0.0f), "0 weight on missing");
    assertTrue(sys.getTotalRolls("x") == 0, "0 rolls on missing");
    assertTrue(sys.getTotalDrops("x") == 0, "0 drops on missing");
    assertTrue(approxEqual(sys.getLuckModifier("x"), 1.0f), "Default luck on missing");
    assertTrue(sys.getTableId("x").empty(), "Empty table_id on missing");
    assertTrue(!sys.removeEntry("x", "item"), "Remove fails on missing");
}

void run_loot_table_system_tests() {
    testLootTableCreate();
    testLootTableInitValidation();
    testLootTableAddEntry();
    testLootTableAddEntryValidation();
    testLootTableDuplicateEntry();
    testLootTableRemoveEntry();
    testLootTableLuckModifier();
    testLootTableRollLoot();
    testLootTableRollEmpty();
    testLootTableGetByRarity();
    testLootTableUpdate();
    testLootTableMissing();
}
