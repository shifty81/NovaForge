// Tests for: EVA Airlock System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "components/navigation_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/eva_airlock_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== EVA Airlock System Tests ====================

static void testEVAAirlockCreate() {
    std::cout << "\n=== EVA Airlock Create ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);

    assertTrue(sys.createAirlock("airlock1", "ship1", 1.0f), "Airlock created");
    assertTrue(!sys.createAirlock("airlock1", "ship1"), "Duplicate fails");
    assertTrue(sys.getPhase("airlock1") ==
               static_cast<int>(components::EVAAirlockState::Phase::Idle),
               "Starts idle");
}

static void testEVAAirlockBeginEVA() {
    std::cout << "\n=== EVA Airlock Begin EVA ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 0.5f);

    // Insufficient oxygen
    assertTrue(!sys.beginEVA("airlock1", "player1", 5.0f),
               "EVA denied with low oxygen");

    // Sufficient oxygen
    assertTrue(sys.beginEVA("airlock1", "player1", 50.0f),
               "EVA started with enough oxygen");
    assertTrue(sys.getPhase("airlock1") ==
               static_cast<int>(components::EVAAirlockState::Phase::EnterChamber),
               "Phase is EnterChamber");
}

static void testEVAAirlockFullSequence() {
    std::cout << "\n=== EVA Airlock Full EVA Sequence ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 0.5f);

    sys.beginEVA("airlock1", "player1", 100.0f);

    // Run through all phases: EnterChamber → InnerSeal → Depressurize → OuterOpen → EVAActive
    for (int i = 0; i < 50; ++i) sys.update(0.1f);

    assertTrue(sys.isInEVA("airlock1"), "Player is in EVA");
    assertTrue(approxEqual(sys.getChamberPressure("airlock1"), 0.0f),
               "Chamber depressurized");
}

static void testEVAAirlockReentry() {
    std::cout << "\n=== EVA Airlock Reentry ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 0.5f);

    sys.beginEVA("airlock1", "player1", 100.0f);
    for (int i = 0; i < 50; ++i) sys.update(0.1f);  // Reach EVA

    assertTrue(sys.beginReentry("airlock1", "player1"), "Reentry started");

    // Run through re-entry: OuterSeal → Repressurize → InnerOpen → Complete
    for (int i = 0; i < 50; ++i) sys.update(0.1f);

    assertTrue(sys.getPhase("airlock1") ==
               static_cast<int>(components::EVAAirlockState::Phase::Complete),
               "Reentry complete");
    assertTrue(approxEqual(sys.getChamberPressure("airlock1"), 1.0f),
               "Chamber repressurized");
}

static void testEVAAirlockAbort() {
    std::cout << "\n=== EVA Airlock Abort ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 1.0f);

    sys.beginEVA("airlock1", "player1", 100.0f);
    sys.update(0.5f);  // Partial progress in EnterChamber

    assertTrue(sys.abortSequence("airlock1"), "Abort succeeds during pre-EVA");

    sys.update(0.01f);  // Process abort
    assertTrue(sys.getPhase("airlock1") ==
               static_cast<int>(components::EVAAirlockState::Phase::Idle),
               "Reverted to Idle after abort");
}

static void testEVAAirlockReentryWrongPlayer() {
    std::cout << "\n=== EVA Airlock Reentry Wrong Player ===" << std::endl;
    ecs::World world;
    systems::EVAAirlockSystem sys(&world);
    sys.createAirlock("airlock1", "ship1", 0.5f);

    sys.beginEVA("airlock1", "player1", 100.0f);
    for (int i = 0; i < 50; ++i) sys.update(0.1f);

    assertTrue(!sys.beginReentry("airlock1", "player2"),
               "Wrong player can't reenter");
}

static void testEVAAirlockPhaseNames() {
    std::cout << "\n=== EVA Airlock Phase Names ===" << std::endl;
    assertTrue(systems::EVAAirlockSystem::phaseName(0) == "Idle", "Idle name");
    assertTrue(systems::EVAAirlockSystem::phaseName(1) == "EnterChamber", "EnterChamber name");
    assertTrue(systems::EVAAirlockSystem::phaseName(3) == "Depressurize", "Depressurize name");
    assertTrue(systems::EVAAirlockSystem::phaseName(5) == "EVAActive", "EVAActive name");
    assertTrue(systems::EVAAirlockSystem::phaseName(9) == "Complete", "Complete name");
}

static void testEVAAirlockComponentDefaults() {
    std::cout << "\n=== EVA Airlock Component Defaults ===" << std::endl;
    components::EVAAirlockState state;
    assertTrue(state.phase == 0, "Default phase Idle");
    assertTrue(approxEqual(state.chamber_pressure, 1.0f), "Default chamber pressure 1.0");
    assertTrue(!state.inner_door_open, "Default inner door closed");
    assertTrue(!state.outer_door_open, "Default outer door closed");
    assertTrue(approxEqual(state.min_suit_oxygen, 10.0f), "Default min suit oxygen 10");
    assertTrue(!state.abort_requested, "Default no abort");
}


void run_eva_airlock_system_tests() {
    testEVAAirlockCreate();
    testEVAAirlockBeginEVA();
    testEVAAirlockFullSequence();
    testEVAAirlockReentry();
    testEVAAirlockAbort();
    testEVAAirlockReentryWrongPlayer();
    testEVAAirlockPhaseNames();
    testEVAAirlockComponentDefaults();
}
