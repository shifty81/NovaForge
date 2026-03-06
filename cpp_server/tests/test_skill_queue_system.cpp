// Tests for: Skill Queue System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/skill_queue_system.h"

using namespace atlas;

// ==================== Skill Queue System Tests ====================

static void testSkillQueueCreate() {
    std::cout << "\n=== SkillQueue: Create ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_001"), "Init succeeds");
    assertTrue(sys.getQueueLength("p1") == 0, "Empty queue");
    assertTrue(sys.getCurrentSkillId("p1").empty(), "No current skill");
    assertTrue(approxEqual(sys.getCurrentProgress("p1"), 0.0f), "0 progress");
    assertTrue(!sys.isTraining("p1"), "Not training");
    assertTrue(!sys.isPaused("p1"), "Not paused");
    assertTrue(sys.getCompletedCount("p1") == 0, "0 completed");
    assertTrue(approxEqual(sys.getTotalTrainingTime("p1"), 0.0f), "0 total time");
}

static void testSkillQueueEnqueue() {
    std::cout << "\n=== SkillQueue: Enqueue ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.enqueueSkill("p1", "navigation_1", 1, 60.0f), "Enqueue nav");
    assertTrue(sys.enqueueSkill("p1", "gunnery_1", 1, 120.0f), "Enqueue gun");
    assertTrue(sys.getQueueLength("p1") == 2, "2 queued");
    assertTrue(sys.isQueued("p1", "navigation_1"), "nav queued");
    assertTrue(sys.isQueued("p1", "gunnery_1"), "gun queued");
    assertTrue(!sys.isQueued("p1", "missing"), "missing not queued");
    // Duplicate rejected
    assertTrue(!sys.enqueueSkill("p1", "navigation_1", 2, 180.0f), "Dup rejected");
    assertTrue(sys.getQueueLength("p1") == 2, "Still 2");
    assertTrue(sys.isTraining("p1"), "Now training");
    assertTrue(sys.getCurrentSkillId("p1") == "navigation_1", "Current is nav");
}

static void testSkillQueueMax() {
    std::cout << "\n=== SkillQueue: Max ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::SkillQueueState>();
    state->max_queue_size = 2;
    sys.enqueueSkill("p1", "s1", 1, 60.0f);
    sys.enqueueSkill("p1", "s2", 1, 60.0f);
    assertTrue(!sys.enqueueSkill("p1", "s3", 1, 60.0f), "Max queue enforced");
    assertTrue(sys.getQueueLength("p1") == 2, "Still 2");
}

static void testSkillQueueDequeue() {
    std::cout << "\n=== SkillQueue: Dequeue ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.enqueueSkill("p1", "s1", 1, 60.0f);
    sys.enqueueSkill("p1", "s2", 1, 120.0f);
    assertTrue(sys.dequeueSkill("p1", "s1"), "Dequeue s1");
    assertTrue(sys.getQueueLength("p1") == 1, "1 left");
    assertTrue(!sys.isQueued("p1", "s1"), "s1 gone");
    assertTrue(!sys.dequeueSkill("p1", "s1"), "Double dequeue fails");
    assertTrue(!sys.dequeueSkill("p1", "nonexistent"), "Nonexistent fails");
}

static void testSkillQueueTrainingProgress() {
    std::cout << "\n=== SkillQueue: TrainingProgress ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.enqueueSkill("p1", "nav", 1, 100.0f);
    sys.update(30.0f);
    assertTrue(approxEqual(sys.getCurrentProgress("p1"), 0.3f, 0.01f), "30% progress");
    assertTrue(approxEqual(sys.getRemainingTime("p1"), 70.0f, 0.1f), "70s remaining");
    sys.update(50.0f);
    assertTrue(approxEqual(sys.getCurrentProgress("p1"), 0.8f, 0.01f), "80% progress");
    assertTrue(approxEqual(sys.getRemainingTime("p1"), 20.0f, 0.1f), "20s remaining");
}

static void testSkillQueueCompletion() {
    std::cout << "\n=== SkillQueue: Completion ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.enqueueSkill("p1", "nav", 3, 50.0f);
    sys.enqueueSkill("p1", "gun", 1, 80.0f);
    // Complete first skill
    sys.update(55.0f);
    assertTrue(sys.getCompletedCount("p1") == 1, "1 completed");
    assertTrue(sys.hasCompleted("p1", "nav"), "nav completed");
    assertTrue(sys.getSkillLevel("p1", "nav") == 3, "nav level 3");
    assertTrue(sys.getQueueLength("p1") == 1, "1 remaining in queue");
    assertTrue(sys.getCurrentSkillId("p1") == "gun", "Now training gun");
    assertTrue(sys.isTraining("p1"), "Still training");
    // Complete second skill
    sys.update(85.0f);
    assertTrue(sys.getCompletedCount("p1") == 2, "2 completed");
    assertTrue(sys.hasCompleted("p1", "gun"), "gun completed");
    assertTrue(sys.getQueueLength("p1") == 0, "Queue empty");
    assertTrue(!sys.isTraining("p1"), "No longer training");
    assertTrue(sys.getCurrentSkillId("p1").empty(), "No current skill");
}

