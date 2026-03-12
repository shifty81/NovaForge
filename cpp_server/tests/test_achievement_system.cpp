// Tests for: AchievementSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/achievement_system.h"

using namespace atlas;

// ==================== AchievementSystem Tests ====================

static void testAchievementInit() {
    std::cout << "\n=== Achievement: Init ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(sys.getAchievementCount("p1") == 0, "Zero achievements initially");
    assertTrue(sys.getUnlockedCount("p1") == 0, "Zero unlocked initially");
    assertTrue(sys.getTotalRewardPoints("p1") == 0, "Zero reward points initially");
    assertTrue(sys.getTotalProgressCalls("p1") == 0, "Zero progress calls initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testAchievementAdd() {
    std::cout << "\n=== Achievement: Add ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    assertTrue(sys.addAchievement("p1", "first_kill", "First Kill", Cat::Combat, 1, 10),
               "Add combat achievement");
    assertTrue(sys.addAchievement("p1", "mine_100", "Mine 100 Ore", Cat::Economy, 100, 50),
               "Add economy achievement");
    assertTrue(sys.getAchievementCount("p1") == 2, "Two achievements stored");
    assertTrue(sys.hasAchievement("p1", "first_kill"), "Has first_kill");
    assertTrue(sys.hasAchievement("p1", "mine_100"), "Has mine_100");
    assertTrue(!sys.hasAchievement("p1", "unknown"), "No unknown achievement");
}

static void testAchievementAddValidation() {
    std::cout << "\n=== Achievement: AddValidation ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    assertTrue(!sys.addAchievement("p1", "", "Empty ID", Cat::Combat, 1, 0),
               "Empty id rejected");
    assertTrue(!sys.addAchievement("p1", "id1", "", Cat::Combat, 1, 0),
               "Empty name rejected");
    assertTrue(!sys.addAchievement("p1", "id1", "Name", Cat::Combat, 0, 0),
               "Zero required_count rejected");
    assertTrue(!sys.addAchievement("p1", "id1", "Name", Cat::Combat, -1, 0),
               "Negative required_count rejected");
    assertTrue(!sys.addAchievement("p1", "id1", "Name", Cat::Combat, 1, -5),
               "Negative reward_points rejected");
    assertTrue(sys.addAchievement("p1", "valid", "Valid", Cat::Combat, 1, 0),
               "Valid achievement added");
    assertTrue(!sys.addAchievement("p1", "valid", "Duplicate", Cat::Combat, 1, 0),
               "Duplicate id rejected");
    assertTrue(sys.getAchievementCount("p1") == 1, "Only 1 valid achievement");
}

static void testAchievementMaxCap() {
    std::cout << "\n=== Achievement: MaxCap ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    // Default max is 50, fill up to 50
    for (int i = 0; i < 50; i++) {
        std::string id = "ach" + std::to_string(i);
        assertTrue(sys.addAchievement("p1", id, "Ach", Cat::Progression, 1, 1),
                   "Achievement added within limit");
    }
    assertTrue(!sys.addAchievement("p1", "ach50", "Over limit", Cat::Progression, 1, 1),
               "Blocked at max");
    assertTrue(sys.getAchievementCount("p1") == 50, "Count is 50");
}

static void testAchievementProgress() {
    std::cout << "\n=== Achievement: Progress ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    sys.addAchievement("p1", "kills", "Kill 10", Cat::Combat, 10, 100);
    sys.addAchievement("p1", "explore", "Scan 5", Cat::Exploration, 5, 50);

    assertTrue(sys.progressAchievement("p1", "kills", 3), "Progress 3/10");
    assertTrue(sys.getProgress("p1", "kills") == 3, "Progress is 3");
    assertTrue(!sys.isUnlocked("p1", "kills"), "Not yet unlocked");
    assertTrue(sys.getTotalProgressCalls("p1") == 1, "1 progress call");

    assertTrue(sys.progressAchievement("p1", "kills", 10), "Progress capped");
    assertTrue(sys.getProgress("p1", "kills") == 10, "Progress capped at 10");
    assertTrue(sys.isUnlocked("p1", "kills"), "Kills unlocked");
    assertTrue(sys.getUnlockedCount("p1") == 1, "1 unlocked");
    assertTrue(sys.getTotalRewardPoints("p1") == 100, "100 reward points");

    assertTrue(sys.progressAchievement("p1", "explore", 5), "Unlock explore");
    assertTrue(sys.isUnlocked("p1", "explore"), "Explore unlocked");
    assertTrue(sys.getUnlockedCount("p1") == 2, "2 unlocked");
    assertTrue(sys.getTotalRewardPoints("p1") == 150, "150 total reward points");
}

static void testAchievementProgressInvalid() {
    std::cout << "\n=== Achievement: ProgressInvalid ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    sys.addAchievement("p1", "kills", "Kill 1", Cat::Combat, 1, 10);
    sys.progressAchievement("p1", "kills", 1);

    assertTrue(!sys.progressAchievement("p1", "kills", 1), "Cannot progress unlocked");
    assertTrue(!sys.progressAchievement("p1", "unknown", 1), "Unknown ach fails");
    assertTrue(!sys.progressAchievement("p1", "kills", 0), "Zero amount rejected");
    assertTrue(!sys.progressAchievement("p1", "kills", -1), "Negative amount rejected");
}

