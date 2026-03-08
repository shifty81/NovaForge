// Tests for: HangarTransitionSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/hangar_transition_system.h"

using namespace atlas;
using Phase = components::HangarTransitionState::TransitionPhase;

// ==================== HangarTransitionSystem Tests ====================

static void testHangarDefaultState() {
    std::cout << "\n=== HangarTransition: DefaultState ===" << std::endl;
    ecs::World world;
    systems::HangarTransitionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    auto* state = addComp<components::HangarTransitionState>(e);

    assertTrue(state->phase == Phase::Idle, "Default phase Idle");
    assertTrue(approxEqual(state->phase_timer, 0.0f), "Default timer 0");
    assertTrue(approxEqual(state->dock_approach_duration, 3.0f), "Approach 3s");
    assertTrue(approxEqual(state->dock_sequence_duration, 5.0f), "Sequence 5s");
    assertTrue(approxEqual(state->undock_sequence_duration, 4.0f), "Undock seq 4s");
    assertTrue(approxEqual(state->undock_launch_duration, 2.0f), "Undock launch 2s");
    assertTrue(state->target_station_id.empty(), "No target station");
    assertTrue(state->total_docks == 0, "0 docks");
    assertTrue(state->total_undocks == 0, "0 undocks");
    assertTrue(!state->animation_playing, "Not animating");
    assertTrue(approxEqual(state->animation_progress, 0.0f), "Progress 0");
    assertTrue(state->active, "Active");
}

static void testHangarFullDockSequence() {
    std::cout << "\n=== HangarTransition: FullDockSequence ===" << std::endl;
    ecs::World world;
    systems::HangarTransitionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    auto* state = addComp<components::HangarTransitionState>(e);

    assertTrue(sys.beginDock("ship1", "station_alpha"), "Begin dock OK");
    assertTrue(state->phase == Phase::DockApproach, "Phase DockApproach");
    assertTrue(state->target_station_id == "station_alpha", "Station stored");
    assertTrue(state->animation_playing, "Animating during approach");
    assertTrue(sys.isDocking("ship1"), "isDocking true");
    assertTrue(!sys.isDocked("ship1"), "Not yet docked");

    // Advance halfway through approach (3s duration)
    sys.update(1.5f);
    assertTrue(approxEqual(state->animation_progress, 0.5f), "Approach 50%");

    // Complete approach phase
    sys.update(2.0f);
    assertTrue(state->phase == Phase::DockSequence, "Phase DockSequence");

    // Advance through dock sequence (5s)
    sys.update(2.5f);
    assertTrue(approxEqual(state->animation_progress, 0.5f), "Sequence 50%");

    sys.update(3.0f);
    assertTrue(state->phase == Phase::DockComplete, "Phase DockComplete");
    assertTrue(sys.isDocked("ship1"), "isDocked true");
    assertTrue(!sys.isDocking("ship1"), "isDocking false after complete");
    assertTrue(state->total_docks == 1, "1 dock completed");
}

static void testHangarFullUndockSequence() {
    std::cout << "\n=== HangarTransition: FullUndockSequence ===" << std::endl;
    ecs::World world;
    systems::HangarTransitionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    auto* state = addComp<components::HangarTransitionState>(e);

    // Dock first
    sys.beginDock("ship1", "st1");
    sys.update(3.5f); // complete approach
    sys.update(5.5f); // complete sequence
    assertTrue(sys.isDocked("ship1"), "Docked");

    assertTrue(sys.beginUndock("ship1"), "Begin undock OK");
    assertTrue(state->phase == Phase::UndockSequence, "Phase UndockSequence");
    assertTrue(sys.isUndocking("ship1"), "isUndocking true");

    // Advance through undock sequence (4s)
    sys.update(2.0f);
    assertTrue(approxEqual(state->animation_progress, 0.5f), "Undock seq 50%");

    sys.update(2.5f);
    assertTrue(state->phase == Phase::UndockLaunch, "Phase UndockLaunch");

    // Advance through launch (2s)
    sys.update(1.0f);
    assertTrue(approxEqual(state->animation_progress, 0.5f), "Launch 50%");

    sys.update(1.5f);
    // Should auto-advance through UndockComplete to Idle
    assertTrue(state->phase == Phase::Idle, "Back to Idle");
    assertTrue(sys.isIdle("ship1"), "isIdle true");
    assertTrue(state->total_undocks == 1, "1 undock completed");
}

