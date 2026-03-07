// Tests for: Skill Training System
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/skill_training_system.h"

using namespace atlas;

// ==================== Skill Training System Tests ====================

static void testSkillTrainingCreate() {
    std::cout << "\n=== SkillTraining: Create ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    world.createEntity("player1");
    assertTrue(sys.initialize("player1", 10.0f), "Init succeeds");
    assertTrue(sys.getQueueLength("player1") == 0, "0 in queue");
    assertTrue(sys.getCurrentSkillId("player1") == "", "No current skill");
    assertTrue(sys.getCurrentSkillLevel("player1") == 0, "Level 0");
    assertTrue(sys.getCurrentSkillProgress("player1") == 0.0f, "0 progress");
    assertTrue(sys.getTotalSkillsCompleted("player1") == 0, "0 completed");
    assertTrue(sys.getTotalSpEarned("player1") == 0.0f, "0 SP earned");
    assertTrue(!sys.isTraining("player1"), "Not training (empty queue)");

    // Invalid SP rate
    world.createEntity("player2");
    assertTrue(!sys.initialize("player2", 0.0f), "SP rate 0 rejected");
    assertTrue(!sys.initialize("player2", -1.0f), "Negative SP rate rejected");
}

static void testSkillTrainingEnqueue() {
    std::cout << "\n=== SkillTraining: Enqueue ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", 10.0f);

    assertTrue(sys.enqueueSkill("player1", "navigation", 3, 1000), "Enqueue navigation");
    assertTrue(sys.enqueueSkill("player1", "gunnery", 2, 2000), "Enqueue gunnery");
    assertTrue(sys.getQueueLength("player1") == 2, "2 in queue");
    assertTrue(sys.getCurrentSkillId("player1") == "navigation", "First is navigation");
    assertTrue(sys.isTraining("player1"), "Is training");

    // Duplicate rejected
    assertTrue(!sys.enqueueSkill("player1", "navigation", 4, 1000), "Dup rejected");

    // Invalid level
    assertTrue(!sys.enqueueSkill("player1", "mining", 0, 1000), "Level 0 rejected");
    assertTrue(!sys.enqueueSkill("player1", "mining", 6, 1000), "Level 6 rejected");

    // Invalid SP cost
    assertTrue(!sys.enqueueSkill("player1", "mining", 3, 0), "SP cost 0 rejected");
}

static void testSkillTrainingRemove() {
    std::cout << "\n=== SkillTraining: Remove ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", 10.0f);
    sys.enqueueSkill("player1", "navigation", 3, 1000);
    sys.enqueueSkill("player1", "gunnery", 2, 2000);

    assertTrue(sys.removeSkill("player1", "gunnery"), "Remove gunnery");
    assertTrue(sys.getQueueLength("player1") == 1, "1 in queue");
    assertTrue(!sys.removeSkill("player1", "gunnery"), "Can't remove twice");
    assertTrue(!sys.removeSkill("player1", "nonexistent"), "Can't remove missing");
}

static void testSkillTrainingProgress() {
    std::cout << "\n=== SkillTraining: Progress ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", 100.0f);  // 100 SP/s for fast testing

    // Level 1 of a skill with base_sp_cost=100: requires 100×1²=100 SP → 1 second
    sys.enqueueSkill("player1", "navigation", 1, 100);
    assertTrue(sys.isTraining("player1"), "Training active");

    // Half progress
    sys.update(0.5f);
    float progress = sys.getCurrentSkillProgress("player1");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% progress at 0.5s");
    assertTrue(sys.getTotalSpEarned("player1") > 49.0f, "~50 SP earned");

    // Complete
    sys.update(0.5f);
    assertTrue(sys.getQueueLength("player1") == 0, "Queue empty after completion");
    assertTrue(sys.getTotalSkillsCompleted("player1") == 1, "1 completed");
    assertTrue(!sys.isTraining("player1"), "Not training (queue empty)");
}

static void testSkillTrainingLevelScaling() {
    std::cout << "\n=== SkillTraining: LevelScaling ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", 100.0f);

    // Level 3 of a skill with base_sp_cost=100: requires 100×3²=900 SP → 9 seconds
    sys.enqueueSkill("player1", "gunnery", 3, 100);

    sys.update(4.5f);  // 450 SP = 50% of 900
    float progress = sys.getCurrentSkillProgress("player1");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% at 4.5s for level 3");

    sys.update(4.5f);  // 900 SP total → complete
    assertTrue(sys.getTotalSkillsCompleted("player1") == 1, "Level 3 completed");
}

