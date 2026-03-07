// Tests for: LeaderboardSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/social_components.h"
#include "ecs/system.h"
#include "systems/leaderboard_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== LeaderboardSystem Tests ====================

static void testLeaderboardRecordKill() {
    std::cout << "\n=== Leaderboard Record Kill ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p1", "Alice");

    assertTrue(lbSys.getPlayerKills("board_1", "p1") == 3, "Player has 3 kills");
    assertTrue(lbSys.getEntryCount("board_1") == 1, "One entry on board");
}

static void testLeaderboardMultiplePlayers() {
    std::cout << "\n=== Leaderboard Multiple Players ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    lbSys.recordKill("board_1", "p2", "Bob");
    lbSys.recordKill("board_1", "p1", "Alice");

    assertTrue(lbSys.getEntryCount("board_1") == 2, "Two entries on board");
    assertTrue(lbSys.getPlayerKills("board_1", "p1") == 2, "Alice has 2 kills");
    assertTrue(lbSys.getPlayerKills("board_1", "p2") == 1, "Bob has 1 kill");
}

static void testLeaderboardIscTracking() {
    std::cout << "\n=== Leaderboard Credits Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordIscEarned("board_1", "p1", "Alice", 50000.0);
    lbSys.recordIscEarned("board_1", "p1", "Alice", 25000.0);

    assertTrue(approxEqual(static_cast<float>(lbSys.getPlayerIscEarned("board_1", "p1")), 75000.0f), "Credits earned is 75K");
}

static void testLeaderboardMissionTracking() {
    std::cout << "\n=== Leaderboard Mission Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordMissionComplete("board_1", "p1", "Alice");
    lbSys.recordMissionComplete("board_1", "p1", "Alice");

    assertTrue(lbSys.getPlayerMissions("board_1", "p1") == 2, "Player completed 2 missions");
}

static void testLeaderboardRanking() {
    std::cout << "\n=== Leaderboard Ranking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordKill("board_1", "p1", "Alice");
    for (int i = 0; i < 5; ++i) lbSys.recordKill("board_1", "p2", "Bob");
    for (int i = 0; i < 3; ++i) lbSys.recordKill("board_1", "p3", "Charlie");

    auto ranking = lbSys.getRankingByKills("board_1");
    assertTrue(static_cast<int>(ranking.size()) == 3, "Ranking has 3 entries");
    assertTrue(ranking[0] == "p2", "Bob is rank 1 (5 kills)");
    assertTrue(ranking[1] == "p3", "Charlie is rank 2 (3 kills)");
    assertTrue(ranking[2] == "p1", "Alice is rank 3 (1 kill)");
}

static void testLeaderboardAchievementDefine() {
    std::cout << "\n=== Leaderboard Achievement Define ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get your first kill", "combat", "total_kills", 1);
    lbSys.defineAchievement("board_1", "veteran", "Veteran", "Reach 100 kills", "combat", "total_kills", 100);

    auto* lb = board->getComponent<components::Leaderboard>();
    assertTrue(static_cast<int>(lb->achievements.size()) == 2, "Two achievements defined");
}

static void testLeaderboardAchievementUnlock() {
    std::cout << "\n=== Leaderboard Achievement Unlock ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get your first kill", "combat", "total_kills", 1);
    lbSys.defineAchievement("board_1", "veteran", "Veteran", "Reach 100 kills", "combat", "total_kills", 100);

    lbSys.recordKill("board_1", "p1", "Alice");
    int unlocked = lbSys.checkAchievements("board_1", "p1", 1000.0f);

    assertTrue(unlocked == 1, "One achievement unlocked");
    assertTrue(lbSys.hasAchievement("board_1", "p1", "first_blood"), "First Blood unlocked");
    assertTrue(!lbSys.hasAchievement("board_1", "p1", "veteran"), "Veteran not unlocked yet");
}

static void testLeaderboardAchievementNoDuplicate() {
    std::cout << "\n=== Leaderboard Achievement No Duplicate ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.defineAchievement("board_1", "first_blood", "First Blood", "Get first kill", "combat", "total_kills", 1);
    lbSys.recordKill("board_1", "p1", "Alice");

    lbSys.checkAchievements("board_1", "p1");
    int second = lbSys.checkAchievements("board_1", "p1");

    assertTrue(second == 0, "No duplicate unlock");
    assertTrue(lbSys.getPlayerAchievementCount("board_1", "p1") == 1, "Still 1 achievement total");
}

static void testLeaderboardNonexistentPlayer() {
    std::cout << "\n=== Leaderboard Nonexistent Player ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    assertTrue(lbSys.getPlayerKills("board_1", "fake") == 0, "Zero kills for nonexistent");
    assertTrue(approxEqual(static_cast<float>(lbSys.getPlayerIscEarned("board_1", "fake")), 0.0f), "Zero Credits for nonexistent");
    assertTrue(lbSys.getPlayerMissions("board_1", "fake") == 0, "Zero missions for nonexistent");
}

static void testLeaderboardDamageTracking() {
    std::cout << "\n=== Leaderboard Damage Tracking ===" << std::endl;
    ecs::World world;
    systems::LeaderboardSystem lbSys(&world);

    auto* board = world.createEntity("board_1");
    addComp<components::Leaderboard>(board);

    lbSys.recordDamageDealt("board_1", "p1", "Alice", 5000.0);
    lbSys.recordDamageDealt("board_1", "p1", "Alice", 3000.0);

    auto* lb = board->getComponent<components::Leaderboard>();
    bool found = false;
    for (const auto& e : lb->entries) {
        if (e.player_id == "p1") {
            found = true;
            assertTrue(approxEqual(static_cast<float>(e.total_damage_dealt), 8000.0f), "Total damage is 8000");
        }
    }
    assertTrue(found, "Player entry found for damage tracking");
}


void run_leaderboard_system_tests() {
    testLeaderboardRecordKill();
    testLeaderboardMultiplePlayers();
    testLeaderboardIscTracking();
    testLeaderboardMissionTracking();
    testLeaderboardRanking();
    testLeaderboardAchievementDefine();
    testLeaderboardAchievementUnlock();
    testLeaderboardAchievementNoDuplicate();
    testLeaderboardNonexistentPlayer();
    testLeaderboardDamageTracking();
}
