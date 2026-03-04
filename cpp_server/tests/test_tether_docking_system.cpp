// Tests for: Tether Docking System Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/docking_system.h"
#include "systems/tether_docking_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Tether Docking System Tests ====================

static void testTetherArmCreate() {
    std::cout << "\n=== Tether Arm Create ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);

    assertTrue(sys.createArm("arm1", "station1"), "Arm created");
    assertTrue(!sys.createArm("arm1", "station1"), "Duplicate arm fails");
    assertTrue(!sys.isOccupied("arm1"), "Arm empty");
}

static void testTetherBeginTether() {
    std::cout << "\n=== Tether Begin ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");

    assertTrue(sys.beginTether("arm1", "capital_ship"), "Tether started");
    assertTrue(sys.isOccupied("arm1"), "Arm occupied");
    assertTrue(sys.getTetheredShip("arm1") == "capital_ship", "Correct ship tethered");
    assertTrue(sys.getArmState("arm1") ==
               components::TetherDockingArm::ArmState::Extending, "Arm extending");
}

static void testTetherExtendToLock() {
    std::cout << "\n=== Tether Extend to Lock ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "capital_ship");

    // Run enough time for the arm to fully extend (default 0.5/s → 2 seconds).
    sys.update(3.0f);

    assertTrue(sys.getArmState("arm1") ==
               components::TetherDockingArm::ArmState::Locked, "Arm locked after extending");
    assertTrue(sys.isCrewTransferEnabled("arm1"), "Crew transfer enabled when locked");
    assertTrue(approxEqual(sys.getExtendProgress("arm1"), 1.0f), "Fully extended");
}

static void testTetherUndockRetract() {
    std::cout << "\n=== Tether Undock Retract ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "capital_ship");
    sys.update(3.0f); // Lock

    assertTrue(sys.beginUndock("arm1"), "Undock started");
    assertTrue(!sys.isCrewTransferEnabled("arm1"), "Crew transfer disabled during retract");
    assertTrue(sys.getArmState("arm1") ==
               components::TetherDockingArm::ArmState::Retracting, "Arm retracting");

    sys.update(3.0f); // Retract

    assertTrue(sys.getArmState("arm1") ==
               components::TetherDockingArm::ArmState::Retracted, "Arm retracted");
    assertTrue(!sys.isOccupied("arm1"), "Arm empty after retraction");
    assertTrue(sys.getTetheredShip("arm1").empty(), "No ship tethered");
}

static void testTetherDoubleOccupy() {
    std::cout << "\n=== Tether Double Occupy ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "ship_a");

    assertTrue(!sys.beginTether("arm1", "ship_b"), "Can't tether second ship");
}

static void testTetherUndockOnlyWhenLocked() {
    std::cout << "\n=== Tether Undock Only When Locked ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "capital_ship");

    // Still extending, not locked yet.
    sys.update(0.5f);
    assertTrue(!sys.beginUndock("arm1"), "Can't undock while extending");
}

static void testTetherPartialExtend() {
    std::cout << "\n=== Tether Partial Extend ===" << std::endl;
    ecs::World world;
    systems::TetherDockingSystem sys(&world);
    sys.createArm("arm1", "station1");
    sys.beginTether("arm1", "capital_ship");

    sys.update(1.0f); // 0.5 progress/s * 1.0s = 0.5 progress
    float progress = sys.getExtendProgress("arm1");
    assertTrue(progress > 0.4f && progress < 0.6f, "Partially extended at ~0.5");
    assertTrue(!sys.isCrewTransferEnabled("arm1"), "No crew transfer while extending");
}

static void testTetherComponentDefaults() {
    std::cout << "\n=== Tether Component Defaults ===" << std::endl;
    components::TetherDockingArm arm;
    assertTrue(arm.state == components::TetherDockingArm::ArmState::Retracted, "Default retracted");
    assertTrue(!arm.isOccupied(), "Default not occupied");
    assertTrue(arm.isFullyRetracted(), "Default fully retracted");
    assertTrue(!arm.isFullyExtended(), "Default not extended");
    assertTrue(arm.station_shield_active, "Shield active by default");
    assertTrue(!arm.crew_transfer_enabled, "Crew transfer disabled by default");
}


void run_tether_docking_system_tests() {
    testTetherArmCreate();
    testTetherBeginTether();
    testTetherExtendToLock();
    testTetherUndockRetract();
    testTetherDoubleOccupy();
    testTetherUndockOnlyWhenLocked();
    testTetherPartialExtend();
    testTetherComponentDefaults();
}
