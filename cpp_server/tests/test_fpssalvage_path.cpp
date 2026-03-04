// Tests for: FPSSalvagePath Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fps_salvage_path_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== FPSSalvagePath Tests ====================

static void testSalvagePathInit() {
    std::cout << "\n=== FPSSalvagePath: Init ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    assertTrue(sys.initializePath("site_1", "wreck_01", "player_1", 5), "Path initialized");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 0.0f), "0% explored initially");
    assertTrue(!sys.initializePath("site_1", "wreck_01", "player_1", 5), "Duplicate init fails");
}

static void testSalvagePathEntry() {
    std::cout << "\n=== FPSSalvagePath: Entry Points ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 5);
    assertTrue(sys.addEntryPoint("site_1", "hatch_1", 10.0f, "cutter"), "Entry added");
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "sealed", "Entry is sealed");
    assertTrue(!sys.addEntryPoint("site_1", "hatch_1", 5.0f, "cutter"), "Duplicate entry rejected");
}

static void testSalvagePathCutting() {
    std::cout << "\n=== FPSSalvagePath: Cutting ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addEntryPoint("site_1", "hatch_1", 5.0f, "cutter");
    assertTrue(sys.startCutting("site_1", "hatch_1"), "Cutting started");
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "cutting", "State is cutting");
    assertTrue(!sys.startCutting("site_1", "hatch_1"), "Can't restart cutting");
    sys.setActive("site_1", true);
    for (int i = 0; i < 10; i++) sys.update(1.0f);
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "open", "Entry is open after cutting");
}

static void testSalvagePathExploration() {
    std::cout << "\n=== FPSSalvagePath: Exploration ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 4);
    assertTrue(sys.exploreRoom("site_1"), "Room 1 explored");
    assertTrue(sys.exploreRoom("site_1"), "Room 2 explored");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 0.5f), "50% explored");
    sys.exploreRoom("site_1");
    sys.exploreRoom("site_1");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 1.0f), "100% explored");
    assertTrue(!sys.exploreRoom("site_1"), "Can't explore beyond total");
}

static void testSalvagePathLoot() {
    std::cout << "\n=== FPSSalvagePath: Loot Nodes ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    assertTrue(sys.addLootNode("site_1", "loot_1", "Tritanium Plate",
        components::FPSSalvagePath::LootRarity::Common, 100.0f), "Loot added");
    assertTrue(!sys.addLootNode("site_1", "loot_1", "Dup",
        components::FPSSalvagePath::LootRarity::Common, 50.0f), "Duplicate loot rejected");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 0, "0 discovered initially");
}

static void testSalvagePathDiscover() {
    std::cout << "\n=== FPSSalvagePath: Discover Loot ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addLootNode("site_1", "loot_1", "Armor Plate",
        components::FPSSalvagePath::LootRarity::Rare, 500.0f);
    assertTrue(sys.discoverLoot("site_1", "loot_1"), "Loot discovered");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 1, "1 discovered");
    assertTrue(!sys.discoverLoot("site_1", "loot_1"), "Can't re-discover");
}

static void testSalvagePathCollect() {
    std::cout << "\n=== FPSSalvagePath: Collect Loot ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addLootNode("site_1", "loot_1", "Power Core",
        components::FPSSalvagePath::LootRarity::Epic, 2000.0f);
    assertTrue(!sys.collectLoot("site_1", "loot_1"), "Can't collect undiscovered");
    sys.discoverLoot("site_1", "loot_1");
    assertTrue(sys.collectLoot("site_1", "loot_1"), "Collected discovered loot");
    assertTrue(sys.getCollectedLootCount("site_1") == 1, "1 collected");
    assertTrue(!sys.collectLoot("site_1", "loot_1"), "Can't re-collect");
}

static void testSalvagePathMultipleLoot() {
    std::cout << "\n=== FPSSalvagePath: Multiple Loot ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 5);
    sys.addLootNode("site_1", "loot_1", "Scrap Metal",
        components::FPSSalvagePath::LootRarity::Common, 50.0f);
    sys.addLootNode("site_1", "loot_2", "Ancient Relic",
        components::FPSSalvagePath::LootRarity::Legendary, 10000.0f);
    sys.addLootNode("site_1", "loot_3", "Circuit Board",
        components::FPSSalvagePath::LootRarity::Uncommon, 200.0f);
    sys.discoverLoot("site_1", "loot_1");
    sys.discoverLoot("site_1", "loot_2");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 2, "2 discovered");
    sys.collectLoot("site_1", "loot_1");
    assertTrue(sys.getCollectedLootCount("site_1") == 1, "1 collected");
}

static void testSalvagePathActive() {
    std::cout << "\n=== FPSSalvagePath: Active State ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addEntryPoint("site_1", "hatch_1", 3.0f, "cutter");
    sys.startCutting("site_1", "hatch_1");
    sys.update(1.0f);
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "cutting", "Still cutting when inactive");
    sys.setActive("site_1", true);
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "open", "Opens when active");
}

static void testSalvagePathMissing() {
    std::cout << "\n=== FPSSalvagePath: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);
    assertTrue(!sys.initializePath("nonexistent", "s", "p", 5), "Init fails on missing");
    assertTrue(!sys.addEntryPoint("nonexistent", "e", 5.0f, "c"), "Entry fails on missing");
    assertTrue(approxEqual(sys.getExplorationProgress("nonexistent"), 0.0f), "0% on missing");
    assertTrue(sys.getDiscoveredLootCount("nonexistent") == 0, "0 discovered on missing");
    assertTrue(sys.getEntryState("nonexistent", "e") == "unknown", "Unknown state on missing");
}


void run_fpssalvage_path_tests() {
    testSalvagePathInit();
    testSalvagePathEntry();
    testSalvagePathCutting();
    testSalvagePathExploration();
    testSalvagePathLoot();
    testSalvagePathDiscover();
    testSalvagePathCollect();
    testSalvagePathMultipleLoot();
    testSalvagePathActive();
    testSalvagePathMissing();
}