static void testAchievementRemove() {
    std::cout << "\n=== Achievement: Remove ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    sys.addAchievement("p1", "ach1", "Ach1", Cat::Combat, 1, 10);
    sys.addAchievement("p1", "ach2", "Ach2", Cat::Economy, 1, 20);

    assertTrue(sys.removeAchievement("p1", "ach1"), "Remove succeeds");
    assertTrue(sys.getAchievementCount("p1") == 1, "Count is 1");
    assertTrue(!sys.hasAchievement("p1", "ach1"), "ach1 removed");
    assertTrue(sys.hasAchievement("p1", "ach2"), "ach2 still present");
    assertTrue(!sys.removeAchievement("p1", "ach1"), "Remove nonexistent fails");
    assertTrue(!sys.removeAchievement("p1", "unknown"), "Remove unknown fails");
}

static void testAchievementClear() {
    std::cout << "\n=== Achievement: Clear ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    sys.addAchievement("p1", "ach1", "Ach1", Cat::Combat, 1, 10);
    sys.addAchievement("p1", "ach2", "Ach2", Cat::Economy, 1, 20);

    assertTrue(sys.clearAchievements("p1"), "Clear succeeds");
    assertTrue(sys.getAchievementCount("p1") == 0, "Count is 0 after clear");
    assertTrue(!sys.hasAchievement("p1", "ach1"), "ach1 gone");
    assertTrue(!sys.hasAchievement("p1", "ach2"), "ach2 gone");
}

static void testAchievementSetRewardPoints() {
    std::cout << "\n=== Achievement: SetRewardPoints ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    sys.addAchievement("p1", "ach1", "Ach1", Cat::Combat, 5, 10);

    assertTrue(sys.setRewardPoints("p1", "ach1", 200), "Set reward points");
    sys.progressAchievement("p1", "ach1", 5);
    assertTrue(sys.getTotalRewardPoints("p1") == 200, "200 points awarded");
    assertTrue(!sys.setRewardPoints("p1", "ach1", 300), "Cannot set after unlocked");
    assertTrue(!sys.setRewardPoints("p1", "ach1", -1), "Negative points rejected");
    assertTrue(!sys.setRewardPoints("p1", "unknown", 10), "Unknown ach fails");
}

static void testAchievementCategories() {
    std::cout << "\n=== Achievement: Categories ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    sys.addAchievement("p1", "c1", "C1", Cat::Combat, 1, 0);
    sys.addAchievement("p1", "c2", "C2", Cat::Combat, 1, 0);
    sys.addAchievement("p1", "e1", "E1", Cat::Economy, 1, 0);
    sys.addAchievement("p1", "x1", "X1", Cat::Exploration, 1, 0);
    sys.addAchievement("p1", "s1", "S1", Cat::Social, 1, 0);
    sys.addAchievement("p1", "p1a", "P1", Cat::Progression, 1, 0);

    assertTrue(sys.getCountByCategory("p1", Cat::Combat) == 2, "2 combat");
    assertTrue(sys.getCountByCategory("p1", Cat::Economy) == 1, "1 economy");
    assertTrue(sys.getCountByCategory("p1", Cat::Exploration) == 1, "1 exploration");
    assertTrue(sys.getCountByCategory("p1", Cat::Social) == 1, "1 social");
    assertTrue(sys.getCountByCategory("p1", Cat::Progression) == 1, "1 progression");
    assertTrue(sys.getAchievementCount("p1") == 6, "6 total");
}

static void testAchievementGetRequired() {
    std::cout << "\n=== Achievement: GetRequiredCount ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    using Cat = components::AchievementState::Category;
    sys.addAchievement("p1", "kills", "Kill 25", Cat::Combat, 25, 100);
    assertTrue(sys.getRequiredCount("p1", "kills") == 25, "Required is 25");
    assertTrue(sys.getRequiredCount("p1", "unknown") == 0, "0 for unknown");
}

static void testAchievementMissing() {
    std::cout << "\n=== Achievement: Missing ===" << std::endl;
    ecs::World world;
    systems::AchievementSystem sys(&world);

    using Cat = components::AchievementState::Category;
    assertTrue(!sys.addAchievement("none", "id", "Name", Cat::Combat, 1, 0),
               "Add fails on missing");
    assertTrue(!sys.removeAchievement("none", "id"), "Remove fails on missing");
    assertTrue(!sys.clearAchievements("none"), "Clear fails on missing");
    assertTrue(!sys.progressAchievement("none", "id", 1), "Progress fails on missing");
    assertTrue(!sys.setRewardPoints("none", "id", 10), "SetReward fails on missing");
    assertTrue(sys.getAchievementCount("none") == 0, "0 count on missing");
    assertTrue(sys.getUnlockedCount("none") == 0, "0 unlocked on missing");
    assertTrue(!sys.isUnlocked("none", "id"), "Not unlocked on missing");
    assertTrue(sys.getProgress("none", "id") == 0, "0 progress on missing");
    assertTrue(sys.getRequiredCount("none", "id") == 0, "0 required on missing");
    assertTrue(sys.getTotalRewardPoints("none") == 0, "0 reward on missing");
    assertTrue(sys.getTotalProgressCalls("none") == 0, "0 calls on missing");
    assertTrue(sys.getCountByCategory("none", Cat::Combat) == 0, "0 category on missing");
    assertTrue(!sys.hasAchievement("none", "id"), "No ach on missing");
}

void run_achievement_system_tests() {
    testAchievementInit();
    testAchievementAdd();
    testAchievementAddValidation();
    testAchievementMaxCap();
    testAchievementProgress();
    testAchievementProgressInvalid();
    testAchievementRemove();
    testAchievementClear();
    testAchievementSetRewardPoints();
    testAchievementCategories();
    testAchievementGetRequired();
    testAchievementMissing();
}
