// Tests for: EVAAirlockExitSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/eva_airlock_exit_system.h"

using namespace atlas;

// ==================== EVAAirlockExitSystem Tests ====================

static void testEVACreateExitPoint() {
    std::cout << "\n=== EVAAirlockExit: CreateExitPoint ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");

    assertTrue(sys.createExitPoint("airlock1", "ship_1", 300.0f), "Create exit point");
    assertTrue(sys.getState("airlock1") == 0, "Initial state is Inactive");
    assertTrue(!sys.isInSpace("airlock1"), "Not in space initially");
    assertTrue(!sys.isExitBlocked("airlock1"), "Not blocked initially");
    assertTrue(sys.getDistanceFromShip("airlock1") == 0.0f, "Zero distance initially");
}

static void testEVACreateInvalid() {
    std::cout << "\n=== EVAAirlockExit: CreateInvalid ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);

    assertTrue(!sys.createExitPoint("missing", "ship"), "Missing entity fails");

    world.createEntity("airlock1");
    assertTrue(sys.createExitPoint("airlock1", "ship"), "First create succeeds");
    assertTrue(!sys.createExitPoint("airlock1", "ship2"), "Double create fails");
}

static void testEVARequestExit() {
    std::cout << "\n=== EVAAirlockExit: RequestExit ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");
    sys.createExitPoint("airlock1", "ship_1");

    assertTrue(sys.requestExit("airlock1", "player_1", 80.0f), "Request exit");
    assertTrue(sys.getState("airlock1") == 1, "State is RequestingExit");

    // Cannot request again while active
    assertTrue(!sys.requestExit("airlock1", "player_2", 90.0f), "Double request fails");
}

static void testEVADockBlocking() {
    std::cout << "\n=== EVAAirlockExit: DockBlocking ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");
    sys.createExitPoint("airlock1", "ship_1");

    // Set ship as docked
    sys.setDockState("airlock1", true);
    sys.requestExit("airlock1", "player_1", 80.0f);

    // Progress through RequestingExit to CheckingDockState
    sys.update(3.0f); // Advances past RequestingExit
    // Once in CheckingDockState, next update checks dock
    sys.update(0.1f);
    assertTrue(sys.isExitBlocked("airlock1"), "Exit blocked when docked");
    assertTrue(sys.getState("airlock1") == 0, "Returned to Inactive");
}

static void testEVAOxygenBlocking() {
    std::cout << "\n=== EVAAirlockExit: OxygenBlocking ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");
    sys.createExitPoint("airlock1", "ship_1");

    // Low oxygen (below min_oxygen default of 10.0)
    sys.setDockState("airlock1", false);
    sys.requestExit("airlock1", "player_1", 5.0f);

    // Progress to CheckingDockState
    sys.update(3.0f);
    sys.update(0.1f);
    assertTrue(sys.isExitBlocked("airlock1"), "Exit blocked on low oxygen");
    assertTrue(sys.getState("airlock1") == 0, "Returned to Inactive");
}

static void testEVASuccessfulExit() {
    std::cout << "\n=== EVAAirlockExit: SuccessfulExit ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");
    sys.createExitPoint("airlock1", "ship_1");
    sys.setDockState("airlock1", false);
    sys.requestExit("airlock1", "player_1", 80.0f);

    // Progress through all states to InSpace
    // RequestingExit(1) → CheckingDockState(2) → PreparingExit(3) → Exiting(4) → InSpace(5)
    // Each state takes state_duration (2.0s default)
    sys.update(2.5f); // Past RequestingExit
    sys.update(2.5f); // Past CheckingDockState
    sys.update(2.5f); // Past PreparingExit
    sys.update(2.5f); // Past Exiting → InSpace

    assertTrue(sys.getState("airlock1") == 5, "State is InSpace");
    assertTrue(sys.isInSpace("airlock1"), "isInSpace true");
    assertTrue(sys.getDistanceFromShip("airlock1") == 0.0f, "Distance reset on enter space");

    // InSpace does not auto-advance
    sys.update(10.0f);
    assertTrue(sys.getState("airlock1") == 5, "Still InSpace after update");
}

static void testEVATetherRange() {
    std::cout << "\n=== EVAAirlockExit: TetherRange ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");
    sys.createExitPoint("airlock1", "ship_1", 100.0f);
    sys.setDockState("airlock1", false);
    sys.requestExit("airlock1", "player_1", 80.0f);

    // Get to InSpace
    sys.update(2.5f);
    sys.update(2.5f);
    sys.update(2.5f);
    sys.update(2.5f);
    assertTrue(sys.isInSpace("airlock1"), "In space");

    // Move within tether
    assertTrue(sys.moveAway("airlock1", 50.0f), "Move 50m");
    assertTrue(approxEqual(sys.getDistanceFromShip("airlock1"), 50.0f), "Distance is 50m");

    // Move beyond tether — clamped
    assertTrue(sys.moveAway("airlock1", 80.0f), "Move 80m more (clamped)");
    assertTrue(approxEqual(sys.getDistanceFromShip("airlock1"), 100.0f), "Distance clamped to 100m");
    assertTrue(sys.isTetherActive("airlock1"), "Tether is active");
}