static void testHangarCannotDockWhenNotIdle() {
    std::cout << "\n=== HangarTransition: CannotDockWhenNotIdle ===" << std::endl;
    ecs::World world;
    systems::HangarTransitionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    addComp<components::HangarTransitionState>(e);

    sys.beginDock("ship1", "st1");
    assertTrue(!sys.beginDock("ship1", "st2"), "Cannot dock during approach");

    // Complete dock
    sys.update(4.0f);
    sys.update(6.0f);
    assertTrue(!sys.beginDock("ship1", "st2"), "Cannot dock when docked");
}

static void testHangarCannotUndockWhenNotDocked() {
    std::cout << "\n=== HangarTransition: CannotUndockWhenNotDocked ===" << std::endl;
    ecs::World world;
    systems::HangarTransitionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    addComp<components::HangarTransitionState>(e);

    assertTrue(!sys.beginUndock("ship1"), "Cannot undock from Idle");

    sys.beginDock("ship1", "st1");
    assertTrue(!sys.beginUndock("ship1"), "Cannot undock during approach");
}

static void testHangarAnimationProgress() {
    std::cout << "\n=== HangarTransition: AnimationProgress ===" << std::endl;
    ecs::World world;
    systems::HangarTransitionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    addComp<components::HangarTransitionState>(e);

    assertTrue(approxEqual(sys.getAnimationProgress("ship1"), 0.0f), "Initial progress 0");

    sys.beginDock("ship1", "st1");
    sys.update(1.5f);
    float prog = sys.getAnimationProgress("ship1");
    assertTrue(approxEqual(prog, 0.5f), "Approach 50%");

    sys.update(1.5f);
    assertTrue(approxEqual(sys.getAnimationProgress("ship1"), 0.0f), "Reset after phase change");
}

static void testHangarDockUndockCounts() {
    std::cout << "\n=== HangarTransition: DockUndockCounts ===" << std::endl;
    ecs::World world;
    systems::HangarTransitionSystem sys(&world);
    auto* e = world.createEntity("ship1");
    addComp<components::HangarTransitionState>(e);

    // Do two full dock/undock cycles
    for (int i = 0; i < 2; i++) {
        sys.beginDock("ship1", "st1");
        sys.update(4.0f);
        sys.update(6.0f);
        sys.beginUndock("ship1");
        sys.update(5.0f);
        sys.update(3.0f);
    }

    assertTrue(sys.getTotalDocks("ship1") == 2, "2 total docks");
    assertTrue(sys.getTotalUndocks("ship1") == 2, "2 total undocks");
}

static void testHangarMissingEntity() {
    std::cout << "\n=== HangarTransition: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::HangarTransitionSystem sys(&world);

    assertTrue(!sys.beginDock("x", "st1"), "beginDock on missing");
    assertTrue(!sys.beginUndock("x"), "beginUndock on missing");
    assertTrue(sys.getPhase("x") == Phase::Idle, "Phase on missing");
    assertTrue(approxEqual(sys.getAnimationProgress("x"), 0.0f), "Progress on missing");
    assertTrue(!sys.isDocking("x"), "isDocking on missing");
    assertTrue(!sys.isUndocking("x"), "isUndocking on missing");
    assertTrue(sys.isIdle("x"), "isIdle on missing");
    assertTrue(!sys.isDocked("x"), "isDocked on missing");
    assertTrue(sys.getTotalDocks("x") == 0, "Docks on missing");
    assertTrue(sys.getTotalUndocks("x") == 0, "Undocks on missing");
}

void run_hangar_transition_system_tests() {
    testHangarDefaultState();
    testHangarFullDockSequence();
    testHangarFullUndockSequence();
    testHangarCannotDockWhenNotIdle();
    testHangarCannotUndockWhenNotDocked();
    testHangarAnimationProgress();
    testHangarDockUndockCounts();
    testHangarMissingEntity();
}
