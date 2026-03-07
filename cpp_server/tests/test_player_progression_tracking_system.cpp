// Tests for: Player Progression Tracking System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/player_progression_tracking_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Player Progression Tracking System Tests ====================

static void testPlayerProgressionCreate() {
    std::cout << "\n=== PlayerProgressionTracking: Create ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(sys.getMilestoneCount("p1") == 0, "No milestones initially");
    assertTrue(approxEqual(sys.getIscEarned("p1"), 0.0f), "ISC earned is 0");
    assertTrue(approxEqual(sys.getIscSpent("p1"), 0.0f), "ISC spent is 0");
    assertTrue(sys.getKills("p1") == 0, "0 kills initially");
    assertTrue(sys.getDeaths("p1") == 0, "0 deaths initially");
    assertTrue(sys.getDocks("p1") == 0, "0 docks initially");
    assertTrue(sys.getJumps("p1") == 0, "0 jumps initially");
    assertTrue(approxEqual(sys.getMiningYield("p1"), 0.0f), "0 mining yield initially");
    assertTrue(approxEqual(sys.getPlayTime("p1"), 0.0f), "0 play time initially");
}

static void testPlayerProgressionMilestones() {
    std::cout << "\n=== PlayerProgressionTracking: Milestones ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    assertTrue(sys.recordMilestone("p1", "first_dock", "Docked at a station"), "Record first dock");
    assertTrue(sys.recordMilestone("p1", "first_kill", "Destroyed an NPC"), "Record first kill");
    assertTrue(sys.getMilestoneCount("p1") == 2, "2 milestones recorded");
    assertTrue(sys.hasMilestone("p1", "first_dock"), "Has first_dock milestone");
    assertTrue(sys.hasMilestone("p1", "first_kill"), "Has first_kill milestone");
    assertTrue(!sys.hasMilestone("p1", "first_trade"), "Does not have first_trade");
}

static void testPlayerProgressionDuplicateMilestone() {
    std::cout << "\n=== PlayerProgressionTracking: DuplicateMilestone ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.recordMilestone("p1", "first_dock", "Docked at a station");
    assertTrue(!sys.recordMilestone("p1", "first_dock", "Docked again"), "Duplicate milestone rejected");
    assertTrue(sys.getMilestoneCount("p1") == 1, "Still 1 milestone");
}

static void testPlayerProgressionIsc() {
    std::cout << "\n=== PlayerProgressionTracking: ISC ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    assertTrue(sys.addIscEarned("p1", 5000.0f), "Add ISC earned");
    assertTrue(sys.addIscEarned("p1", 3000.0f), "Add more ISC earned");
    assertTrue(approxEqual(sys.getIscEarned("p1"), 8000.0f), "Total ISC earned is 8000");
    assertTrue(sys.addIscSpent("p1", 2000.0f), "Add ISC spent");
    assertTrue(approxEqual(sys.getIscSpent("p1"), 2000.0f), "ISC spent is 2000");
    assertTrue(!sys.addIscEarned("p1", -100.0f), "Negative ISC earned rejected");
    assertTrue(!sys.addIscSpent("p1", 0.0f), "Zero ISC spent rejected");
}

static void testPlayerProgressionCombatStats() {
    std::cout << "\n=== PlayerProgressionTracking: CombatStats ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    assertTrue(sys.recordKill("p1"), "Record kill");
    assertTrue(sys.recordKill("p1"), "Record second kill");
    assertTrue(sys.recordDeath("p1"), "Record death");
    assertTrue(sys.getKills("p1") == 2, "2 kills");
    assertTrue(sys.getDeaths("p1") == 1, "1 death");
}

static void testPlayerProgressionNavigation() {
    std::cout << "\n=== PlayerProgressionTracking: Navigation ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    assertTrue(sys.recordDock("p1"), "Record dock");
    assertTrue(sys.recordDock("p1"), "Record second dock");
    assertTrue(sys.recordJump("p1"), "Record jump");
    assertTrue(sys.getDocks("p1") == 2, "2 docks");
    assertTrue(sys.getJumps("p1") == 1, "1 jump");
}

