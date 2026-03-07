// Tests for: Docking Request System
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/docking_request_system.h"

using namespace atlas;

// ==================== Docking Request System Tests ====================

static void testDockingRequestCreate() {
    std::cout << "\n=== DockingRequest: Create ===" << std::endl;
    ecs::World world;
    systems::DockingRequestSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", "station_alpha"), "Init succeeds");
    assertTrue(sys.getPhase("ship1") == "idle", "Phase is idle");
    assertTrue(sys.getTetherProgress("ship1") == 0.0f, "Tether 0");
    assertTrue(sys.getTotalDockings("ship1") == 0, "0 dockings");
    assertTrue(sys.getDeniedCount("ship1") == 0, "0 denied");
}

static void testDockingRequestApproach() {
    std::cout << "\n=== DockingRequest: Approach ===" << std::endl;
    ecs::World world;
    systems::DockingRequestSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "station_alpha");

    assertTrue(sys.beginApproach("ship1", 5000.0f), "Begin approach");
    assertTrue(sys.getPhase("ship1") == "approach", "Phase is approach");

    // Too far to request
    assertTrue(!sys.requestDocking("ship1"), "Request denied — out of range");

    // Can't approach again while already approaching
    assertTrue(!sys.beginApproach("ship1", 1000.0f), "Double approach rejected");
}

static void testDockingRequestFullDocking() {
    std::cout << "\n=== DockingRequest: FullDocking ===" << std::endl;
    ecs::World world;
    systems::DockingRequestSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "station_alpha");

    sys.beginApproach("ship1", 2000.0f);  // within docking range (2500m)
    assertTrue(sys.requestDocking("ship1"), "Request accepted — in range");
    assertTrue(sys.getPhase("ship1") == "requested", "Phase is requested");

    assertTrue(sys.grantDocking("ship1"), "Grant docking");
    assertTrue(sys.getPhase("ship1") == "granted", "Phase is granted");

    // Tether fills over time (speed = 0.5/s → 2s to fill)
    for (int i = 0; i < 20; i++) sys.update(0.1f);
    assertTrue(sys.getPhase("ship1") == "docked", "Phase is docked");
    assertTrue(sys.getTotalDockings("ship1") == 1, "1 docking");
    assertTrue(approxEqual(sys.getTetherProgress("ship1"), 1.0f), "Tether full");
}

static void testDockingRequestDeny() {
    std::cout << "\n=== DockingRequest: Deny ===" << std::endl;
    ecs::World world;
    systems::DockingRequestSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "station_alpha");

    sys.beginApproach("ship1", 1500.0f);
    sys.requestDocking("ship1");
    assertTrue(sys.denyDocking("ship1"), "Deny docking");
    assertTrue(sys.getPhase("ship1") == "idle", "Back to idle");
    assertTrue(sys.getDeniedCount("ship1") == 1, "1 denied");
}

static void testDockingRequestUndock() {
    std::cout << "\n=== DockingRequest: Undock ===" << std::endl;
    ecs::World world;
    systems::DockingRequestSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "station_alpha");

    // Dock first
    sys.beginApproach("ship1", 1000.0f);
    sys.requestDocking("ship1");
    sys.grantDocking("ship1");
    for (int i = 0; i < 25; i++) sys.update(0.1f);
    assertTrue(sys.getPhase("ship1") == "docked", "Docked");

    assertTrue(sys.undock("ship1"), "Undock succeeds");
    assertTrue(sys.getPhase("ship1") == "idle", "Back to idle");
    assertTrue(approxEqual(sys.getTetherProgress("ship1"), 0.0f), "Tether reset");
}

static void testDockingRequestMissing() {
    std::cout << "\n=== DockingRequest: Missing ===" << std::endl;
    ecs::World world;
    systems::DockingRequestSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "station"), "Init fails on missing");
    assertTrue(!sys.beginApproach("nonexistent", 1000.0f), "Approach fails on missing");
    assertTrue(!sys.requestDocking("nonexistent"), "Request fails on missing");
    assertTrue(!sys.grantDocking("nonexistent"), "Grant fails on missing");
    assertTrue(!sys.denyDocking("nonexistent"), "Deny fails on missing");
    assertTrue(!sys.undock("nonexistent"), "Undock fails on missing");
    assertTrue(sys.getPhase("nonexistent") == "unknown", "Unknown phase on missing");
    assertTrue(sys.getTetherProgress("nonexistent") == 0.0f, "0 tether on missing");
    assertTrue(sys.getTotalDockings("nonexistent") == 0, "0 dockings on missing");
    assertTrue(sys.getDeniedCount("nonexistent") == 0, "0 denied on missing");
}

void run_docking_request_system_tests() {
    testDockingRequestCreate();
    testDockingRequestApproach();
    testDockingRequestFullDocking();
    testDockingRequestDeny();
    testDockingRequestUndock();
    testDockingRequestMissing();
}
