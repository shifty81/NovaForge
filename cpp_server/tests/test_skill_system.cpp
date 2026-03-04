// Tests for: Skill System Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/skill_system.h"

using namespace atlas;

// ==================== Skill System Tests ====================

static void testSkillTraining() {
    std::cout << "\n=== Skill Training ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    // Queue skill training
    bool queued = skillSys.queueSkillTraining("player1", "gunnery_001",
        "Small Projectile Turret", 1, 60.0f);
    assertTrue(queued, "Skill training queued");

    auto* skillset = player->getComponent<components::SkillSet>();
    assertTrue(skillset->training_queue.size() == 1, "One skill in queue");

    // Partially train
    skillSys.update(30.0f);
    assertTrue(skillSys.getSkillLevel("player1", "gunnery_001") == 0,
               "Skill not yet complete after 30s");

    // Complete training
    skillSys.update(35.0f);
    assertTrue(skillSys.getSkillLevel("player1", "gunnery_001") == 1,
               "Skill trained to level 1 after 65s");
    assertTrue(skillset->training_queue.empty(), "Queue empty after completion");
    assertTrue(skillset->total_sp > 0.0, "SP awarded");
}

static void testSkillInstantTrain() {
    std::cout << "\n=== Skill Instant Train ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    bool trained = skillSys.trainSkillInstant("player1", "nav_001",
        "Navigation", 3);
    assertTrue(trained, "Instant train succeeds");
    assertTrue(skillSys.getSkillLevel("player1", "nav_001") == 3,
               "Skill is level 3");
}

static void testSkillQueueMultiple() {
    std::cout << "\n=== Skill Queue Multiple ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    skillSys.queueSkillTraining("player1", "skill_a", "Skill A", 1, 10.0f);
    skillSys.queueSkillTraining("player1", "skill_b", "Skill B", 1, 20.0f);

    auto* skillset = player->getComponent<components::SkillSet>();
    assertTrue(skillset->training_queue.size() == 2, "Two skills in queue");

    // Complete first
    skillSys.update(12.0f);
    assertTrue(skillSys.getSkillLevel("player1", "skill_a") == 1, "First skill complete");
    assertTrue(skillset->training_queue.size() == 1, "One skill remaining");

    // Complete second
    skillSys.update(20.0f);
    assertTrue(skillSys.getSkillLevel("player1", "skill_b") == 1, "Second skill complete");
    assertTrue(skillset->training_queue.empty(), "Queue empty");
}

static void testSkillInvalidLevel() {
    std::cout << "\n=== Skill Invalid Level ===" << std::endl;

    ecs::World world;
    systems::SkillSystem skillSys(&world);

    auto* player = world.createEntity("player1");
    addComp<components::SkillSet>(player);

    bool result = skillSys.queueSkillTraining("player1", "test", "Test", 6, 10.0f);
    assertTrue(!result, "Level 6 rejected (max is 5)");

    result = skillSys.queueSkillTraining("player1", "test", "Test", 0, 10.0f);
    assertTrue(!result, "Level 0 rejected (min is 1)");
}


void run_skill_system_tests() {
    testSkillTraining();
    testSkillInstantTrain();
    testSkillQueueMultiple();
    testSkillInvalidLevel();
}
