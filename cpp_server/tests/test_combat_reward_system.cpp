// Tests for: CombatReward System Tests
#include "test_log.h"
#include "components/combat_components.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/combat_reward_system.h"

using namespace atlas;

// ==================== CombatReward System Tests ====================

static void testCombatRewardCreate() {
    std::cout << "\n=== CombatReward: Create ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initializeRewards("player1", 2.0f), "Init rewards succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCredits("player1")), 0.0f), "0 credits initially");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 0.0f), "0 XP initially");
    assertTrue(sys.getTotalKills("player1") == 0, "0 kills initially");
    assertTrue(sys.getPendingCount("player1") == 0, "0 pending initially");
}

static void testCombatRewardRecordKill() {
    std::cout << "\n=== CombatReward: Record Kill ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    world.createEntity("player1");
    sys.initializeRewards("player1");

    assertTrue(sys.recordKill("player1", "npc_1", "Pirate Scout", 50.0f, 1000.0, "common"),
               "Record kill succeeds");
    assertTrue(sys.getTotalKills("player1") == 1, "1 kill recorded");
    assertTrue(sys.getPendingCount("player1") == 1, "1 pending reward");
}

static void testCombatRewardInvalidKill() {
    std::cout << "\n=== CombatReward: Invalid Kill ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    world.createEntity("player1");
    sys.initializeRewards("player1");

    assertTrue(!sys.recordKill("player1", "", "Empty", 10.0f, 100.0), "Empty target rejected");
    assertTrue(!sys.recordKill("nonexistent", "npc_1", "Test", 10.0f, 100.0), "Missing entity rejected");
}

static void testCombatRewardManualFlush() {
    std::cout << "\n=== CombatReward: Manual Flush ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    world.createEntity("player1");
    sys.initializeRewards("player1", 60.0f);  // Long interval so auto-flush won't fire

    sys.recordKill("player1", "npc_1", "Pirate Scout", 50.0f, 1000.0);
    sys.recordKill("player1", "npc_2", "Pirate Raider", 75.0f, 1500.0);

    int flushed = sys.flushRewards("player1");
    assertTrue(flushed == 2, "2 rewards flushed");
    assertTrue(sys.getPendingCount("player1") == 0, "0 pending after flush");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCredits("player1")), 2500.0f), "2500 credits total");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 125.0f), "125 XP total");
}

static void testCombatRewardAutoFlush() {
    std::cout << "\n=== CombatReward: Auto Flush ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    world.createEntity("player1");
    sys.initializeRewards("player1", 1.0f);  // 1 second flush interval

    sys.recordKill("player1", "npc_1", "Pirate", 100.0f, 500.0);
    assertTrue(sys.getPendingCount("player1") == 1, "1 pending before auto-flush");

    // Simulate 1.1 seconds
    for (int i = 0; i < 11; ++i) sys.update(0.1f);

    assertTrue(sys.getPendingCount("player1") == 0, "0 pending after auto-flush");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCredits("player1")), 500.0f), "500 credits after auto-flush");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 100.0f), "100 XP after auto-flush");
}

static void testCombatRewardMultipleKills() {
    std::cout << "\n=== CombatReward: Multiple Kills ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    world.createEntity("player1");
    sys.initializeRewards("player1", 0.5f);

    for (int i = 0; i < 5; ++i) {
        sys.recordKill("player1", "npc_" + std::to_string(i),
                       "Pirate " + std::to_string(i), 20.0f, 200.0);
    }

    assertTrue(sys.getTotalKills("player1") == 5, "5 kills recorded");
    assertTrue(sys.getPendingCount("player1") == 5, "5 pending rewards");

    // Auto-flush after 0.5s
    for (int i = 0; i < 5; ++i) sys.update(0.1f);

    assertTrue(sys.getPendingCount("player1") == 0, "All flushed");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCredits("player1")), 1000.0f), "1000 total credits");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 100.0f), "100 total XP");
}

static void testCombatRewardDuplicateInit() {
    std::cout << "\n=== CombatReward: Duplicate Init ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initializeRewards("player1"), "First init succeeds");
    assertTrue(!sys.initializeRewards("player1"), "Duplicate init rejected");
}

static void testCombatRewardDoubleFlush() {
    std::cout << "\n=== CombatReward: Double Flush ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    world.createEntity("player1");
    sys.initializeRewards("player1", 60.0f);

    sys.recordKill("player1", "npc_1", "Test", 10.0f, 100.0);
    sys.flushRewards("player1");
    int second = sys.flushRewards("player1");
    assertTrue(second == 0, "Second flush returns 0 (already flushed)");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalCredits("player1")), 100.0f), "Credits not doubled");
}

static void testCombatRewardMissingEntity() {
    std::cout << "\n=== CombatReward: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::CombatRewardSystem sys(&world);
    assertTrue(!sys.initializeRewards("nonexistent"), "Init fails for missing entity");
    assertTrue(sys.getTotalKills("nonexistent") == 0, "0 kills for missing");
    assertTrue(sys.getPendingCount("nonexistent") == 0, "0 pending for missing");
    assertTrue(approxEqual(sys.getTotalXP("nonexistent"), 0.0f), "0 XP for missing");
}

void run_combat_reward_system_tests() {
    testCombatRewardCreate();
    testCombatRewardRecordKill();
    testCombatRewardInvalidKill();
    testCombatRewardManualFlush();
    testCombatRewardAutoFlush();
    testCombatRewardMultipleKills();
    testCombatRewardDuplicateInit();
    testCombatRewardDoubleFlush();
    testCombatRewardMissingEntity();
}
