// Tests for: View Mode Transition System Tests
#include "test_log.h"
#include "components/ui_components.h"
#include "ecs/system.h"
#include "systems/view_mode_transition_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== View Mode Transition System Tests ====================

static void testViewModeDefaults() {
    std::cout << "\n=== View Mode Defaults ===" << std::endl;
    components::ViewModeState state;
    assertTrue(state.current_mode == 1, "Default mode is Interior (1)");
    assertTrue(!state.transitioning, "Default not transitioning");
    assertTrue(approxEqual(state.transition_progress, 0.0f), "Default zero progress");
}

static void testViewModeInitialize() {
    std::cout << "\n=== View Mode Initialize ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);

    assertTrue(sys.initializePlayer("player1"), "Initialize succeeds");
    assertTrue(sys.getCurrentMode("player1") == 1, "Default Interior mode");
    assertTrue(!sys.isTransitioning("player1"), "Not transitioning initially");
}

static void testViewModeTransition() {
    std::cout << "\n=== View Mode Transition ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);
    sys.initializePlayer("player1");

    int cockpit = static_cast<int>(components::ViewModeState::Mode::Cockpit);
    assertTrue(sys.requestTransition("player1", cockpit), "Interior to Cockpit valid");
    assertTrue(sys.isTransitioning("player1"), "Transitioning after request");

    // Complete transition
    sys.update(2.0f);
    assertTrue(!sys.isTransitioning("player1"), "Not transitioning after completion");
    assertTrue(sys.getCurrentMode("player1") == cockpit, "Now in Cockpit mode");
}

static void testViewModeInvalidTransition() {
    std::cout << "\n=== View Mode Invalid Transition ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);
    sys.initializePlayer("player1");

    int eva = static_cast<int>(components::ViewModeState::Mode::EVA);
    // Interior -> EVA is a valid adjacent transition, so test a truly invalid one:
    // First transition to RTS Overlay, then try EVA which is not adjacent to RTS
    int rts = static_cast<int>(components::ViewModeState::Mode::RTSOverlay);
    sys.requestTransition("player1", rts);
    sys.update(2.0f); // complete transition to RTS
    assertTrue(!sys.requestTransition("player1", eva), "RTS to EVA invalid (not adjacent)");
}

static void testViewModeCancel() {
    std::cout << "\n=== View Mode Cancel ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);
    sys.initializePlayer("player1");

    int cockpit = static_cast<int>(components::ViewModeState::Mode::Cockpit);
    sys.requestTransition("player1", cockpit);
    assertTrue(sys.isTransitioning("player1"), "Transitioning");
    assertTrue(sys.cancelTransition("player1"), "Cancel succeeds");
    assertTrue(!sys.isTransitioning("player1"), "No longer transitioning");
    assertTrue(sys.getCurrentMode("player1") == 1, "Still in Interior after cancel");
}

static void testViewModeProgress() {
    std::cout << "\n=== View Mode Progress ===" << std::endl;
    ecs::World world;
    systems::ViewModeTransitionSystem sys(&world);
    sys.initializePlayer("player1");

    int cockpit = static_cast<int>(components::ViewModeState::Mode::Cockpit);
    sys.requestTransition("player1", cockpit);
    sys.update(0.75f); // Half of 1.5s default duration
    float progress = sys.getTransitionProgress("player1");
    assertTrue(progress > 0.0f && progress < 1.0f, "Progress is mid-transition");
    assertTrue(sys.isTransitioning("player1"), "Still transitioning");
}

static void testViewModeNames() {
    std::cout << "\n=== View Mode Names ===" << std::endl;
    assertTrue(systems::ViewModeTransitionSystem::getModeName(0) == "Cockpit", "Mode 0 is Cockpit");
    assertTrue(systems::ViewModeTransitionSystem::getModeName(1) == "Interior", "Mode 1 is Interior");
    assertTrue(systems::ViewModeTransitionSystem::getModeName(2) == "EVA", "Mode 2 is EVA");
    assertTrue(systems::ViewModeTransitionSystem::getModeName(3) == "RTS Overlay", "Mode 3 is RTS Overlay");
    assertTrue(systems::ViewModeTransitionSystem::getModeName(99) == "Unknown", "Invalid mode is Unknown");
}


void run_view_mode_transition_system_tests() {
    testViewModeDefaults();
    testViewModeInitialize();
    testViewModeTransition();
    testViewModeInvalidTransition();
    testViewModeCancel();
    testViewModeProgress();
    testViewModeNames();
}