static void testSkillQueuePauseResume() {
    std::cout << "\n=== SkillQueue: PauseResume ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.enqueueSkill("p1", "nav", 1, 100.0f);
    sys.update(20.0f);
    assertTrue(approxEqual(sys.getCurrentProgress("p1"), 0.2f, 0.01f), "20% before pause");
    assertTrue(sys.pauseTraining("p1"), "Pause");
    assertTrue(sys.isPaused("p1"), "Is paused");
    assertTrue(!sys.isTraining("p1"), "Not training while paused");
    assertTrue(!sys.pauseTraining("p1"), "Double pause fails");
    sys.update(50.0f); // Time passes but no progress
    assertTrue(approxEqual(sys.getCurrentProgress("p1"), 0.2f, 0.01f), "Still 20% while paused");
    assertTrue(sys.resumeTraining("p1"), "Resume");
    assertTrue(!sys.isPaused("p1"), "Not paused");
    assertTrue(!sys.resumeTraining("p1"), "Double resume fails");
    sys.update(30.0f);
    assertTrue(approxEqual(sys.getCurrentProgress("p1"), 0.5f, 0.01f), "50% after resume");
}

static void testSkillQueueMoveToFront() {
    std::cout << "\n=== SkillQueue: MoveToFront ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.enqueueSkill("p1", "nav", 1, 100.0f);
    sys.enqueueSkill("p1", "gun", 1, 80.0f);
    sys.enqueueSkill("p1", "eng", 1, 60.0f);
    assertTrue(sys.getCurrentSkillId("p1") == "nav", "nav first");
    assertTrue(sys.moveToFront("p1", "eng"), "Move eng to front");
    assertTrue(sys.getCurrentSkillId("p1") == "eng", "eng now first");
    assertTrue(sys.getQueueLength("p1") == 3, "Still 3 in queue");
    // Already at front
    assertTrue(!sys.moveToFront("p1", "eng"), "Already at front");
    // Nonexistent
    assertTrue(!sys.moveToFront("p1", "missing"), "Missing skill");
}

static void testSkillQueueTotalTimes() {
    std::cout << "\n=== SkillQueue: TotalTimes ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.enqueueSkill("p1", "nav", 1, 60.0f);
    sys.enqueueSkill("p1", "gun", 1, 120.0f);
    assertTrue(approxEqual(sys.getTotalTrainingTime("p1"), 180.0f), "180s total");
    assertTrue(approxEqual(sys.getTotalRemainingTime("p1"), 180.0f), "180s remaining");
    sys.update(30.0f);
    assertTrue(approxEqual(sys.getTotalRemainingTime("p1"), 150.0f, 0.1f), "150s remaining after 30s");
}

static void testSkillQueueLevelUpgrade() {
    std::cout << "\n=== SkillQueue: LevelUpgrade ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    // Train nav to level 1
    sys.enqueueSkill("p1", "nav", 1, 30.0f);
    sys.update(35.0f);
    assertTrue(sys.getSkillLevel("p1", "nav") == 1, "nav level 1");
    // Train nav to level 2
    sys.enqueueSkill("p1", "nav", 2, 60.0f);
    sys.update(65.0f);
    assertTrue(sys.getSkillLevel("p1", "nav") == 2, "nav level 2");
    assertTrue(sys.getCompletedCount("p1") == 1, "Still 1 unique skill");
}

static void testSkillQueueUpdate() {
    std::cout << "\n=== SkillQueue: Update ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.enqueueSkill("p1", "nav", 1, 1000.0f);
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::SkillQueueState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testSkillQueueMissing() {
    std::cout << "\n=== SkillQueue: Missing ===" << std::endl;
    ecs::World world;
    systems::SkillQueueSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails");
    assertTrue(!sys.enqueueSkill("nonexistent", "s", 1, 1.0f), "enqueue fails");
    assertTrue(!sys.dequeueSkill("nonexistent", "s"), "dequeue fails");
    assertTrue(sys.getQueueLength("nonexistent") == 0, "0 queue");
    assertTrue(!sys.isQueued("nonexistent", "s"), "not queued");
    assertTrue(sys.getCurrentSkillId("nonexistent").empty(), "No current");
    assertTrue(approxEqual(sys.getCurrentProgress("nonexistent"), 0.0f), "0 progress");
    assertTrue(approxEqual(sys.getRemainingTime("nonexistent"), 0.0f), "0 remaining");
    assertTrue(!sys.isTraining("nonexistent"), "Not training");
    assertTrue(!sys.pauseTraining("nonexistent"), "pause fails");
    assertTrue(!sys.resumeTraining("nonexistent"), "resume fails");
    assertTrue(!sys.isPaused("nonexistent"), "not paused");
    assertTrue(sys.getCompletedCount("nonexistent") == 0, "0 completed");
    assertTrue(!sys.hasCompleted("nonexistent", "s"), "not completed");
    assertTrue(sys.getSkillLevel("nonexistent", "s") == 0, "0 level");
    assertTrue(!sys.moveToFront("nonexistent", "s"), "moveToFront fails");
    assertTrue(approxEqual(sys.getTotalTrainingTime("nonexistent"), 0.0f), "0 total time");
    assertTrue(approxEqual(sys.getTotalRemainingTime("nonexistent"), 0.0f), "0 total remaining");
}

void run_skill_queue_system_tests() {
    testSkillQueueCreate();
    testSkillQueueEnqueue();
    testSkillQueueMax();
    testSkillQueueDequeue();
    testSkillQueueTrainingProgress();
    testSkillQueueCompletion();
    testSkillQueuePauseResume();
    testSkillQueueMoveToFront();
    testSkillQueueTotalTimes();
    testSkillQueueLevelUpgrade();
    testSkillQueueUpdate();
    testSkillQueueMissing();
}
