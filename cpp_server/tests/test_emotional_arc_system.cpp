// Tests for: EmotionalArcSystem Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/emotional_arc_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== EmotionalArcSystem Tests ====================

static void testEmotionalArcVictory() {
    std::cout << "\n=== Emotional Arc Victory ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    world.createEntity("cap1");
    float baseline = sys.getConfidence("cap1");
    sys.onCombatVictory("cap1");
    assertTrue(sys.getConfidence("cap1") > baseline, "Confidence increased after victory");
}

static void testEmotionalArcDefeat() {
    std::cout << "\n=== Emotional Arc Defeat ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline_conf = sys.getConfidence("cap1");
    float baseline_fat = sys.getFatigue("cap1");
    sys.onCombatDefeat("cap1");
    assertTrue(sys.getConfidence("cap1") < baseline_conf, "Confidence decreased after defeat");
    assertTrue(sys.getFatigue("cap1") > baseline_fat, "Fatigue increased after defeat");
}

static void testEmotionalArcRest() {
    std::cout << "\n=== Emotional Arc Rest ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* state = addComp<components::EmotionalState>(entity);
    state->fatigue = 50.0f;
    sys.onRest("cap1");
    assertTrue(state->fatigue < 50.0f, "Fatigue decreased after rest");
}

static void testEmotionalArcTrust() {
    std::cout << "\n=== Emotional Arc Trust ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline = sys.getTrust("cap1");
    sys.onPlayerTrust("cap1");
    assertTrue(sys.getTrust("cap1") > baseline, "Trust increased after player trust");
}

static void testEmotionalArcBetray() {
    std::cout << "\n=== Emotional Arc Betray ===" << std::endl;
    ecs::World world;
    systems::EmotionalArcSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    addComp<components::EmotionalState>(entity);
    float baseline = sys.getTrust("cap1");
    sys.onPlayerBetray("cap1");
    assertTrue(sys.getTrust("cap1") < baseline, "Trust decreased after betrayal");
}


void run_emotional_arc_system_tests() {
    testEmotionalArcVictory();
    testEmotionalArcDefeat();
    testEmotionalArcRest();
    testEmotionalArcTrust();
    testEmotionalArcBetray();
}
