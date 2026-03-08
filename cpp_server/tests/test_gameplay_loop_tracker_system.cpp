// Tests for: GameplayLoopTrackerSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/gameplay_loop_tracker_system.h"

using namespace atlas;
using Phase = components::GameplayLoopTrackerState::LoopPhase;

// ==================== GameplayLoopTrackerSystem Tests ====================

static void testLoopDefaultState() {
    std::cout << "\n=== GameplayLoopTracker: DefaultState ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::GameplayLoopTrackerState>(e);

    assertTrue(state->current_phase == Phase::Docked, "Default phase is Docked");
    assertTrue(state->loops_completed == 0, "Zero loops completed");
    assertTrue(state->total_undocks == 0, "Zero undocks");
    assertTrue(state->total_docks == 0, "Zero docks");
    assertTrue(!state->has_undocked, "Has not undocked");
    assertTrue(!state->has_mined, "Has not mined");
    assertTrue(!state->has_traded, "Has not traded");
    assertTrue(!state->has_fought, "Has not fought");
    assertTrue(sys.getLoopsCompleted("player1") == 0, "getLoopsCompleted 0");
    assertTrue(!sys.hasCompletedAllActivities("player1"), "Not all activities");
}

static void testLoopValidTransitions() {
    std::cout << "\n=== GameplayLoopTracker: ValidTransitions ===" << std::endl;

    // Docked -> Undocking
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Docked, Phase::Undocking),
               "Docked -> Undocking valid");
    // Undocking -> Flying
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Undocking, Phase::Flying),
               "Undocking -> Flying valid");
    // Flying -> Mining
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Flying, Phase::Mining),
               "Flying -> Mining valid");
    // Flying -> Combat
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Flying, Phase::Combat),
               "Flying -> Combat valid");
    // Flying -> Trading
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Flying, Phase::Trading),
               "Flying -> Trading valid");
    // Flying -> Docking
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Flying, Phase::Docking),
               "Flying -> Docking valid");
    // Mining -> Flying
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Mining, Phase::Flying),
               "Mining -> Flying valid");
    // Combat -> Flying
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Combat, Phase::Flying),
               "Combat -> Flying valid");
    // Docking -> Docked
    assertTrue(systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Docking, Phase::Docked),
               "Docking -> Docked valid");
}

static void testLoopInvalidTransitions() {
    std::cout << "\n=== GameplayLoopTracker: InvalidTransitions ===" << std::endl;

    // Docked -> Flying (must undock first)
    assertTrue(!systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Docked, Phase::Flying),
               "Docked -> Flying invalid");
    // Docked -> Mining
    assertTrue(!systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Docked, Phase::Mining),
               "Docked -> Mining invalid");
    // Flying -> Docked (must dock first)
    assertTrue(!systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Flying, Phase::Docked),
               "Flying -> Docked invalid");
    // Same-phase
    assertTrue(!systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Flying, Phase::Flying),
               "Flying -> Flying invalid (same phase)");
    // Mining -> Docked (must fly, then dock)
    assertTrue(!systems::GameplayLoopTrackerSystem::isValidTransition(Phase::Mining, Phase::Docked),
               "Mining -> Docked invalid");
}

static void testLoopFullCycle() {
    std::cout << "\n=== GameplayLoopTracker: FullCycle ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::GameplayLoopTrackerState>(e);

    // Full loop: Docked -> Undocking -> Flying -> Mining -> Flying -> Docking -> Docked
    assertTrue(sys.transitionTo("player1", Phase::Undocking), "Undock");
    assertTrue(sys.transitionTo("player1", Phase::Flying), "Fly");
    assertTrue(sys.transitionTo("player1", Phase::Mining), "Mine");
    assertTrue(sys.transitionTo("player1", Phase::Flying), "Fly back");
    assertTrue(sys.transitionTo("player1", Phase::Docking), "Dock");
    assertTrue(sys.transitionTo("player1", Phase::Docked), "Docked");

    assertTrue(sys.getLoopsCompleted("player1") == 1, "1 loop completed");
    assertTrue(sys.getTotalUndocks("player1") == 1, "1 undock");
}

static void testLoopMultipleCycles() {
    std::cout << "\n=== GameplayLoopTracker: MultipleCycles ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::GameplayLoopTrackerState>(e);

    for (int i = 0; i < 3; i++) {
        sys.transitionTo("player1", Phase::Undocking);
        sys.transitionTo("player1", Phase::Flying);
        sys.transitionTo("player1", Phase::Docking);
        sys.transitionTo("player1", Phase::Docked);
    }

    assertTrue(sys.getLoopsCompleted("player1") == 3, "3 loops completed");
    assertTrue(sys.getTotalUndocks("player1") == 3, "3 undocks");
}

