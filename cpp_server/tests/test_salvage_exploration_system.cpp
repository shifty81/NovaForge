// Tests for: SalvageExplorationSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/salvage_exploration_system.h"

using namespace atlas;

// ==================== SalvageExplorationSystem Tests ====================

static void testSalvageDiscoverNode() {
    std::cout << "\n=== SalvageExploration: DiscoverNode ===" << std::endl;
    ecs::World world;
    systems::SalvageExplorationSystem sys(&world);
    auto* e = world.createEntity("site1");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 3;

    assertTrue(sys.getDiscoveredNodes("site1") == 0, "Zero discovered initially");
    assertTrue(sys.discoverNode("site1"), "Discover node 1");
    assertTrue(sys.getDiscoveredNodes("site1") == 1, "1 discovered");
    assertTrue(sys.discoverNode("site1"), "Discover node 2");
    assertTrue(sys.discoverNode("site1"), "Discover node 3");
    assertTrue(sys.getDiscoveredNodes("site1") == 3, "3 discovered");

    // Cannot discover beyond total
    assertTrue(!sys.discoverNode("site1"), "Cannot discover past total");
    assertTrue(sys.getDiscoveredNodes("site1") == 3, "Still 3 discovered");
}

static void testSalvageLootNode() {
    std::cout << "\n=== SalvageExploration: LootNode ===" << std::endl;
    ecs::World world;
    systems::SalvageExplorationSystem sys(&world);
    auto* e = world.createEntity("site1");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 3;

    // Cannot loot undiscovered
    assertTrue(!sys.lootNode("site1"), "Cannot loot before discovering");

    // Discover 2, loot 1
    sys.discoverNode("site1");
    sys.discoverNode("site1");
    assertTrue(sys.lootNode("site1"), "Loot node 1");
    assertTrue(sys.getRemainingNodes("site1") == 2, "2 remaining");
    assertTrue(sys.lootNode("site1"), "Loot node 2");
    assertTrue(sys.getRemainingNodes("site1") == 1, "1 remaining");

    // Cannot loot more than discovered
    assertTrue(!sys.lootNode("site1"), "Cannot loot undiscovered nodes");
}

static void testSalvageFullyLooted() {
    std::cout << "\n=== SalvageExploration: FullyLooted ===" << std::endl;
    ecs::World world;
    systems::SalvageExplorationSystem sys(&world);
    auto* e = world.createEntity("site1");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 2;

    assertTrue(!sys.isFullyLooted("site1"), "Not fully looted initially");

    sys.discoverNode("site1");
    sys.discoverNode("site1");
    sys.lootNode("site1");
    assertTrue(!sys.isFullyLooted("site1"), "Not fully looted with 1 remaining");

    sys.lootNode("site1");
    assertTrue(sys.isFullyLooted("site1"), "Fully looted when all looted");
    assertTrue(sys.getRemainingNodes("site1") == 0, "Zero remaining");
}

static void testSalvageZeroNodes() {
    std::cout << "\n=== SalvageExploration: ZeroNodes ===" << std::endl;
    ecs::World world;
    systems::SalvageExplorationSystem sys(&world);
    auto* e = world.createEntity("site1");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 0;

    assertTrue(!sys.discoverNode("site1"), "Cannot discover when total is 0");
    assertTrue(!sys.lootNode("site1"), "Cannot loot when total is 0");
    assertTrue(!sys.isFullyLooted("site1"), "Not fully looted when total is 0");
    assertTrue(sys.getRemainingNodes("site1") == 0, "Zero remaining");
}

static void testSalvageAncientTech() {
    std::cout << "\n=== SalvageExploration: AncientTech ===" << std::endl;
    ecs::World world;
    systems::SalvageExplorationSystem sys(&world);
    auto* e = world.createEntity("site1");
    auto* site = addComp<components::SalvageSite>(e);

    assertTrue(!sys.hasAncientTech("site1"), "No ancient tech by default");

    site->has_ancient_tech = true;
    assertTrue(sys.hasAncientTech("site1"), "Has ancient tech when set");
}

static void testSalvageGenerateTrinkets() {
    std::cout << "\n=== SalvageExploration: GenerateTrinkets ===" << std::endl;
    ecs::World world;
    systems::SalvageExplorationSystem sys(&world);
    auto* e = world.createEntity("site1");
    addComp<components::SalvageSite>(e);

    // Deterministic RNG — same seed produces same result
    int count1 = sys.generateTrinkets("site1", 42);
    int count2 = sys.generateTrinkets("site1", 42);
    assertTrue(count1 == count2, "Same seed gives same trinket count");
    assertTrue(count1 >= 0 && count1 <= 5, "Trinket count in range 0-5");

    // Different seeds may give different results
    int count3 = sys.generateTrinkets("site1", 123456789);
    assertTrue(count3 >= 0 && count3 <= 5, "Different seed in range 0-5");
}

static void testSalvageUpdate() {
    std::cout << "\n=== SalvageExploration: Update ===" << std::endl;
    ecs::World world;
    systems::SalvageExplorationSystem sys(&world);
    auto* e = world.createEntity("site1");
    auto* site = addComp<components::SalvageSite>(e);
    site->total_loot_nodes = 3;

    // Update should be a no-op
    sys.update(1.0f);
    assertTrue(sys.getDiscoveredNodes("site1") == 0, "Update does not change state");
    assertTrue(sys.getRemainingNodes("site1") == 3, "Remaining unchanged after update");
}

static void testSalvageMissing() {
    std::cout << "\n=== SalvageExploration: Missing ===" << std::endl;
    ecs::World world;
    systems::SalvageExplorationSystem sys(&world);

    assertTrue(!sys.discoverNode("x"), "Discover on missing fails");
    assertTrue(!sys.lootNode("x"), "Loot on missing fails");
    assertTrue(!sys.isFullyLooted("x"), "FullyLooted on missing");
    assertTrue(sys.getRemainingNodes("x") == 0, "Remaining on missing");
    assertTrue(sys.getDiscoveredNodes("x") == 0, "Discovered on missing");
    assertTrue(!sys.hasAncientTech("x"), "AncientTech on missing");
    assertTrue(sys.generateTrinkets("x", 42) == 0, "Trinkets on missing");
}

void run_salvage_exploration_system_tests() {
    testSalvageDiscoverNode();
    testSalvageLootNode();
    testSalvageFullyLooted();
    testSalvageZeroNodes();
    testSalvageAncientTech();
    testSalvageGenerateTrinkets();
    testSalvageUpdate();
    testSalvageMissing();
}
