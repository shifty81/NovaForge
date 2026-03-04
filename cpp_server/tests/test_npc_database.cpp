// Tests for: NpcDatabase Tests
#include "test_log.h"
#include "components/core_components.h"
#include "data/npc_database.h"
#include "data/ship_database.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== NpcDatabase Tests ====================

static void testNpcDatabaseLoad() {
    std::cout << "\n=== NpcDatabase Load ===" << std::endl;

    data::NpcDatabase npcDb;

    // Try multiple paths (same strategy as ShipDatabase tests)
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    assertTrue(loaded > 0, "NpcDatabase loaded NPCs from directory");
    assertTrue(npcDb.getNpcCount() >= 32, "At least 32 NPC templates loaded");
}

static void testNpcDatabaseGetNpc() {
    std::cout << "\n=== NpcDatabase GetNpc ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "venom_syndicate_scout found");
    assertTrue(scout->name == "Venom Syndicate Scout", "NPC name correct");
    assertTrue(scout->type == "frigate", "NPC type correct");
    assertTrue(scout->faction == "Venom Syndicate", "NPC faction correct");
}

static void testNpcDatabaseHpValues() {
    std::cout << "\n=== NpcDatabase HP Values ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for HP test");
    assertTrue(approxEqual(scout->hull_hp, 300.0f), "Hull HP is 300");
    assertTrue(approxEqual(scout->armor_hp, 250.0f), "Armor HP is 250");
    assertTrue(approxEqual(scout->shield_hp, 350.0f), "Shield HP is 350");
    assertTrue(approxEqual(static_cast<float>(scout->bounty), 12500.0f), "Bounty is 12500");
}

static void testNpcDatabaseWeapons() {
    std::cout << "\n=== NpcDatabase Weapons ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for weapons test");
    assertTrue(!scout->weapons.empty(), "Scout has weapons");
    assertTrue(scout->weapons[0].type == "small_hybrid", "Weapon type is small_hybrid");
    assertTrue(approxEqual(scout->weapons[0].damage, 28.0f), "Weapon damage is 28");
    assertTrue(scout->weapons[0].damage_type == "kinetic", "Weapon damage type is kinetic");
    assertTrue(approxEqual(scout->weapons[0].rate_of_fire, 4.5f), "Rate of fire is 4.5");
}

static void testNpcDatabaseResistances() {
    std::cout << "\n=== NpcDatabase Resistances ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    const data::NpcTemplate* scout = npcDb.getNpc("venom_syndicate_scout");
    assertTrue(scout != nullptr, "Scout found for resistances test");

    // Shield: em=0, thermal=60, kinetic=85, explosive=50 -> /100
    assertTrue(approxEqual(scout->shield_resists.em, 0.0f), "Shield EM resist is 0.0");
    assertTrue(approxEqual(scout->shield_resists.thermal, 0.60f), "Shield thermal resist is 0.60");
    assertTrue(approxEqual(scout->shield_resists.kinetic, 0.85f), "Shield kinetic resist is 0.85");
    assertTrue(approxEqual(scout->shield_resists.explosive, 0.50f), "Shield explosive resist is 0.50");

    // Armor: em=10, thermal=35, kinetic=25, explosive=45 -> /100
    assertTrue(approxEqual(scout->armor_resists.em, 0.10f), "Armor EM resist is 0.10");
    assertTrue(approxEqual(scout->armor_resists.kinetic, 0.25f), "Armor kinetic resist is 0.25");
}

static void testNpcDatabaseIds() {
    std::cout << "\n=== NpcDatabase IDs ===" << std::endl;

    data::NpcDatabase npcDb;
    int loaded = npcDb.loadFromDirectory("../data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("data");
    if (loaded == 0) loaded = npcDb.loadFromDirectory("../../data");

    auto ids = npcDb.getNpcIds();
    assertTrue(!ids.empty(), "getNpcIds returns non-empty list");
    assertTrue(ids.size() == npcDb.getNpcCount(), "IDs count matches getNpcCount");
}

static void testNpcDatabaseNonexistent() {
    std::cout << "\n=== NpcDatabase Nonexistent ===" << std::endl;

    data::NpcDatabase npcDb;
    npcDb.loadFromDirectory("../data");

    const data::NpcTemplate* result = npcDb.getNpc("totally_fake_npc");
    assertTrue(result == nullptr, "Nonexistent NPC returns nullptr");
}


void run_npc_database_tests() {
    testNpcDatabaseLoad();
    testNpcDatabaseGetNpc();
    testNpcDatabaseHpValues();
    testNpcDatabaseWeapons();
    testNpcDatabaseResistances();
    testNpcDatabaseIds();
    testNpcDatabaseNonexistent();
}
