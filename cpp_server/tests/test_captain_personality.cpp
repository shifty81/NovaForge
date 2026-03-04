// Tests for: Phase 9: Disagreement Model Tests, Phase 9: Silence Interpretation Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/fleet_chatter_system.h"

using namespace atlas;

// ==================== Phase 9: Disagreement Model Tests ====================

static void testDisagreementBasicScore() {
    std::cout << "\n=== Disagreement Basic Score ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* pers = addComp<components::CaptainPersonality>(entity);
    pers->aggression = 0.2f;   // cautious
    pers->optimism = 0.3f;     // somewhat grim
    auto* morale = addComp<components::FleetMorale>(entity);
    morale->losses = 5;

    // risk=0.8, no task mismatch
    float score = sys.computeDisagreement("cap1", 0.8f, false);
    // expected: 0.8*(1-0.2) + 5*(1-0.3) = 0.64 + 3.5 = 4.14
    assertTrue(approxEqual(score, 4.14f, 0.1f), "Disagreement score ~4.14");
}

static void testDisagreementTaskMismatch() {
    std::cout << "\n=== Disagreement Task Mismatch ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* pers = addComp<components::CaptainPersonality>(entity);
    pers->aggression = 0.5f;
    pers->optimism = 0.5f;

    float noMismatch = sys.computeDisagreement("cap1", 0.5f, false);
    float withMismatch = sys.computeDisagreement("cap1", 0.5f, true);
    assertTrue(withMismatch - noMismatch >= 9.9f, "Task mismatch adds +10 to score");
}

static void testDisagreementAggressiveLow() {
    std::cout << "\n=== Disagreement Aggressive Captain Low Score ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* entity = world.createEntity("cap1");
    auto* pers = addComp<components::CaptainPersonality>(entity);
    pers->aggression = 0.9f;  // very aggressive → tolerates risk
    pers->optimism = 0.9f;    // very optimistic → shrugs off losses

    float score = sys.computeDisagreement("cap1", 1.0f, false);
    // expected: 1.0*(1-0.9) + 0*(1-0.9) = 0.1
    assertTrue(score < 1.0f, "Aggressive+optimistic captain has low disagreement");
}


// ==================== Phase 9: Silence Interpretation Tests ====================

static void testSilenceInterpretationTriggered() {
    std::cout << "\n=== Silence Interpretation Triggered ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* player = world.createEntity("player");
    auto* presence = addComp<components::PlayerPresence>(player);
    presence->time_since_last_command = 150.0f;  // >120s threshold

    auto* cap = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(cap);
    addComp<components::FleetChatterState>(cap);

    std::string line = sys.getSilenceAwareLine("cap1", "player");
    assertTrue(!line.empty(), "Captain speaks about silence");

    // Should be one of the silence lines
    bool isSilenceLine = (line.find("Quiet") != std::string::npos ||
                          line.find("alright") != std::string::npos ||
                          line.find("heard from you") != std::string::npos ||
                          line.find("okay") != std::string::npos ||
                          line.find("checking in") != std::string::npos);
    assertTrue(isSilenceLine, "Line is a silence interpretation");
}

static void testSilenceInterpretationNotTriggered() {
    std::cout << "\n=== Silence Interpretation Not Triggered ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* player = world.createEntity("player");
    auto* presence = addComp<components::PlayerPresence>(player);
    presence->time_since_last_command = 30.0f;  // <120s, no silence

    auto* cap = world.createEntity("cap1");
    addComp<components::CaptainPersonality>(cap);
    addComp<components::FleetChatterState>(cap);
    sys.setActivity("cap1", "Idle");

    std::string line = sys.getSilenceAwareLine("cap1", "player");
    // Should fall back to contextual line (not silence-specific)
    assertTrue(!line.empty(), "Captain speaks (contextual fallback)");
}


void run_captain_personality_tests() {
    testDisagreementBasicScore();
    testDisagreementTaskMismatch();
    testDisagreementAggressiveLow();
    testSilenceInterpretationTriggered();
    testSilenceInterpretationNotTriggered();
}
