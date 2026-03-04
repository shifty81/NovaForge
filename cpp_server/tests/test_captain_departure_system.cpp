// Tests for: Captain Departure System Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/captain_departure_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Captain Departure System Tests ====================

static void testDeparturePhaseNone() {
    std::cout << "\n=== Departure Phase None ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep1");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 2.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f);
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep1") == DP::None, "Low disagreement stays None");
}

static void testDeparturePhaseGrumbling() {
    std::cout << "\n=== Departure Phase Grumbling ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep2");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 6.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f);
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep2") == DP::Grumbling, "Score >= grumble_threshold transitions to Grumbling");
}

static void testDeparturePhaseRequest() {
    std::cout << "\n=== Departure Phase Request ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep3");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 11.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f);
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep3") == DP::FormalRequest, "Score >= request_threshold transitions to FormalRequest");
}

static void testDepartureAcknowledge() {
    std::cout << "\n=== Departure Acknowledge ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep4");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 11.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f);
    sys.acknowledgeRequest("dep4");
    sys.update(0.1f);
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep4") == DP::Departing, "Acknowledge causes transition to Departing");
}

static void testDepartureTimerCountdown() {
    std::cout << "\n=== Departure Timer Countdown ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("dep5");
    auto* state = addComp<components::CaptainDepartureState>(e);
    state->disagreement_score = 11.0f;
    state->departure_delay = 10.0f;

    systems::CaptainDepartureSystem sys(&world);
    sys.update(1.0f); // transitions to FormalRequest
    using DP = components::CaptainDepartureState::DeparturePhase;
    assertTrue(sys.getDeparturePhase("dep5") == DP::FormalRequest, "In FormalRequest phase");
    sys.update(5.0f);
    assertTrue(state->departure_timer < 10.0f, "Timer counting down");
    sys.update(6.0f);
    assertTrue(sys.getDeparturePhase("dep5") == DP::Departing, "Timer expired, now Departing");
}


void run_captain_departure_system_tests() {
    testDeparturePhaseNone();
    testDeparturePhaseGrumbling();
    testDeparturePhaseRequest();
    testDepartureAcknowledge();
    testDepartureTimerCountdown();
}
