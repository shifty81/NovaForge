// Tests for: BountyCollection System Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/bounty_collection_system.h"

using namespace atlas;

// ==================== BountyCollection System Tests ====================

static void testBountyCreate() {
    std::cout << "\n=== BountyCollection: Create ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initializeBountyTracker("player1"), "Init bounty tracker succeeds");
    assertTrue(sys.getPendingCount("player1") == 0, "No pending bounties initially");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCollected("player1")), 0.0f), "0 total collected");
    assertTrue(sys.getTotalKillsClaimed("player1") == 0, "0 kills claimed");
}

static void testBountyRecordKill() {
    std::cout << "\n=== BountyCollection: RecordKill ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBountyTracker("player1");

    assertTrue(sys.recordKill("player1", "pirate1", "Pirate", 1000.0), "Record pirate kill");
    assertTrue(sys.getPendingCount("player1") == 1, "1 pending");
    assertTrue(sys.hasPendingBounties("player1"), "Has pending bounties");
}

static void testBountyPayout() {
    std::cout << "\n=== BountyCollection: Payout ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBountyTracker("player1");

    sys.recordKill("player1", "pirate1", "Pirate", 1000.0);
    sys.update(0.1f);

    assertTrue(sys.getPendingCount("player1") == 0, "No pending after payout");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCollected("player1")), 1000.0f), "1000 collected");
    assertTrue(sys.getTotalKillsClaimed("player1") == 1, "1 kill claimed");
}

static void testBountyMultipleKills() {
    std::cout << "\n=== BountyCollection: MultipleKills ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBountyTracker("player1");

    sys.recordKill("player1", "pirate1", "Pirate", 500.0);
    sys.recordKill("player1", "pirate2", "Elite", 2000.0);
    sys.recordKill("player1", "pirate3", "Frigate", 300.0);
    sys.update(0.1f);

    assertTrue(sys.getTotalKillsClaimed("player1") == 3, "3 kills claimed");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCollected("player1")), 2800.0f), "2800 total");
}

static void testBountyMaxPending() {
    std::cout << "\n=== BountyCollection: MaxPending ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBountyTracker("player1");
    auto* entity = world.getEntity("player1");
    auto* bc = entity->getComponent<components::BountyCollection>();
    bc->max_pending = 2;

    sys.recordKill("player1", "p1", "Pirate", 100.0);
    sys.recordKill("player1", "p2", "Pirate", 200.0);
    assertTrue(!sys.recordKill("player1", "p3", "Pirate", 300.0), "3rd kill rejected at max=2");
}

static void testBountyClear() {
    std::cout << "\n=== BountyCollection: Clear ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBountyTracker("player1");

    sys.recordKill("player1", "p1", "Pirate", 500.0);
    assertTrue(sys.clearPending("player1"), "Clear succeeds");
    assertTrue(sys.getPendingCount("player1") == 0, "0 pending after clear");
    assertTrue(!sys.hasPendingBounties("player1"), "No pending after clear");
}

static void testBountyDuplicateInit() {
    std::cout << "\n=== BountyCollection: DuplicateInit ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initializeBountyTracker("player1"), "First init succeeds");
    assertTrue(!sys.initializeBountyTracker("player1"), "Duplicate init rejected");
}

static void testBountyMissing() {
    std::cout << "\n=== BountyCollection: Missing ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    assertTrue(!sys.initializeBountyTracker("nonexistent"), "Init fails on missing");
    assertTrue(!sys.recordKill("nonexistent", "p1", "Pirate", 100.0), "Record fails on missing");
    assertTrue(sys.getPendingCount("nonexistent") == 0, "0 pending on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCollected("nonexistent")), 0.0f), "0 collected on missing");
    assertTrue(!sys.hasPendingBounties("nonexistent"), "No pending on missing");
    assertTrue(!sys.clearPending("nonexistent"), "Clear fails on missing");
}

static void testBountyInvalidInput() {
    std::cout << "\n=== BountyCollection: InvalidInput ===" << std::endl;
    ecs::World world;
    systems::BountyCollectionSystem sys(&world);
    world.createEntity("player1");
    sys.initializeBountyTracker("player1");

    assertTrue(!sys.recordKill("player1", "", "Pirate", 100.0), "Empty target rejected");
    assertTrue(!sys.recordKill("player1", "p1", "Pirate", 0.0), "Zero bounty rejected");
    assertTrue(!sys.recordKill("player1", "p1", "Pirate", -100.0), "Negative bounty rejected");
}

void run_bounty_collection_system_tests() {
    testBountyCreate();
    testBountyRecordKill();
    testBountyPayout();
    testBountyMultipleKills();
    testBountyMaxPending();
    testBountyClear();
    testBountyDuplicateInit();
    testBountyMissing();
    testBountyInvalidInput();
}
