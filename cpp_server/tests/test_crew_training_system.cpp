// Tests for: CrewTraining System Tests
#include "test_log.h"
#include "components/crew_components.h"
#include "ecs/system.h"
#include "systems/crew_training_system.h"

using namespace atlas;

// ==================== CrewTraining System Tests ====================

static void testCrewTrainingCreate() {
    std::cout << "\n=== CrewTraining: Create ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeTraining("ship1"), "Init training succeeds");
    assertTrue(sys.getTraineeCount("ship1") == 0, "No trainees initially");
    assertTrue(sys.getTotalCompleted("ship1") == 0, "No completions initially");
    assertTrue(approxEqual(sys.getXpBonus("ship1"), 1.0f), "Default XP bonus is 1.0");
}

static void testCrewTrainingEnroll() {
    std::cout << "\n=== CrewTraining: Enroll ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    assertTrue(sys.enrollTrainee("ship1", "crew1", "Navigation"), "Enroll succeeds");
    assertTrue(sys.getTraineeCount("ship1") == 1, "1 trainee");
    assertTrue(sys.getSkillName("ship1", "crew1") == "Navigation", "Skill is Navigation");
    assertTrue(approxEqual(sys.getProgress("ship1", "crew1"), 0.0f), "Initial progress is 0");
}

static void testCrewTrainingDuplicate() {
    std::cout << "\n=== CrewTraining: Duplicate ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Navigation");
    assertTrue(!sys.enrollTrainee("ship1", "crew1", "Weapons"), "Duplicate trainee rejected");
    assertTrue(sys.getTraineeCount("ship1") == 1, "Still 1 trainee");
}

static void testCrewTrainingProgress() {
    std::cout << "\n=== CrewTraining: Progress ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Gunnery");
    // training_rate=0.01, xp_bonus=1.0, delta=10 → progress = 0.1
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getProgress("ship1", "crew1"), 0.1f), "Progress is 0.1 after 10s");
    assertTrue(!sys.isComplete("ship1", "crew1"), "Not complete yet");
}

static void testCrewTrainingCompletion() {
    std::cout << "\n=== CrewTraining: Completion ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Engineering");
    // training_rate=0.01, xp_bonus=1.0, delta=100 → progress = 1.0
    sys.update(100.0f);
    assertTrue(approxEqual(sys.getProgress("ship1", "crew1"), 1.0f), "Progress capped at 1.0");
    assertTrue(sys.isComplete("ship1", "crew1"), "Training complete");
    assertTrue(sys.getTotalCompleted("ship1") == 1, "1 completion counted");
}

static void testCrewTrainingXpBonus() {
    std::cout << "\n=== CrewTraining: XpBonus ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Shields");
    assertTrue(sys.setXpBonus("ship1", 2.0f), "Set XP bonus succeeds");
    assertTrue(approxEqual(sys.getXpBonus("ship1"), 2.0f), "XP bonus is 2.0");
    // training_rate=0.01, xp_bonus=2.0, delta=10 → progress = 0.2
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getProgress("ship1", "crew1"), 0.2f), "Progress 0.2 with 2x bonus");
}

static void testCrewTrainingRemove() {
    std::cout << "\n=== CrewTraining: Remove ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Navigation");
    sys.enrollTrainee("ship1", "crew2", "Weapons");
    assertTrue(sys.removeTrainee("ship1", "crew1"), "Remove succeeds");
    assertTrue(sys.getTraineeCount("ship1") == 1, "1 trainee remains");
    assertTrue(!sys.removeTrainee("ship1", "crew1"), "Remove nonexistent fails");
}

static void testCrewTrainingMaxLimit() {
    std::cout << "\n=== CrewTraining: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    // default max_trainees = 5
    sys.enrollTrainee("ship1", "c1", "Skill1");
    sys.enrollTrainee("ship1", "c2", "Skill2");
    sys.enrollTrainee("ship1", "c3", "Skill3");
    sys.enrollTrainee("ship1", "c4", "Skill4");
    sys.enrollTrainee("ship1", "c5", "Skill5");
    assertTrue(!sys.enrollTrainee("ship1", "c6", "Skill6"), "Max limit enforced at 5");
    assertTrue(sys.getTraineeCount("ship1") == 5, "Still 5 trainees");
}

static void testCrewTrainingMultiComplete() {
    std::cout << "\n=== CrewTraining: MultiComplete ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeTraining("ship1");
    sys.enrollTrainee("ship1", "crew1", "Gunnery");
    sys.enrollTrainee("ship1", "crew2", "Shields");
    sys.update(100.0f); // both complete
    assertTrue(sys.getTotalCompleted("ship1") == 2, "2 completions");
    assertTrue(sys.isComplete("ship1", "crew1"), "crew1 complete");
    assertTrue(sys.isComplete("ship1", "crew2"), "crew2 complete");
    // completed training doesn't increment again
    sys.update(10.0f);
    assertTrue(sys.getTotalCompleted("ship1") == 2, "Still 2 completions");
}

static void testCrewTrainingMissing() {
    std::cout << "\n=== CrewTraining: Missing ===" << std::endl;
    ecs::World world;
    systems::CrewTrainingSystem sys(&world);
    assertTrue(!sys.initializeTraining("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.enrollTrainee("nonexistent", "c1", "Skill"), "Enroll fails on missing");
    assertTrue(!sys.removeTrainee("nonexistent", "c1"), "Remove fails on missing");
    assertTrue(sys.getTraineeCount("nonexistent") == 0, "0 trainees on missing");
    assertTrue(approxEqual(sys.getProgress("nonexistent", "c1"), 0.0f), "0 progress on missing");
    assertTrue(!sys.isComplete("nonexistent", "c1"), "Not complete on missing");
    assertTrue(sys.getTotalCompleted("nonexistent") == 0, "0 completions on missing");
    assertTrue(!sys.setXpBonus("nonexistent", 2.0f), "Set XP fails on missing");
    assertTrue(approxEqual(sys.getXpBonus("nonexistent"), 0.0f), "0 XP bonus on missing");
    assertTrue(sys.getSkillName("nonexistent", "c1") == "", "Empty skill on missing");
}


void run_crew_training_system_tests() {
    testCrewTrainingCreate();
    testCrewTrainingEnroll();
    testCrewTrainingDuplicate();
    testCrewTrainingProgress();
    testCrewTrainingCompletion();
    testCrewTrainingXpBonus();
    testCrewTrainingRemove();
    testCrewTrainingMaxLimit();
    testCrewTrainingMultiComplete();
    testCrewTrainingMissing();
}
