// Tests for: BountySystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/bounty_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== BountySystem Tests ====================

static void testBountyProcessKill() {
    std::cout << "\n=== Bounty Process Kill ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    auto* pc = addComp<components::Player>(player);
    pc->credits = 100000.0;
    
    double bounty = bountySys.processKill("player_1", "npc_pirate_1", "Venom Scout", 12500.0, "Venom Syndicate");
    assertTrue(approxEqual(static_cast<float>(bounty), 12500.0f), "Bounty returned correctly");
    assertTrue(approxEqual(static_cast<float>(pc->credits), 112500.0f), "Credits increased by bounty");
    assertTrue(bountySys.getTotalKills("player_1") == 1, "Kill count is 1");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("player_1")), 12500.0f), "Total bounty correct");
}

static void testBountyMultipleKills() {
    std::cout << "\n=== Bounty Multiple Kills ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    auto* pc = addComp<components::Player>(player);
    pc->credits = 0.0;
    
    bountySys.processKill("player_1", "npc_1", "Scout", 10000.0);
    bountySys.processKill("player_1", "npc_2", "Cruiser", 50000.0);
    bountySys.processKill("player_1", "npc_3", "Battleship", 150000.0);
    
    assertTrue(bountySys.getTotalKills("player_1") == 3, "3 kills recorded");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("player_1")), 210000.0f), "Total bounty is 210K");
    assertTrue(approxEqual(static_cast<float>(pc->credits), 210000.0f), "Credits matches total bounty");
}

static void testBountyLedgerRecordLimit() {
    std::cout << "\n=== Bounty Ledger Record Limit ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    auto* player = world.createEntity("player_1");
    addComp<components::Player>(player);
    
    for (int i = 0; i < 60; ++i) {
        bountySys.processKill("player_1", "npc_" + std::to_string(i), "NPC " + std::to_string(i), 1000.0);
    }
    
    auto* ledger = player->getComponent<components::BountyLedger>();
    assertTrue(ledger != nullptr, "Ledger exists");
    assertTrue(static_cast<int>(ledger->recent_kills.size()) <= components::BountyLedger::MAX_RECENT,
               "Recent kills capped at MAX_RECENT");
    assertTrue(ledger->total_kills == 60, "Total kills tracks all 60");
}

static void testBountyNonexistentPlayer() {
    std::cout << "\n=== Bounty Nonexistent Player ===" << std::endl;
    ecs::World world;
    systems::BountySystem bountySys(&world);
    
    double bounty = bountySys.processKill("fake_player", "npc_1", "Scout", 10000.0);
    assertTrue(approxEqual(static_cast<float>(bounty), 0.0f), "No bounty for nonexistent player");
    assertTrue(bountySys.getTotalKills("fake_player") == 0, "Zero kills for nonexistent");
    assertTrue(approxEqual(static_cast<float>(bountySys.getTotalBounty("fake_player")), 0.0f), "Zero bounty for nonexistent");
}


void run_bounty_system_tests() {
    testBountyProcessKill();
    testBountyMultipleKills();
    testBountyLedgerRecordLimit();
    testBountyNonexistentPlayer();
}
