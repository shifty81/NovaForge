// Tests for: EVAAirlockExit Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/eva_airlock_exit_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== EVAAirlockExit Tests ====================

static void testEVAExitInit() {
    std::cout << "\n=== EVAAirlockExit: Init ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    assertTrue(sys.createExitPoint("airlock_1", "ship_1", 200.0f), "Exit point created");
    assertTrue(sys.getState("airlock_1") == 0, "State is inactive");
    assertTrue(!sys.isInSpace("airlock_1"), "Not in space");
    assertTrue(!sys.createExitPoint("airlock_1", "ship_1"), "Duplicate creation fails");
}

static void testEVAExitRequest() {
    std::cout << "\n=== EVAAirlockExit: Request Exit ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    assertTrue(sys.requestExit("airlock_1", "player_1", 100.0f), "Exit requested");
    assertTrue(sys.getState("airlock_1") == 1, "State is requesting_exit");
    assertTrue(!sys.requestExit("airlock_1", "player_2", 100.0f), "Can't request twice");
}

static void testEVAExitDockedBlocked() {
    std::cout << "\n=== EVAAirlockExit: Docked Blocked ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.setDockState("airlock_1", true);
    sys.requestExit("airlock_1", "player_1", 100.0f);
    // Advance through RequestingExit to CheckingDockState
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.isExitBlocked("airlock_1"), "Exit blocked while docked");
    assertTrue(sys.getState("airlock_1") == 0, "Returned to inactive");
}

static void testEVAExitLowOxygen() {
    std::cout << "\n=== EVAAirlockExit: Low Oxygen ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.requestExit("airlock_1", "player_1", 5.0f); // Below min 10.0
    // Advance through RequestingExit to CheckingDockState
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.isExitBlocked("airlock_1"), "Exit blocked with low oxygen");
}

static void testEVAExitFullCycle() {
    std::cout << "\n=== EVAAirlockExit: Full Cycle ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.setDockState("airlock_1", false);
    sys.requestExit("airlock_1", "player_1", 100.0f);
    // Advance through all phases to InSpace (states 1-5, 2s each = 10s)
    for (int i = 0; i < 15; i++) sys.update(1.0f);
    assertTrue(sys.isInSpace("airlock_1"), "Player is in space");
}

static void testEVAExitTether() {
    std::cout << "\n=== EVAAirlockExit: Tether ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1", 100.0f);
    sys.setDockState("airlock_1", false);
    sys.requestExit("airlock_1", "player_1", 100.0f);
    for (int i = 0; i < 15; i++) sys.update(1.0f);
    assertTrue(sys.isInSpace("airlock_1"), "In space");
    sys.moveAway("airlock_1", 50.0f);
    assertTrue(approxEqual(sys.getDistanceFromShip("airlock_1"), 50.0f), "50m away");
    sys.moveAway("airlock_1", 200.0f);
    assertTrue(approxEqual(sys.getDistanceFromShip("airlock_1"), 100.0f), "Clamped to tether range");
    assertTrue(sys.isTetherActive("airlock_1"), "Tether is active");
}

static void testEVAExitReturn() {
    std::cout << "\n=== EVAAirlockExit: Return ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.setDockState("airlock_1", false);
    sys.requestExit("airlock_1", "player_1", 100.0f);
    for (int i = 0; i < 15; i++) sys.update(1.0f);
    assertTrue(sys.beginReturn("airlock_1"), "Return started");
    // Advance through Returning to Complete
    for (int i = 0; i < 5; i++) sys.update(1.0f);
    assertTrue(sys.getState("airlock_1") == 7, "Reached complete state");
}

static void testEVAExitCancel() {
    std::cout << "\n=== EVAAirlockExit: Cancel ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    world.createEntity("airlock_1");
    sys.createExitPoint("airlock_1", "ship_1");
    sys.requestExit("airlock_1", "player_1", 100.0f);
    assertTrue(sys.cancelExit("airlock_1"), "Exit cancelled");
    assertTrue(sys.getState("airlock_1") == 0, "State reset to inactive");
    assertTrue(!sys.cancelExit("airlock_1"), "Can't cancel inactive");
}

static void testEVAExitStateNames() {
    std::cout << "\n=== EVAAirlockExit: State Names ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    assertTrue(sys.stateName(0) == "inactive", "State 0 name");
    assertTrue(sys.stateName(1) == "requesting_exit", "State 1 name");
    assertTrue(sys.stateName(5) == "in_space", "State 5 name");
    assertTrue(sys.stateName(7) == "complete", "State 7 name");
}

static void testEVAExitMissing() {
    std::cout << "\n=== EVAAirlockExit: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockExitSystem sys(&world);
    assertTrue(!sys.createExitPoint("nonexistent", "ship"), "Create fails on missing");
    assertTrue(!sys.requestExit("nonexistent", "p", 100.0f), "Request fails on missing");
    assertTrue(sys.getState("nonexistent") == 0, "State 0 on missing");
    assertTrue(!sys.isInSpace("nonexistent"), "Not in space on missing");
}


void run_evaairlock_exit_tests() {
    testEVAExitInit();
    testEVAExitRequest();
    testEVAExitDockedBlocked();
    testEVAExitLowOxygen();
    testEVAExitFullCycle();
    testEVAExitTether();
    testEVAExitReturn();
    testEVAExitCancel();
    testEVAExitStateNames();
    testEVAExitMissing();
}