static void testEVABeginReturn() {
    std::cout << "\n=== EVAAirlockExit: BeginReturn ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");
    sys.createExitPoint("airlock1", "ship_1");
    sys.setDockState("airlock1", false);
    sys.requestExit("airlock1", "player_1", 80.0f);

    // Get to InSpace
    sys.update(2.5f);
    sys.update(2.5f);
    sys.update(2.5f);
    sys.update(2.5f);

    assertTrue(sys.beginReturn("airlock1"), "Begin return from InSpace");
    assertTrue(sys.getState("airlock1") == 6, "State is Returning");

    // Cannot begin return again
    assertTrue(!sys.beginReturn("airlock1"), "Double return fails");

    // Returning auto-advances to Complete
    sys.update(2.5f);
    assertTrue(sys.getState("airlock1") == 7, "State is Complete");
}

static void testEVACancelExit() {
    std::cout << "\n=== EVAAirlockExit: CancelExit ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");
    sys.createExitPoint("airlock1", "ship_1");
    sys.requestExit("airlock1", "player_1", 80.0f);

    assertTrue(sys.cancelExit("airlock1"), "Cancel active exit");
    assertTrue(sys.getState("airlock1") == 0, "State reset to Inactive");
    assertTrue(!sys.isExitBlocked("airlock1"), "Not blocked after cancel");
    assertTrue(sys.getDistanceFromShip("airlock1") == 0.0f, "Distance reset after cancel");

    // Cannot cancel when already inactive
    assertTrue(!sys.cancelExit("airlock1"), "Cancel inactive fails");
}

static void testEVAMoveNotInSpace() {
    std::cout << "\n=== EVAAirlockExit: MoveNotInSpace ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock1");
    sys.createExitPoint("airlock1", "ship_1");

    assertTrue(!sys.moveAway("airlock1", 10.0f), "Cannot move when not in space");

    sys.requestExit("airlock1", "player_1", 80.0f);
    assertTrue(!sys.moveAway("airlock1", 10.0f), "Cannot move during RequestingExit");
}

static void testEVAStateNames() {
    std::cout << "\n=== EVAAirlockExit: StateNames ===" << std::endl;
    assertTrue(systems::EVAAirlockExitSystem::stateName(0) == "inactive", "State 0 name");
    assertTrue(systems::EVAAirlockExitSystem::stateName(1) == "requesting_exit", "State 1 name");
    assertTrue(systems::EVAAirlockExitSystem::stateName(2) == "checking_dock_state", "State 2 name");
    assertTrue(systems::EVAAirlockExitSystem::stateName(3) == "preparing_exit", "State 3 name");
    assertTrue(systems::EVAAirlockExitSystem::stateName(4) == "exiting", "State 4 name");
    assertTrue(systems::EVAAirlockExitSystem::stateName(5) == "in_space", "State 5 name");
    assertTrue(systems::EVAAirlockExitSystem::stateName(6) == "returning", "State 6 name");
    assertTrue(systems::EVAAirlockExitSystem::stateName(7) == "complete", "State 7 name");
}

static void testEVAMissing() {
    std::cout << "\n=== EVAAirlockExit: Missing ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);

    assertTrue(sys.getState("x") == 0, "Default state on missing");
    assertTrue(sys.getStateProgress("x") == 0.0f, "Default progress on missing");
    assertTrue(!sys.isExitBlocked("x"), "Default blocked on missing");
    assertTrue(!sys.isInSpace("x"), "Default in-space on missing");
    assertTrue(sys.getDistanceFromShip("x") == 0.0f, "Default distance on missing");
    assertTrue(!sys.isTetherActive("x"), "Default tether on missing");
    assertTrue(!sys.requestExit("x", "p", 80.0f), "Request on missing fails");
    assertTrue(!sys.setDockState("x", true), "SetDock on missing fails");
    assertTrue(!sys.beginReturn("x"), "Return on missing fails");
    assertTrue(!sys.cancelExit("x"), "Cancel on missing fails");
    assertTrue(!sys.moveAway("x", 10.0f), "Move on missing fails");
}

void run_eva_airlock_exit_system_tests() {
    testEVACreateExitPoint();
    testEVACreateInvalid();
    testEVARequestExit();
    testEVADockBlocking();
    testEVAOxygenBlocking();
    testEVASuccessfulExit();
    testEVATetherRange();
    testEVABeginReturn();
    testEVACancelExit();
    testEVAMoveNotInSpace();
    testEVAStateNames();
    testEVAMissing();
}
