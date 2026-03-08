// Tests for: FPSSalvagePathSystem Tests
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/fps_salvage_path_system.h"

using namespace atlas;

// ==================== FPSSalvagePathSystem Tests ====================

static void testFPSPInitAndDefaults() {
    std::cout << "\n=== FPSPSystem: Init and Defaults ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);

    world.createEntity("site_1");
    assertTrue(sys.initializePath("site_1", "wreck_01", "player_1", 5), "Path initialized");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 0.0f), "0% explored initially");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 0, "0 discovered initially");
    assertTrue(sys.getCollectedLootCount("site_1") == 0, "0 collected initially");

    assertTrue(!sys.initializePath("site_1", "wreck_01", "player_1", 5), "Duplicate init fails");

    world.createEntity("site_2");
    assertTrue(sys.initializePath("site_2", "wreck_02", "player_2", 3), "Second path initialized");
}

static void testFPSPEntryPointManagement() {
    std::cout << "\n=== FPSPSystem: Entry Point Management ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);

    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 5);

    assertTrue(sys.addEntryPoint("site_1", "hatch_1", 10.0f, "cutter"), "Entry point added");
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "sealed", "Entry is sealed");
    assertTrue(sys.addEntryPoint("site_1", "hatch_2", 5.0f, "laser"), "Second entry added");
    assertTrue(sys.getEntryState("site_1", "hatch_2") == "sealed", "Second entry is sealed");

    assertTrue(sys.startCutting("site_1", "hatch_1"), "Cutting started on hatch_1");
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "cutting", "hatch_1 is cutting");
    assertTrue(!sys.startCutting("site_1", "hatch_1"), "Can't restart cutting on hatch_1");
}

static void testFPSPCuttingProgress() {
    std::cout << "\n=== FPSPSystem: Cutting Progress ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);

    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addEntryPoint("site_1", "hatch_1", 5.0f, "cutter");
    sys.startCutting("site_1", "hatch_1");
    assertTrue(sys.getEntryState("site_1", "hatch_1") == "cutting", "State is cutting");

    sys.setActive("site_1", true);
    for (int i = 0; i < 10; ++i) sys.update(1.0f);

    assertTrue(sys.getEntryState("site_1", "hatch_1") == "open", "Entry is open after cutting");
    assertTrue(!sys.startCutting("site_1", "hatch_1"), "Can't cut an already open entry");
}

static void testFPSPRoomExploration() {
    std::cout << "\n=== FPSPSystem: Room Exploration ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);

    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 4);

    assertTrue(sys.exploreRoom("site_1"), "Room 1 explored");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 0.25f), "25% explored");

    assertTrue(sys.exploreRoom("site_1"), "Room 2 explored");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 0.5f), "50% explored");

    assertTrue(sys.exploreRoom("site_1"), "Room 3 explored");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 0.75f), "75% explored");

    assertTrue(sys.exploreRoom("site_1"), "Room 4 explored");
    assertTrue(approxEqual(sys.getExplorationProgress("site_1"), 1.0f), "100% explored");

    assertTrue(!sys.exploreRoom("site_1"), "Can't explore beyond total rooms");
}

static void testFPSPLootManagement() {
    std::cout << "\n=== FPSPSystem: Loot Management ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);

    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);

    assertTrue(sys.addLootNode("site_1", "loot_1", "Tritanium Plate",
        components::FPSSalvagePath::LootRarity::Common, 100.0f), "Common loot added");
    assertTrue(sys.addLootNode("site_1", "loot_2", "Ancient Relic",
        components::FPSSalvagePath::LootRarity::Legendary, 10000.0f), "Legendary loot added");
    assertTrue(sys.addLootNode("site_1", "loot_3", "Circuit Board",
        components::FPSSalvagePath::LootRarity::Uncommon, 200.0f), "Uncommon loot added");

    assertTrue(sys.getDiscoveredLootCount("site_1") == 0, "0 discovered before discovery");

    assertTrue(sys.discoverLoot("site_1", "loot_1"), "Loot 1 discovered");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 1, "1 discovered");
    assertTrue(sys.discoverLoot("site_1", "loot_2"), "Loot 2 discovered");
    assertTrue(sys.getDiscoveredLootCount("site_1") == 2, "2 discovered");

    assertTrue(sys.collectLoot("site_1", "loot_1"), "Loot 1 collected");
    assertTrue(sys.getCollectedLootCount("site_1") == 1, "1 collected");
    assertTrue(sys.collectLoot("site_1", "loot_2"), "Loot 2 collected");
    assertTrue(sys.getCollectedLootCount("site_1") == 2, "2 collected");
}

