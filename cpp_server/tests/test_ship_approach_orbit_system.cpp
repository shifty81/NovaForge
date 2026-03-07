// Tests for: Ship Approach/Orbit System
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/ship_approach_orbit_system.h"

using namespace atlas;

// ==================== Ship Approach/Orbit System Tests ====================

static void testApproachOrbitCreate() {
    std::cout << "\n=== ApproachOrbit: Create ===" << std::endl;
    ecs::World world;
    systems::ShipApproachOrbitSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", 100.0f), "Init succeeds");
    assertTrue(sys.getCommandType("ship1") == "none", "No command");
    assertTrue(!sys.isCommandActive("ship1"), "Not active");
    assertTrue(approxEqual(sys.getCurrentSpeed("ship1"), 0.0f), "0 speed");
    assertTrue(approxEqual(sys.getOrbitAngle("ship1"), 0.0f), "0 orbit angle");
}

static void testApproachCommand() {
    std::cout << "\n=== ApproachOrbit: Approach ===" << std::endl;
    ecs::World world;
    systems::ShipApproachOrbitSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 200.0f);

    assertTrue(sys.commandApproach("ship1", "station_01", 1000.0f), "Approach station");
    assertTrue(sys.getCommandType("ship1") == "approach", "Command is approach");
    assertTrue(sys.getTargetId("ship1") == "station_01", "Target is station_01");
    assertTrue(sys.isCommandActive("ship1"), "Command active");
    assertTrue(approxEqual(sys.getCurrentDistance("ship1"), 1000.0f), "1000m distance");

    // Tick 2 seconds at max speed 200m/s => closes 400m
    sys.update(2.0f);
    assertTrue(approxEqual(sys.getCurrentDistance("ship1"), 600.0f), "600m after 2s at 200m/s");
    assertTrue(sys.getCurrentSpeed("ship1") > 0.0f, "Speed > 0 while approaching");

    // Tick 3 more seconds => closes 600m, now at 0
    sys.update(3.0f);
    assertTrue(approxEqual(sys.getCurrentDistance("ship1"), 0.0f), "0m at target");
    assertTrue(approxEqual(sys.getCurrentSpeed("ship1"), 0.0f), "0 speed at target");
}

static void testOrbitCommand() {
    std::cout << "\n=== ApproachOrbit: Orbit ===" << std::endl;
    ecs::World world;
    systems::ShipApproachOrbitSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f);

    assertTrue(sys.commandOrbit("ship1", "asteroid_01", 500.0f), "Orbit at 500m");
    assertTrue(sys.getCommandType("ship1") == "orbit", "Command is orbit");
    assertTrue(approxEqual(sys.getDesiredDistance("ship1"), 500.0f), "Desired 500m");

    // Invalid orbit radius
    assertTrue(!sys.commandOrbit("ship1", "target", 0.0f), "0 radius rejected");
    assertTrue(!sys.commandOrbit("ship1", "target", -100.0f), "Negative radius rejected");

    // Tick and check orbit angle advances
    sys.update(1.0f);
    assertTrue(sys.getOrbitAngle("ship1") > 0.0f, "Orbit angle increased");
    assertTrue(sys.getCurrentSpeed("ship1") > 0.0f, "Speed > 0 while orbiting");
}

static void testKeepAtRangeCommand() {
    std::cout << "\n=== ApproachOrbit: KeepAtRange ===" << std::endl;
    ecs::World world;
    systems::ShipApproachOrbitSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f);

    // Set current distance via approach first, then switch to keep-at-range
    sys.commandApproach("ship1", "target_01", 1000.0f);
    auto* comp = world.getEntity("ship1")->getComponent<components::ApproachOrbitState>();

    assertTrue(sys.commandKeepAtRange("ship1", "target_01", 300.0f), "Keep at 300m");
    assertTrue(sys.getCommandType("ship1") == "keep_at_range", "Command is keep_at_range");
    assertTrue(approxEqual(sys.getDesiredDistance("ship1"), 300.0f), "Desired 300m");

    // Negative range rejected
    assertTrue(!sys.commandKeepAtRange("ship1", "target", -50.0f), "Negative range rejected");
}

static void testStopCommand() {
    std::cout << "\n=== ApproachOrbit: Stop ===" << std::endl;
    ecs::World world;
    systems::ShipApproachOrbitSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f);

    sys.commandApproach("ship1", "station_01", 500.0f);
    assertTrue(sys.isCommandActive("ship1"), "Active before stop");

    assertTrue(sys.stopCommand("ship1"), "Stop succeeds");
    assertTrue(sys.getCommandType("ship1") == "none", "No command after stop");
    assertTrue(!sys.isCommandActive("ship1"), "Not active after stop");

    // Can't stop when already stopped
    assertTrue(!sys.stopCommand("ship1"), "Stop when idle returns false");
}

static void testCommandOverride() {
    std::cout << "\n=== ApproachOrbit: Override ===" << std::endl;
    ecs::World world;
    systems::ShipApproachOrbitSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f);

    sys.commandApproach("ship1", "station_01", 500.0f);
    assertTrue(sys.getCommandType("ship1") == "approach", "Initially approach");

    sys.commandOrbit("ship1", "asteroid_01", 300.0f);
    assertTrue(sys.getCommandType("ship1") == "orbit", "Overridden to orbit");
    assertTrue(sys.getTargetId("ship1") == "asteroid_01", "New target");
}

static void testSetMaxSpeed() {
    std::cout << "\n=== ApproachOrbit: SetMaxSpeed ===" << std::endl;
    ecs::World world;
    systems::ShipApproachOrbitSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", 100.0f);

    assertTrue(sys.setMaxSpeed("ship1", 250.0f), "Set max speed");

    sys.commandApproach("ship1", "s1", 500.0f);
    sys.update(1.0f);
    // At 250 m/s for 1s, should close 250m
    assertTrue(approxEqual(sys.getCurrentDistance("ship1"), 250.0f), "250m after 1s at 250m/s");
}

static void testApproachOrbitMissing() {
    std::cout << "\n=== ApproachOrbit: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipApproachOrbitSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 100.0f), "Init fails on missing");
    assertTrue(!sys.commandApproach("nonexistent", "t", 100.0f), "Approach fails on missing");
    assertTrue(!sys.commandOrbit("nonexistent", "t", 100.0f), "Orbit fails on missing");
    assertTrue(!sys.commandKeepAtRange("nonexistent", "t", 100.0f), "KeepAtRange fails on missing");
    assertTrue(!sys.stopCommand("nonexistent"), "Stop fails on missing");
    assertTrue(sys.getCommandType("nonexistent") == "none", "No cmd on missing");
    assertTrue(sys.getTargetId("nonexistent") == "", "No target on missing");
    assertTrue(!sys.isCommandActive("nonexistent"), "Not active on missing");
}

void run_ship_approach_orbit_system_tests() {
    testApproachOrbitCreate();
    testApproachCommand();
    testOrbitCommand();
    testKeepAtRangeCommand();
    testStopCommand();
    testCommandOverride();
    testSetMaxSpeed();
    testApproachOrbitMissing();
}
