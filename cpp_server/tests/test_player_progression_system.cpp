// Tests for: PlayerProgressionSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "ecs/system.h"
#include "systems/player_progression_system.h"

using namespace atlas;

// ==================== PlayerProgressionSystem Tests ====================

static void testPlayerProgressionInit() {
    std::cout << "\n=== Player Progression: Init ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    assertTrue(sys.initProgression("player1"), "Progression initialized");
    assertTrue(!sys.initProgression("player1"), "Duplicate init rejected");
    assertTrue(!sys.initProgression("nonexistent"), "Missing entity fails");
    assertTrue(sys.getLevel("player1") == 1, "Initial level is 1");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 0.0f), "Initial XP is 0");
}

static void testPlayerProgressionAwardXP() {
    std::cout << "\n=== Player Progression: Award XP ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    assertTrue(sys.awardXP("player1", "combat", 50.0f), "Combat XP awarded");
    assertTrue(sys.awardXP("player1", "mining", 30.0f), "Mining XP awarded");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getCategoryXP("player1", "combat"), 50.0f), "Combat XP is 50");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "mining"), 30.0f), "Mining XP is 30");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 80.0f), "Total XP is 80");
}

static void testPlayerProgressionInvalidCategory() {
    std::cout << "\n=== Player Progression: Invalid Category ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    assertTrue(!sys.awardXP("player1", "invalid_category", 50.0f), "Invalid category rejected");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "invalid_category"), 0.0f), "Unknown category returns 0");
}

static void testPlayerProgressionLevelUp() {
    std::cout << "\n=== Player Progression: Level Up ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    // Level 1 requires 100*1^1.5 = 100 XP
    sys.awardXP("player1", "combat", 100.0f);
    sys.update(0.0f);
    assertTrue(sys.getLevel("player1") == 2, "Level 2 after 100 XP");

    // Level 2 requires 100*2^1.5 ≈ 282.84 XP additional
    sys.awardXP("player1", "combat", 283.0f);
    sys.update(0.0f);
    assertTrue(sys.getLevel("player1") == 3, "Level 3 after ~383 XP total");
}

static void testPlayerProgressionLevelProgress() {
    std::cout << "\n=== Player Progression: Level Progress ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    sys.awardXP("player1", "combat", 50.0f);
    sys.update(0.0f);
    // 50 XP, need 100 for level 1->2, so progress = 0.5
    float progress = sys.getLevelProgress("player1");
    assertTrue(approxEqual(progress, 0.5f), "Level progress is 0.5 at 50/100 XP");
}

static void testPlayerProgressionMilestone() {
    std::cout << "\n=== Player Progression: Milestone ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    sys.addMilestone("player1", "First Blood", "combat", 25.0f);
    sys.addMilestone("player1", "Expert Miner", "mining", 100.0f);

    sys.awardXP("player1", "combat", 30.0f);
    sys.update(0.0f);
    assertTrue(sys.getMilestonesAchieved("player1") == 1, "One milestone achieved (First Blood)");

    sys.awardXP("player1", "mining", 110.0f);
    sys.update(0.0f);
    assertTrue(sys.getMilestonesAchieved("player1") == 2, "Two milestones achieved");
}

static void testPlayerProgressionPrestige() {
    std::cout << "\n=== Player Progression: Prestige ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");

    // Can't prestige below level 50
    assertTrue(!sys.prestige("player1"), "Cannot prestige at level 1");

    // Award enough XP to reach level 50
    // Cumulative XP for levels 1-49: ~724,870
    sys.awardXP("player1", "combat", 730000.0f);
    sys.update(0.0f);
    assertTrue(sys.getLevel("player1") >= 50, "Reached level 50+");

    assertTrue(sys.prestige("player1"), "Prestige succeeds");
    assertTrue(sys.getPrestigeLevel("player1") == 1, "Prestige level is 1");
    assertTrue(approxEqual(sys.getPrestigeMultiplier("player1"), 1.1f), "Prestige multiplier is 1.1");
    assertTrue(sys.getLevel("player1") == 1, "Level reset to 1");
    assertTrue(approxEqual(sys.getTotalXP("player1"), 0.0f), "XP reset to 0");
}

static void testPlayerProgressionPrestigeXPBonus() {
    std::cout << "\n=== Player Progression: Prestige XP Bonus ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");

    // Get to prestige first
    sys.awardXP("player1", "combat", 730000.0f);
    sys.update(0.0f);
    sys.prestige("player1");

    // Now award XP with prestige bonus (1.1x)
    sys.awardXP("player1", "combat", 100.0f);
    sys.update(0.0f);
    // Should be 100 * 1.1 = 110 actual XP
    assertTrue(approxEqual(sys.getCategoryXP("player1", "combat"), 110.0f), "XP scaled by prestige multiplier");
}

static void testPlayerProgressionAllCategories() {
    std::cout << "\n=== Player Progression: All Categories ===" << std::endl;
    ecs::World world;
    world.createEntity("player1");

    systems::PlayerProgressionSystem sys(&world);
    sys.initProgression("player1");
    sys.awardXP("player1", "combat", 10.0f);
    sys.awardXP("player1", "mining", 20.0f);
    sys.awardXP("player1", "exploration", 30.0f);
    sys.awardXP("player1", "industry", 40.0f);
    sys.awardXP("player1", "social", 50.0f);
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getTotalXP("player1"), 150.0f), "Total XP is sum of all categories");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "exploration"), 30.0f), "Exploration XP correct");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "industry"), 40.0f), "Industry XP correct");
    assertTrue(approxEqual(sys.getCategoryXP("player1", "social"), 50.0f), "Social XP correct");
}

static void testPlayerProgressionMissing() {
    std::cout << "\n=== Player Progression: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionSystem sys(&world);
    assertTrue(sys.getLevel("nonexistent") == 0, "Default level for missing");
    assertTrue(approxEqual(sys.getTotalXP("nonexistent"), 0.0f), "Default XP for missing");
    assertTrue(sys.getMilestonesAchieved("nonexistent") == 0, "Default milestones for missing");
    assertTrue(sys.getPrestigeLevel("nonexistent") == 0, "Default prestige for missing");
}

static void testPlayerProgressionXPCurve() {
    std::cout << "\n=== Player Progression: XP Curve ===" << std::endl;
    float level1 = systems::PlayerProgressionSystem::xpForLevel(1);
    float level2 = systems::PlayerProgressionSystem::xpForLevel(2);
    float level10 = systems::PlayerProgressionSystem::xpForLevel(10);
    assertTrue(approxEqual(level1, 100.0f), "Level 1 requires 100 XP");
    assertTrue(level2 > level1, "Level 2 requires more XP than level 1");
    assertTrue(level10 > level2, "Level 10 requires more XP than level 2");
    // Level 2: 100*2^1.5 ≈ 282.84
    assertTrue(approxEqual(level2, 100.0f * std::pow(2.0f, 1.5f)), "Level 2 XP follows curve");
}


void run_player_progression_system_tests() {
    testPlayerProgressionInit();
    testPlayerProgressionAwardXP();
    testPlayerProgressionInvalidCategory();
    testPlayerProgressionLevelUp();
    testPlayerProgressionLevelProgress();
    testPlayerProgressionMilestone();
    testPlayerProgressionPrestige();
    testPlayerProgressionPrestigeXPBonus();
    testPlayerProgressionAllCategories();
    testPlayerProgressionMissing();
    testPlayerProgressionXPCurve();
}