static void testFPSPLootStateValidation() {
    std::cout << "\n=== FPSPSystem: Loot State Validation ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);

    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);
    sys.addLootNode("site_1", "loot_1", "Power Core",
        components::FPSSalvagePath::LootRarity::Epic, 2000.0f);

    assertTrue(!sys.collectLoot("site_1", "loot_1"), "Can't collect undiscovered loot");

    assertTrue(sys.discoverLoot("site_1", "loot_1"), "Loot discovered");
    assertTrue(!sys.discoverLoot("site_1", "loot_1"), "Can't re-discover loot");

    assertTrue(sys.collectLoot("site_1", "loot_1"), "Collected discovered loot");
    assertTrue(!sys.collectLoot("site_1", "loot_1"), "Can't re-collect loot");

    assertTrue(!sys.discoverLoot("site_1", "nonexistent_loot"), "Discover nonexistent loot fails");
    assertTrue(!sys.collectLoot("site_1", "nonexistent_loot"), "Collect nonexistent loot fails");
}

static void testFPSPDuplicateEntry() {
    std::cout << "\n=== FPSPSystem: Duplicate Entry ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);

    world.createEntity("site_1");
    sys.initializePath("site_1", "wreck_01", "player_1", 3);

    assertTrue(sys.addEntryPoint("site_1", "hatch_1", 10.0f, "cutter"), "First entry added");
    assertTrue(!sys.addEntryPoint("site_1", "hatch_1", 5.0f, "laser"), "Duplicate entry rejected");

    assertTrue(!sys.addLootNode("site_1", "loot_1", "Item A",
        components::FPSSalvagePath::LootRarity::Common, 50.0f) ||
        !sys.addLootNode("site_1", "loot_1", "Item B",
            components::FPSSalvagePath::LootRarity::Rare, 500.0f),
        "Duplicate loot_id rejected");
}

static void testFPSPMissingEntity() {
    std::cout << "\n=== FPSPSystem: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FPSSalvagePathSystem sys(&world);

    assertTrue(!sys.initializePath("nonexistent", "s", "p", 5), "Init fails on missing");
    assertTrue(!sys.addEntryPoint("nonexistent", "e", 5.0f, "c"), "Entry fails on missing");
    assertTrue(!sys.startCutting("nonexistent", "e"), "Cutting fails on missing");
    assertTrue(!sys.exploreRoom("nonexistent"), "Explore fails on missing");
    assertTrue(!sys.setActive("nonexistent", true), "SetActive fails on missing");
    assertTrue(!sys.addLootNode("nonexistent", "l", "n",
        components::FPSSalvagePath::LootRarity::Common, 1.0f), "AddLoot fails on missing");
    assertTrue(!sys.discoverLoot("nonexistent", "l"), "Discover fails on missing");
    assertTrue(!sys.collectLoot("nonexistent", "l"), "Collect fails on missing");
    assertTrue(approxEqual(sys.getExplorationProgress("nonexistent"), 0.0f), "0% on missing");
    assertTrue(sys.getDiscoveredLootCount("nonexistent") == 0, "0 discovered on missing");
    assertTrue(sys.getCollectedLootCount("nonexistent") == 0, "0 collected on missing");
    assertTrue(sys.getEntryState("nonexistent", "e") == "unknown", "Unknown state on missing");
}


void run_fps_salvage_path_system_tests() {
    testFPSPInitAndDefaults();
    testFPSPEntryPointManagement();
    testFPSPCuttingProgress();
    testFPSPRoomExploration();
    testFPSPLootManagement();
    testFPSPLootStateValidation();
    testFPSPDuplicateEntry();
    testFPSPMissingEntity();
}
