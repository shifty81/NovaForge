// Tests for: DailyQuestSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/daily_quest_system.h"

using namespace atlas;

// ==================== DailyQuestSystem Tests ====================

static void testDailyQuestInit() {
    std::cout << "\n=== DailyQuest: Init ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(sys.getObjectiveCount("p1") == 0, "Zero objectives initially");
    assertTrue(sys.getCompletedObjectiveCount("p1") == 0, "Zero completed initially");
    assertTrue(!sys.isAllComplete("p1"), "Not all complete initially");
    assertTrue(!sys.isBonusClaimed("p1"), "Bonus not claimed initially");
    assertTrue(sys.getDaysCompleted("p1") == 0, "Zero days completed initially");
    assertTrue(sys.getTotalResets("p1") == 0, "Zero resets initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testDailyQuestAddObjective() {
    std::cout << "\n=== DailyQuest: AddObjective ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.addObjective("p1", "kill_rats", "Kill 5 rats", 5), "Add first objective");
    assertTrue(sys.addObjective("p1", "mine_ore", "Mine 100 ore", 100), "Add second objective");
    assertTrue(sys.getObjectiveCount("p1") == 2, "Two objectives stored");
    assertTrue(!sys.addObjective("p1", "kill_rats", "Duplicate", 1), "Duplicate id rejected");
    assertTrue(!sys.addObjective("p1", "", "Empty ID", 1), "Empty id rejected");
    assertTrue(!sys.addObjective("p1", "zero_count", "Zero count", 0), "Zero required_count rejected");
    assertTrue(sys.getObjectiveCount("p1") == 2, "Count unchanged after rejections");
}

static void testDailyQuestMaxObjectives() {
    std::cout << "\n=== DailyQuest: MaxObjectives ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    // Default max is 5
    for (int i = 0; i < 5; i++) {
        std::string id = "obj" + std::to_string(i);
        assertTrue(sys.addObjective("p1", id, "Desc", 1),
                   "Objective added within limit");
    }
    assertTrue(!sys.addObjective("p1", "obj5", "Over limit", 1), "Blocked at max");
    assertTrue(sys.getObjectiveCount("p1") == 5, "Count is 5");
}

static void testDailyQuestProgress() {
    std::cout << "\n=== DailyQuest: Progress ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addObjective("p1", "kills", "Kill 5", 5);
    sys.addObjective("p1", "mine", "Mine 3", 3);

    assertTrue(sys.progressObjective("p1", "kills", 3), "Progress 3/5");
    assertTrue(sys.getObjectiveProgress("p1", "kills") == 3, "Progress is 3");
    assertTrue(!sys.isObjectiveComplete("p1", "kills"), "Not yet complete");
    assertTrue(!sys.isAllComplete("p1"), "Not all complete yet");

    assertTrue(sys.progressObjective("p1", "kills", 5), "Progress capped at required");
    assertTrue(sys.getObjectiveProgress("p1", "kills") == 5, "Progress capped at 5");
    assertTrue(sys.isObjectiveComplete("p1", "kills"), "Kills complete");
    assertTrue(!sys.isAllComplete("p1"), "Mine not done yet");

    assertTrue(sys.progressObjective("p1", "mine", 3), "Progress mine");
    assertTrue(sys.isObjectiveComplete("p1", "mine"), "Mine complete");
    assertTrue(sys.isAllComplete("p1"), "All complete");
    assertTrue(sys.getCompletedObjectiveCount("p1") == 2, "Both objectives complete");
}

static void testDailyQuestProgressInvalidCases() {
    std::cout << "\n=== DailyQuest: ProgressInvalid ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addObjective("p1", "kills", "Kill 5", 5);
    sys.progressObjective("p1", "kills", 5);

    assertTrue(!sys.progressObjective("p1", "kills", 1), "Cannot progress completed objective");
    assertTrue(!sys.progressObjective("p1", "unknown", 1), "Unknown objective fails");
    assertTrue(!sys.progressObjective("p1", "kills", 0), "Zero amount rejected");
    assertTrue(!sys.progressObjective("p1", "kills", -1), "Negative amount rejected");
}

static void testDailyQuestClaimBonus() {
    std::cout << "\n=== DailyQuest: ClaimBonus ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addObjective("p1", "kills", "Kill 1", 1);
    sys.setBonusReward("p1", 500000.0f);

    assertTrue(!sys.claimBonus("p1"), "Cannot claim before completion");
    sys.progressObjective("p1", "kills", 1);
    assertTrue(sys.isAllComplete("p1"), "All complete");
    assertTrue(sys.claimBonus("p1"), "Claim bonus succeeds");
    assertTrue(sys.isBonusClaimed("p1"), "Bonus is claimed");
    assertTrue(!sys.claimBonus("p1"), "Cannot claim twice");
    assertTrue(approxEqual(sys.getBonusReward("p1"), 500000.0f), "Bonus reward stored");
}

static void testDailyQuestForceReset() {
    std::cout << "\n=== DailyQuest: ForceReset ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addObjective("p1", "kills", "Kill 5", 5);
    sys.progressObjective("p1", "kills", 5);
    sys.claimBonus("p1");

    assertTrue(sys.isAllComplete("p1"), "All complete before reset");
    assertTrue(sys.forceReset("p1"), "Force reset succeeds");
    assertTrue(!sys.isAllComplete("p1"), "Not complete after reset");
    assertTrue(!sys.isBonusClaimed("p1"), "Bonus not claimed after reset");
    assertTrue(sys.getObjectiveCount("p1") == 0, "Objectives cleared after reset");
    assertTrue(sys.getTotalResets("p1") == 1, "Total resets incremented");
    assertTrue(sys.getDaysCompleted("p1") == 1, "Days completed incremented (was complete)");
}

static void testDailyQuestForceResetIncomplete() {
    std::cout << "\n=== DailyQuest: ForceResetIncomplete ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addObjective("p1", "kills", "Kill 5", 5);
    sys.progressObjective("p1", "kills", 2);

    sys.forceReset("p1");
    assertTrue(sys.getTotalResets("p1") == 1, "Total resets incremented");
    assertTrue(sys.getDaysCompleted("p1") == 0, "Days NOT incremented (was incomplete)");
}

static void testDailyQuestTimerReset() {
    std::cout << "\n=== DailyQuest: TimerReset ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.setResetDuration("p1", 5.0f);
    sys.addObjective("p1", "kills", "Kill 1", 1);
    // Progress starts the timer AND completes the objective
    sys.progressObjective("p1", "kills", 1);

    ecs::Entity* entity = world.getEntity("p1");
    (void)entity;
    // Timer counts down; simulate 6 seconds elapsing past reset
    sys.update(6.0f);
    assertTrue(sys.getTotalResets("p1") == 1, "Timer triggered reset");
    assertTrue(sys.getDaysCompleted("p1") == 1, "Day counted as complete");
    assertTrue(sys.getObjectiveCount("p1") == 0, "Objectives cleared by timer");
}

static void testDailyQuestAddObjectiveBlockedWhileRunning() {
    std::cout << "\n=== DailyQuest: AddBlockedWhileRunning ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.setResetDuration("p1", 100.0f);

    // Adding objectives is fine before quest starts
    assertTrue(sys.addObjective("p1", "obj1", "Desc1", 5), "First objective added");
    assertTrue(sys.addObjective("p1", "obj2", "Desc2", 3), "Second objective added");
    // Timer starts on first progress call
    sys.progressObjective("p1", "obj1", 1);
    // Now timer is running — new objectives are blocked
    assertTrue(!sys.addObjective("p1", "obj3", "Desc3", 1), "Blocked while timer running");

    // After force reset, timer is cleared → can add again
    sys.forceReset("p1");
    assertTrue(sys.addObjective("p1", "obj_new", "New Desc", 1), "Can add after force reset");
}

static void testDailyQuestSetResetDuration() {
    std::cout << "\n=== DailyQuest: SetResetDuration ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.setResetDuration("p1", 3600.0f), "Set 1-hour reset");
    assertTrue(!sys.setResetDuration("p1", 0.0f), "Zero duration rejected");
    assertTrue(!sys.setResetDuration("p1", -1.0f), "Negative duration rejected");
    assertTrue(!sys.setResetDuration("nonexistent", 3600.0f), "Fails on missing entity");
}

static void testDailyQuestMissing() {
    std::cout << "\n=== DailyQuest: Missing ===" << std::endl;
    ecs::World world;
    systems::DailyQuestSystem sys(&world);

    assertTrue(!sys.addObjective("none", "obj", "Desc", 1), "Add fails on missing");
    assertTrue(!sys.removeObjective("none", "obj"), "Remove fails on missing");
    assertTrue(!sys.progressObjective("none", "obj", 1), "Progress fails on missing");
    assertTrue(!sys.claimBonus("none"), "ClaimBonus fails on missing");
    assertTrue(!sys.setBonusReward("none", 0.0f), "SetBonus fails on missing");
    assertTrue(!sys.forceReset("none"), "ForceReset fails on missing");
    assertTrue(sys.getObjectiveCount("none") == 0, "0 objectives on missing");
    assertTrue(!sys.isAllComplete("none"), "Not complete on missing");
    assertTrue(!sys.isBonusClaimed("none"), "Not claimed on missing");
    assertTrue(sys.getDaysCompleted("none") == 0, "0 days on missing");
    assertTrue(sys.getTotalResets("none") == 0, "0 resets on missing");
}

void run_daily_quest_system_tests() {
    testDailyQuestInit();
    testDailyQuestAddObjective();
    testDailyQuestMaxObjectives();
    testDailyQuestProgress();
    testDailyQuestProgressInvalidCases();
    testDailyQuestClaimBonus();
    testDailyQuestForceReset();
    testDailyQuestForceResetIncomplete();
    testDailyQuestTimerReset();
    testDailyQuestAddObjectiveBlockedWhileRunning();
    testDailyQuestSetResetDuration();
    testDailyQuestMissing();
}
