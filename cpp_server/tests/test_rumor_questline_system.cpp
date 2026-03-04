// Tests for: Rumor Questline System Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/narrative_components.h"
#include "ecs/system.h"
#include "systems/rumor_questline_system.h"

using namespace atlas;

// ==================== Rumor Questline System Tests ====================

static void testRumorQuestlineNoGraduation() {
    std::cout << "\n=== Rumor Questline No Graduation ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rumor1");
    auto* log = addComp<components::RumorLog>(e);
    log->addRumor("r1", "Some rumor", false);
    log->rumors[0].times_heard = 2;
    log->rumors[0].belief_strength = 0.8f;

    systems::RumorQuestlineSystem sys(&world);
    sys.update(1.0f);
    assertTrue(!sys.hasGraduatedRumor("rumor1", "r1"), "Rumor not graduated with < 3 hearings");
}

static void testRumorQuestlineGraduates() {
    std::cout << "\n=== Rumor Questline Graduates ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rumor2");
    auto* log = addComp<components::RumorLog>(e);
    log->addRumor("r1", "A strong rumor", false);
    log->rumors[0].times_heard = 3;
    log->rumors[0].belief_strength = 0.8f;

    systems::RumorQuestlineSystem sys(&world);
    sys.update(1.0f);
    assertTrue(sys.hasGraduatedRumor("rumor2", "r1"), "Rumor graduated with >= 3 hearings and belief >= 0.7");
    auto quests = sys.getGraduatedQuestlines("rumor2");
    assertTrue(quests.size() == 1, "One questline generated");
}

static void testRumorQuestlineLowBeliefNoGraduation() {
    std::cout << "\n=== Rumor Questline Low Belief ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("rumor3");
    auto* log = addComp<components::RumorLog>(e);
    log->addRumor("r1", "Weak rumor", false);
    log->rumors[0].times_heard = 5;
    log->rumors[0].belief_strength = 0.3f;

    systems::RumorQuestlineSystem sys(&world);
    sys.update(1.0f);
    assertTrue(!sys.hasGraduatedRumor("rumor3", "r1"), "Low belief prevents graduation");
}


void run_rumor_questline_system_tests() {
    testRumorQuestlineNoGraduation();
    testRumorQuestlineGraduates();
    testRumorQuestlineLowBeliefNoGraduation();
}
