// Tests for: Bounty Payout System
#include "test_log.h"
#include "components/core_components.h"
#include "components/combat_components.h"
#include "ecs/system.h"
#include "systems/bounty_payout_system.h"

using namespace atlas;

// ==================== Bounty Payout System Tests ====================

static void testBountyPayoutCreate() {
    std::cout << "\n=== BountyPayout: Create ===" << std::endl;
    ecs::World world;
    systems::BountyPayoutSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initialize("player1"), "Init succeeds");
    assertTrue(sys.getPendingCount("player1") == 0, "No pending payouts");
    assertTrue(sys.getTotalIscPaid("player1") == 0.0, "0 ISC paid");
    assertTrue(sys.getTotalPayoutsProcessed("player1") == 0, "0 processed");
    assertTrue(approxEqual(sys.getPayoutMultiplier("player1"), 1.0f), "Default multiplier 1.0");
}

static void testBountyPayoutRecordKill() {
    std::cout << "\n=== BountyPayout: RecordKill ===" << std::endl;
    ecs::World world;
    systems::BountyPayoutSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1");

    assertTrue(sys.recordKill("player1", "player1", "pirate_01", "pirate", 1000.0),
               "Record kill succeeds");
    assertTrue(sys.getPendingCount("player1") == 1, "1 pending");

    // Process payouts
    sys.update(1.0f);
    assertTrue(sys.getPendingCount("player1") == 0, "0 pending after processing");
    assertTrue(sys.getTotalIscPaid("player1") == 1000.0, "1000 ISC paid");
    assertTrue(sys.getTotalPayoutsProcessed("player1") == 1, "1 processed");
}

static void testBountyPayoutMultiplier() {
    std::cout << "\n=== BountyPayout: Multiplier ===" << std::endl;
    ecs::World world;
    systems::BountyPayoutSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1");
    sys.setPayoutMultiplier("player1", 1.5f);

    sys.recordKill("player1", "player1", "drone_01", "rogue_drone", 2000.0);
    sys.update(1.0f);
    assertTrue(sys.getTotalIscPaid("player1") == 3000.0, "2000 * 1.5 = 3000 ISC");
}

static void testBountyPayoutMultipleKills() {
    std::cout << "\n=== BountyPayout: MultipleKills ===" << std::endl;
    ecs::World world;
    systems::BountyPayoutSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1");

    sys.recordKill("player1", "player1", "pirate_01", "pirate", 500.0);
    sys.recordKill("player1", "player1", "pirate_02", "pirate", 750.0);
    sys.recordKill("player1", "player1", "pirate_03", "pirate", 1000.0);
    assertTrue(sys.getPendingCount("player1") == 3, "3 pending");

    sys.update(1.0f);
    assertTrue(sys.getPendingCount("player1") == 0, "All processed");
    assertTrue(sys.getTotalPayoutsProcessed("player1") == 3, "3 processed");
    assertTrue(sys.getTotalIscPaid("player1") == 2250.0, "Total 2250 ISC");
}

static void testBountyPayoutMaxPending() {
    std::cout << "\n=== BountyPayout: MaxPending ===" << std::endl;
    ecs::World world;
    systems::BountyPayoutSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1");

    // Fill pending queue to max (100)
    for (int i = 0; i < 100; i++) {
        assertTrue(sys.recordKill("player1", "player1", "npc_" + std::to_string(i),
                                  "pirate", 10.0), "Record kill " + std::to_string(i));
    }
    assertTrue(sys.getPendingCount("player1") == 100, "100 pending");
    assertTrue(!sys.recordKill("player1", "player1", "npc_overflow", "pirate", 10.0),
               "Overflow rejected");
}

static void testBountyPayoutMissing() {
    std::cout << "\n=== BountyPayout: Missing ===" << std::endl;
    ecs::World world;
    systems::BountyPayoutSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.recordKill("nonexistent", "p", "v", "type", 100.0), "RecordKill fails on missing");
    assertTrue(!sys.setPayoutMultiplier("nonexistent", 1.0f), "SetMultiplier fails on missing");
    assertTrue(sys.getPendingCount("nonexistent") == 0, "0 pending on missing");
    assertTrue(sys.getTotalIscPaid("nonexistent") == 0.0, "0 ISC on missing");
    assertTrue(sys.getTotalPayoutsProcessed("nonexistent") == 0, "0 processed on missing");
    assertTrue(approxEqual(sys.getPayoutMultiplier("nonexistent"), 0.0f), "0 multiplier on missing");
}

void run_bounty_payout_system_tests() {
    testBountyPayoutCreate();
    testBountyPayoutRecordKill();
    testBountyPayoutMultiplier();
    testBountyPayoutMultipleKills();
    testBountyPayoutMaxPending();
    testBountyPayoutMissing();
}
