// Tests for: Phase 9: Rumor Propagation Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/narrative_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/fleet_chatter_system.h"

using namespace atlas;

// ==================== Phase 9: Rumor Propagation Tests ====================

static void testRumorPropagationNewRumor() {
    std::cout << "\n=== Rumor Propagation New Rumor ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* speaker = world.createEntity("speaker");
    auto* speakerLog = addComp<components::RumorLog>(speaker);
    speakerLog->addRumor("ancient_gate", "There's an old gate near Sigma-7", true);

    auto* listener = world.createEntity("listener");
    // No RumorLog yet

    sys.propagateRumor("speaker", "listener");

    auto* listenerLog = listener->getComponent<components::RumorLog>();
    assertTrue(listenerLog != nullptr, "Listener gained RumorLog");
    assertTrue(listenerLog->hasRumor("ancient_gate"), "Rumor propagated to listener");
    // Second-hand: belief should be halved
    float belief = 0.0f;
    for (const auto& r : listenerLog->rumors) {
        if (r.rumor_id == "ancient_gate") belief = r.belief_strength;
    }
    assertTrue(approxEqual(belief, 0.25f, 0.01f), "Second-hand belief is halved (0.25)");
}

static void testRumorPropagationReinforces() {
    std::cout << "\n=== Rumor Propagation Reinforces ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* speaker = world.createEntity("speaker");
    auto* speakerLog = addComp<components::RumorLog>(speaker);
    speakerLog->addRumor("derelict_ship", "Derelict spotted in belt", true);

    auto* listener = world.createEntity("listener");
    auto* listenerLog = addComp<components::RumorLog>(listener);
    listenerLog->addRumor("derelict_ship", "Derelict spotted in belt", false);

    float initialBelief = 0.0f;
    for (const auto& r : listenerLog->rumors) {
        if (r.rumor_id == "derelict_ship") initialBelief = r.belief_strength;
    }

    sys.propagateRumor("speaker", "listener");

    float newBelief = 0.0f;
    int timesHeard = 0;
    for (const auto& r : listenerLog->rumors) {
        if (r.rumor_id == "derelict_ship") {
            newBelief = r.belief_strength;
            timesHeard = r.times_heard;
        }
    }
    assertTrue(newBelief > initialBelief, "Belief reinforced after hearing again");
    assertTrue(timesHeard == 2, "Times heard incremented");
}

static void testRumorPropagationNoRumors() {
    std::cout << "\n=== Rumor Propagation No Rumors ===" << std::endl;
    ecs::World world;
    systems::FleetChatterSystem sys(&world);
    auto* speaker = world.createEntity("speaker");
    addComp<components::RumorLog>(speaker);  // empty rumor log

    auto* listener = world.createEntity("listener");

    sys.propagateRumor("speaker", "listener");

    auto* listenerLog = listener->getComponent<components::RumorLog>();
    assertTrue(listenerLog == nullptr, "No propagation when speaker has no rumors");
}


void run_rumor_propagation_tests() {
    testRumorPropagationNewRumor();
    testRumorPropagationReinforces();
    testRumorPropagationNoRumors();
}
