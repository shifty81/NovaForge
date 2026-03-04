// Tests for: Salvage Exploration Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/salvage_exploration_system.h"

using namespace atlas;

// ==================== Salvage Exploration Tests ====================

static void testSalvageSiteDefaults() {
    std::cout << "\n=== Salvage Site Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site1");
    auto* site = addComp<components::SalvageSite>(e);
    assertTrue(site->discovered_nodes == 0, "No discovered nodes");
    assertTrue(site->looted_nodes == 0, "No looted nodes");
    assertTrue(site->total_loot_nodes == 0, "No total nodes");
}

static void testSalvageDiscover() {
    std::cout << "\n=== Salvage Discover ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site2");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 5;

    systems::SalvageExplorationSystem sys(&world);
    bool result = sys.discoverNode("site2");
    assertTrue(result, "Discover succeeded");
    assertTrue(sys.getDiscoveredNodes("site2") == 1, "Discovered 1 node");
}

static void testSalvageLoot() {
    std::cout << "\n=== Salvage Loot ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site3");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 5;

    systems::SalvageExplorationSystem sys(&world);
    sys.discoverNode("site3");
    bool result = sys.lootNode("site3");
    assertTrue(result, "Loot succeeded");
    assertTrue(site->looted_nodes == 1, "Looted 1 node");

    // Can't loot more than discovered
    bool result2 = sys.lootNode("site3");
    assertTrue(!result2, "Can't loot undiscovered");
}

static void testSalvageFullyLooted() {
    std::cout << "\n=== Salvage Fully Looted ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site4");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 2;

    systems::SalvageExplorationSystem sys(&world);
    sys.discoverNode("site4");
    sys.discoverNode("site4");
    sys.lootNode("site4");
    sys.lootNode("site4");
    assertTrue(sys.isFullyLooted("site4"), "Fully looted");
    assertTrue(sys.getRemainingNodes("site4") == 0, "0 remaining");
}

static void testSalvageTrinkets() {
    std::cout << "\n=== Salvage Trinkets ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("site5");
    addComp<components::SalvageSite>(e);

    systems::SalvageExplorationSystem sys(&world);
    int count = sys.generateTrinkets("site5", 42);
    assertTrue(count >= 0 && count <= 5, "Trinket count 0-5");
}


void run_salvage_exploration_tests() {
    testSalvageSiteDefaults();
    testSalvageDiscover();
    testSalvageLoot();
    testSalvageFullyLooted();
    testSalvageTrinkets();
}