static void testLoopMilestoneFlags() {
    std::cout << "\n=== GameplayLoopTracker: MilestoneFlags ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::GameplayLoopTrackerState>(e);

    sys.transitionTo("player1", Phase::Undocking);
    assertTrue(state->has_undocked, "has_undocked set");

    sys.transitionTo("player1", Phase::Flying);
    sys.transitionTo("player1", Phase::Mining);
    assertTrue(state->has_mined, "has_mined set");

    sys.transitionTo("player1", Phase::Flying);
    sys.transitionTo("player1", Phase::Combat);
    assertTrue(state->has_fought, "has_fought set");

    sys.transitionTo("player1", Phase::Flying);
    sys.transitionTo("player1", Phase::Trading);
    assertTrue(state->has_traded, "has_traded set");

    assertTrue(sys.hasCompletedAllActivities("player1"), "All activities completed");
}

static void testLoopInvalidTransitionRejected() {
    std::cout << "\n=== GameplayLoopTracker: InvalidTransitionRejected ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);
    auto* e = world.createEntity("player1");
    addComp<components::GameplayLoopTrackerState>(e);

    // Try to fly directly from docked (need undocking first)
    assertTrue(!sys.transitionTo("player1", Phase::Flying), "Cannot fly from docked");
    assertTrue(sys.getCurrentPhase("player1") == Phase::Docked, "Still docked");

    // Try same phase
    assertTrue(!sys.transitionTo("player1", Phase::Docked), "Cannot transition to same phase");
}

static void testLoopTimeTracking() {
    std::cout << "\n=== GameplayLoopTracker: TimeTracking ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::GameplayLoopTrackerState>(e);

    // Accumulate docked time
    sys.update(5.0f);
    assertTrue(approxEqual(state->total_docked_time, 5.0f), "Docked time accumulated");
    assertTrue(approxEqual(sys.getTimeInCurrentPhase("player1"), 5.0f), "Time in phase 5s");

    // Transition and accumulate flight time
    sys.transitionTo("player1", Phase::Undocking);
    sys.transitionTo("player1", Phase::Flying);
    sys.update(3.0f);
    assertTrue(approxEqual(state->total_flight_time, 3.0f), "Flight time accumulated");

    // Mining time
    sys.transitionTo("player1", Phase::Mining);
    sys.update(2.0f);
    assertTrue(approxEqual(state->total_mining_time, 2.0f), "Mining time accumulated");
}

static void testLoopCombatTime() {
    std::cout << "\n=== GameplayLoopTracker: CombatTime ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::GameplayLoopTrackerState>(e);

    sys.transitionTo("player1", Phase::Undocking);
    sys.transitionTo("player1", Phase::Flying);
    sys.transitionTo("player1", Phase::Combat);
    sys.update(4.0f);

    assertTrue(approxEqual(state->total_combat_time, 4.0f), "Combat time accumulated");
    assertTrue(state->total_combat_encounters == 1, "1 combat encounter");
}

static void testLoopHaulingTransitions() {
    std::cout << "\n=== GameplayLoopTracker: HaulingTransitions ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);
    auto* e = world.createEntity("player1");
    auto* state = addComp<components::GameplayLoopTrackerState>(e);

    sys.transitionTo("player1", Phase::Undocking);
    sys.transitionTo("player1", Phase::Flying);
    sys.transitionTo("player1", Phase::Mining);
    assertTrue(sys.transitionTo("player1", Phase::Hauling), "Mining -> Hauling valid");

    sys.update(2.0f);
    assertTrue(approxEqual(state->total_flight_time, 2.0f), "Hauling counts as flight time");

    assertTrue(sys.transitionTo("player1", Phase::Trading), "Hauling -> Trading valid");
    assertTrue(sys.transitionTo("player1", Phase::Docking), "Trading -> Docking valid");
    assertTrue(sys.transitionTo("player1", Phase::Docked), "Docking -> Docked valid");
}

static void testLoopMissing() {
    std::cout << "\n=== GameplayLoopTracker: Missing ===" << std::endl;
    ecs::World world;
    systems::GameplayLoopTrackerSystem sys(&world);

    assertTrue(!sys.transitionTo("x", Phase::Undocking), "Transition on missing");
    assertTrue(sys.getCurrentPhase("x") == Phase::Docked, "Phase on missing");
    assertTrue(sys.getLoopsCompleted("x") == 0, "Loops on missing");
    assertTrue(approxEqual(sys.getTimeInCurrentPhase("x"), 0.0f), "Time on missing");
    assertTrue(!sys.hasCompletedAllActivities("x"), "Activities on missing");
    assertTrue(sys.getTotalUndocks("x") == 0, "Undocks on missing");
}

void run_gameplay_loop_tracker_system_tests() {
    testLoopDefaultState();
    testLoopValidTransitions();
    testLoopInvalidTransitions();
    testLoopFullCycle();
    testLoopMultipleCycles();
    testLoopMilestoneFlags();
    testLoopInvalidTransitionRejected();
    testLoopTimeTracking();
    testLoopCombatTime();
    testLoopHaulingTransitions();
    testLoopMissing();
}