static void testSkillTrainingQueueChain() {
    std::cout << "\n=== SkillTraining: QueueChain ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", 200.0f);

    // Two level-1 skills, base 100: each needs 100 SP → 0.5s each
    sys.enqueueSkill("player1", "navigation", 1, 100);
    sys.enqueueSkill("player1", "gunnery", 1, 100);
    assertTrue(sys.getQueueLength("player1") == 2, "2 in queue");

    sys.update(0.5f);  // completes navigation
    assertTrue(sys.getTotalSkillsCompleted("player1") == 1, "Navigation completed");
    assertTrue(sys.getCurrentSkillId("player1") == "gunnery", "Now training gunnery");

    sys.update(0.5f);  // completes gunnery
    assertTrue(sys.getTotalSkillsCompleted("player1") == 2, "Both completed");
    assertTrue(sys.getQueueLength("player1") == 0, "Queue empty");
}

static void testSkillTrainingPauseResume() {
    std::cout << "\n=== SkillTraining: PauseResume ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", 100.0f);
    sys.enqueueSkill("player1", "navigation", 1, 100);

    assertTrue(sys.pauseTraining("player1"), "Pause succeeds");
    assertTrue(!sys.isTraining("player1"), "Not training while paused");
    assertTrue(!sys.pauseTraining("player1"), "Can't pause twice");

    // No progress while paused
    sys.update(1.0f);
    assertTrue(sys.getCurrentSkillProgress("player1") == 0.0f, "0 progress while paused");

    assertTrue(sys.resumeTraining("player1"), "Resume succeeds");
    assertTrue(sys.isTraining("player1"), "Training resumed");
    assertTrue(!sys.resumeTraining("player1"), "Can't resume twice");

    // Progress resumes
    sys.update(0.5f);
    float progress = sys.getCurrentSkillProgress("player1");
    assertTrue(progress > 0.49f && progress < 0.51f, "~50% after resume");
}

static void testSkillTrainingMaxQueue() {
    std::cout << "\n=== SkillTraining: MaxQueue ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    world.createEntity("player1");
    sys.initialize("player1", 10.0f);

    // Fill to max (10)
    for (int i = 0; i < 10; i++) {
        assertTrue(sys.enqueueSkill("player1", "skill_" + std::to_string(i), 1, 1000),
                   "Enqueue skill_" + std::to_string(i));
    }
    assertTrue(sys.getQueueLength("player1") == 10, "10 in queue at max");
    assertTrue(!sys.enqueueSkill("player1", "skill_10", 1, 1000), "11th rejected");
}

static void testSkillTrainingMissing() {
    std::cout << "\n=== SkillTraining: Missing ===" << std::endl;
    ecs::World world;
    systems::SkillTrainingSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 10.0f), "Init fails on missing");
    assertTrue(!sys.enqueueSkill("nonexistent", "nav", 1, 100), "Enqueue fails on missing");
    assertTrue(!sys.removeSkill("nonexistent", "nav"), "Remove fails on missing");
    assertTrue(!sys.pauseTraining("nonexistent"), "Pause fails on missing");
    assertTrue(!sys.resumeTraining("nonexistent"), "Resume fails on missing");
    assertTrue(sys.getQueueLength("nonexistent") == 0, "0 queue on missing");
    assertTrue(sys.getCurrentSkillId("nonexistent") == "", "Empty skill on missing");
    assertTrue(sys.getCurrentSkillLevel("nonexistent") == 0, "0 level on missing");
    assertTrue(sys.getCurrentSkillProgress("nonexistent") == 0.0f, "0 progress on missing");
    assertTrue(sys.getTotalSkillsCompleted("nonexistent") == 0, "0 completed on missing");
    assertTrue(sys.getTotalSpEarned("nonexistent") == 0.0f, "0 SP on missing");
    assertTrue(!sys.isTraining("nonexistent"), "Not training on missing");
}

void run_skill_training_system_tests() {
    testSkillTrainingCreate();
    testSkillTrainingEnqueue();
    testSkillTrainingRemove();
    testSkillTrainingProgress();
    testSkillTrainingLevelScaling();
    testSkillTrainingQueueChain();
    testSkillTrainingPauseResume();
    testSkillTrainingMaxQueue();
    testSkillTrainingMissing();
}