static void testPlayerProgressionMining() {
    std::cout << "\n=== PlayerProgressionTracking: Mining ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    assertTrue(sys.addMiningYield("p1", 500.0f), "Add mining yield");
    assertTrue(sys.addMiningYield("p1", 300.0f), "Add more mining yield");
    assertTrue(approxEqual(sys.getMiningYield("p1"), 800.0f), "Total mining yield is 800");
    assertTrue(!sys.addMiningYield("p1", -50.0f), "Negative mining yield rejected");
}

static void testPlayerProgressionPlayTime() {
    std::cout << "\n=== PlayerProgressionTracking: PlayTime ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getPlayTime("p1"), 10.0f), "Play time is 10s");
    sys.update(5.0f);
    assertTrue(approxEqual(sys.getPlayTime("p1"), 15.0f), "Play time is 15s");
}

static void testPlayerProgressionMaxMilestones() {
    std::cout << "\n=== PlayerProgressionTracking: MaxMilestones ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    auto* entity = world.getEntity("p1");
    auto* ppt = entity->getComponent<components::PlayerProgressionTracking>();
    ppt->max_milestones = 2;

    sys.recordMilestone("p1", "m1", "First");
    sys.recordMilestone("p1", "m2", "Second");
    assertTrue(!sys.recordMilestone("p1", "m3", "Third"), "Max milestones enforced");
    assertTrue(sys.getMilestoneCount("p1") == 2, "Still 2 milestones");
}

static void testPlayerProgressionMissing() {
    std::cout << "\n=== PlayerProgressionTracking: Missing ===" << std::endl;
    ecs::World world;
    systems::PlayerProgressionTrackingSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.recordMilestone("nonexistent", "m1", "test"), "Milestone fails on missing");
    assertTrue(!sys.addIscEarned("nonexistent", 100.0f), "ISC earned fails on missing");
    assertTrue(!sys.addIscSpent("nonexistent", 100.0f), "ISC spent fails on missing");
    assertTrue(!sys.recordKill("nonexistent"), "Kill fails on missing");
    assertTrue(!sys.recordDeath("nonexistent"), "Death fails on missing");
    assertTrue(!sys.recordDock("nonexistent"), "Dock fails on missing");
    assertTrue(!sys.recordJump("nonexistent"), "Jump fails on missing");
    assertTrue(!sys.addMiningYield("nonexistent", 100.0f), "Mining yield fails on missing");
    assertTrue(sys.getMilestoneCount("nonexistent") == 0, "0 milestones on missing");
    assertTrue(!sys.hasMilestone("nonexistent", "m1"), "No milestone on missing");
    assertTrue(approxEqual(sys.getIscEarned("nonexistent"), 0.0f), "0 ISC earned on missing");
    assertTrue(approxEqual(sys.getIscSpent("nonexistent"), 0.0f), "0 ISC spent on missing");
    assertTrue(sys.getKills("nonexistent") == 0, "0 kills on missing");
    assertTrue(sys.getDeaths("nonexistent") == 0, "0 deaths on missing");
    assertTrue(sys.getDocks("nonexistent") == 0, "0 docks on missing");
    assertTrue(sys.getJumps("nonexistent") == 0, "0 jumps on missing");
    assertTrue(approxEqual(sys.getMiningYield("nonexistent"), 0.0f), "0 mining yield on missing");
    assertTrue(approxEqual(sys.getPlayTime("nonexistent"), 0.0f), "0 play time on missing");
}


void run_player_progression_tracking_system_tests() {
    testPlayerProgressionCreate();
    testPlayerProgressionMilestones();
    testPlayerProgressionDuplicateMilestone();
    testPlayerProgressionIsc();
    testPlayerProgressionCombatStats();
    testPlayerProgressionNavigation();
    testPlayerProgressionMining();
    testPlayerProgressionPlayTime();
    testPlayerProgressionMaxMilestones();
    testPlayerProgressionMissing();
}
