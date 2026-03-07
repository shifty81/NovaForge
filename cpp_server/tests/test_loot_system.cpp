// Tests for: Loot System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/narrative_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/loot_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Loot System Tests ====================

static void testLootGenerate() {
    std::cout << "\n=== Loot Generate ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(42);

    auto* npc = world.createEntity("pirate1");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isc_drop = 15000.0;

    components::LootTable::LootEntry entry1;
    entry1.item_id     = "scrap_metal";
    entry1.name        = "Scrap Metal";
    entry1.type        = "salvage";
    entry1.drop_chance = 1.0f;  // always drops
    entry1.min_quantity = 1;
    entry1.max_quantity = 5;
    entry1.volume      = 1.0f;
    lt->entries.push_back(entry1);

    components::LootTable::LootEntry entry2;
    entry2.item_id     = "rare_module";
    entry2.name        = "Rare Module";
    entry2.type        = "module";
    entry2.drop_chance = 1.0f;  // always drops for testing
    entry2.min_quantity = 1;
    entry2.max_quantity = 1;
    entry2.volume      = 5.0f;
    lt->entries.push_back(entry2);

    std::string wreck_id = lootSys.generateLoot("pirate1");
    assertTrue(!wreck_id.empty(), "Wreck entity created");

    auto* wreck = world.getEntity(wreck_id);
    assertTrue(wreck != nullptr, "Wreck entity exists in world");

    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv != nullptr, "Wreck has Inventory component");
    assertTrue(wreck_inv->items.size() >= 1, "Wreck has at least one item");

    auto* wreck_lt = wreck->getComponent<components::LootTable>();
    assertTrue(wreck_lt != nullptr, "Wreck has LootTable for Credits");
    assertTrue(approxEqual(static_cast<float>(wreck_lt->isc_drop), 15000.0f),
               "Credits bounty preserved on wreck");
}

static void testLootCollect() {
    std::cout << "\n=== Loot Collect ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(42);

    // Create NPC with loot
    auto* npc = world.createEntity("pirate2");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isc_drop = 25000.0;

    components::LootTable::LootEntry entry;
    entry.item_id     = "hybrid_charges";
    entry.name        = "Hybrid Charges";
    entry.type        = "ammo";
    entry.drop_chance = 1.0f;
    entry.min_quantity = 10;
    entry.max_quantity = 10;
    entry.volume      = 0.01f;
    lt->entries.push_back(entry);

    std::string wreck_id = lootSys.generateLoot("pirate2");

    // Create player
    auto* player = world.createEntity("player1");
    auto* player_inv = addComp<components::Inventory>(player);
    player_inv->max_capacity = 400.0f;
    auto* player_comp = addComp<components::Player>(player);
    player_comp->credits = 100000.0;

    bool collected = lootSys.collectLoot(wreck_id, "player1");
    assertTrue(collected, "Loot collected successfully");
    assertTrue(player_inv->items.size() >= 1, "Player received items");
    assertTrue(approxEqual(static_cast<float>(player_comp->credits), 125000.0f),
               "Player Credits increased by bounty");
}

static void testLootEmptyTable() {
    std::cout << "\n=== Loot Empty Table ===" << std::endl;

    ecs::World world;
    systems::LootSystem lootSys(&world);
    lootSys.setRandomSeed(99);

    auto* npc = world.createEntity("pirate3");
    auto* lt = addComp<components::LootTable>(npc);
    lt->isc_drop = 0.0;
    // No entries

    std::string wreck_id = lootSys.generateLoot("pirate3");
    assertTrue(!wreck_id.empty(), "Wreck created even with empty loot table");

    auto* wreck = world.getEntity(wreck_id);
    auto* wreck_inv = wreck->getComponent<components::Inventory>();
    assertTrue(wreck_inv->items.empty(), "Wreck has no items from empty table");
}


void run_loot_system_tests() {
    testLootGenerate();
    testLootCollect();
    testLootEmptyTable();
}
