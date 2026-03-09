// Tests for: SpacePlanetTransitionSystem Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/space_planet_transition_system.h"

using namespace atlas;

// ==================== SpacePlanetTransitionSystem Tests ====================

static void testSPTInitialize() {
    std::cout << "\n=== SPTSystem: Initialize ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);

    world.createEntity("ship_1");
    assertTrue(sys.initializeTransition("ship_1", "planet_a", true),
               "Transition initialized with atmosphere");
    assertTrue(sys.getTransitionState("ship_1") == "in_space", "Initial state is in_space");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 1000.0f), "Initial altitude is 1000");
    assertTrue(approxEqual(sys.getHeatLevel("ship_1"), 0.0f), "Initial heat is 0");

    world.createEntity("ship_2");
    assertTrue(sys.initializeTransition("ship_2", "planet_b", false),
               "Transition initialized without atmosphere");
    assertTrue(sys.getTransitionState("ship_2") == "in_space", "No-atmo initial state is in_space");

    assertTrue(!sys.initializeTransition("ship_1", "planet_c", true),
               "Duplicate init fails");
}

static void testSPTBeginDescent() {
    std::cout << "\n=== SPTSystem: Begin Descent ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);

    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_a", true);
    assertTrue(sys.beginDescent("ship_1"), "Descent begun from InSpace");
    assertTrue(sys.getTransitionState("ship_1") == "orbit_entry", "State is orbit_entry");
    assertTrue(!sys.beginDescent("ship_1"), "Cannot begin descent again from OrbitEntry");
}

static void testSPTDescentPhaseProgression() {
    std::cout << "\n=== SPTSystem: Descent Phase Progression ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);

    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_a", true);
    sys.beginDescent("ship_1");
    assertTrue(sys.getTransitionState("ship_1") == "orbit_entry", "Started at orbit_entry");

    for (int i = 0; i < 200; ++i) sys.update(0.1f);

    std::string state = sys.getTransitionState("ship_1");
    assertTrue(state != "in_space", "No longer in_space after updates");
    assertTrue(sys.getAltitude("ship_1") < 1000.0f, "Altitude decreased during descent");

    for (int i = 0; i < 600; ++i) sys.update(0.1f);

    float alt_after = sys.getAltitude("ship_1");
    assertTrue(alt_after < 500.0f, "Altitude significantly reduced after extended descent");
}

static void testSPTBeginLaunch() {
    std::cout << "\n=== SPTSystem: Begin Launch ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);

    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_a", true);

    // Manually progress to Landed by running descent to completion
    sys.beginDescent("ship_1");
    for (int i = 0; i < 2000; ++i) sys.update(0.1f);

    // If not yet landed, manually set via component
    auto* entity = world.getEntity("ship_1");
    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    tr->transition_state = components::SpacePlanetTransition::TransitionState::Landed;
    tr->altitude = 0.0f;

    assertTrue(sys.beginLaunch("ship_1"), "Launch begun from Landed");
    assertTrue(sys.getTransitionState("ship_1") == "launch_sequence", "State is launch_sequence");
    assertTrue(!sys.beginLaunch("ship_1"), "Cannot launch again from LaunchSequence");
}

static void testSPTAbortTransition() {
    std::cout << "\n=== SPTSystem: Abort Transition ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);

    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_a", true);
    sys.beginDescent("ship_1");
    assertTrue(sys.getTransitionState("ship_1") == "orbit_entry", "In orbit_entry before abort");

    assertTrue(sys.abortTransition("ship_1"), "Abort from OrbitEntry succeeds");
    assertTrue(sys.getTransitionState("ship_1") == "in_space", "State returned to in_space");
    assertTrue(approxEqual(sys.getAltitude("ship_1"), 1000.0f), "Altitude reset to 1000");
}

static void testSPTAbortFromInvalidStates() {
    std::cout << "\n=== SPTSystem: Abort From Invalid States ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);

    // Abort from InSpace
    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_a", true);
    assertTrue(!sys.abortTransition("ship_1"), "Cannot abort from InSpace");

    // Abort from Landed
    world.createEntity("ship_2");
    sys.initializeTransition("ship_2", "planet_b", false);
    auto* entity = world.getEntity("ship_2");
    auto* tr = entity->getComponent<components::SpacePlanetTransition>();
    tr->transition_state = components::SpacePlanetTransition::TransitionState::Landed;
    tr->altitude = 0.0f;
    assertTrue(!sys.abortTransition("ship_2"), "Cannot abort from Landed");
}

static void testSPTLandingTarget() {
    std::cout << "\n=== SPTSystem: Autopilot and Landing Target ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);

    world.createEntity("ship_1");
    sys.initializeTransition("ship_1", "planet_a", true);

    assertTrue(sys.setAutopilot("ship_1", true), "Autopilot enabled");
    assertTrue(sys.setAutopilot("ship_1", false), "Autopilot disabled");
    assertTrue(sys.setLandingTarget("ship_1", 150.5f, -42.3f), "Landing target set");
    assertTrue(sys.setLandingTarget("ship_1", 0.0f, 0.0f), "Landing target reset to origin");
}

static void testSPTMissingEntity() {
    std::cout << "\n=== SPTSystem: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::SpacePlanetTransitionSystem sys(&world);

    assertTrue(!sys.initializeTransition("nonexistent", "p", true), "Init fails on missing");
    assertTrue(!sys.beginDescent("nonexistent"), "Descent fails on missing");
    assertTrue(!sys.beginLaunch("nonexistent"), "Launch fails on missing");
    assertTrue(!sys.abortTransition("nonexistent"), "Abort fails on missing");
    assertTrue(!sys.setAutopilot("nonexistent", true), "Autopilot fails on missing");
    assertTrue(!sys.setLandingTarget("nonexistent", 0.0f, 0.0f), "LandingTarget fails on missing");
    assertTrue(approxEqual(sys.getAltitude("nonexistent"), 0.0f), "Altitude 0 on missing");
    assertTrue(approxEqual(sys.getHeatLevel("nonexistent"), 0.0f), "Heat 0 on missing");
    assertTrue(sys.getTransitionState("nonexistent").empty() ||
               sys.getTransitionState("nonexistent") == "unknown", "Empty/unknown state on missing");
}


void run_space_planet_transition_system_tests() {
    testSPTInitialize();
    testSPTBeginDescent();
    testSPTDescentPhaseProgression();
    testSPTBeginLaunch();
    testSPTAbortTransition();
    testSPTAbortFromInvalidStates();
    testSPTLandingTarget();
    testSPTMissingEntity();
}
