// Tests for: Beacon Navigation System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/beacon_navigation_system.h"

using namespace atlas;

// ==================== Beacon Navigation System Tests ====================

static void testBeaconNavigationCreate() {
    std::cout << "\n=== BeaconNavigation: Create ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    world.createEntity("beacon1");
    assertTrue(sys.initialize("beacon1", "nav_001", "player1", "sol"), "Init succeeds");
    assertTrue(sys.getState("beacon1") == "Online", "Online initially");
    assertTrue(sys.getLabel("beacon1") == "", "No label initially");
    assertTrue(sys.getType("beacon1") == "Waypoint", "Default Waypoint type");
    assertTrue(approxEqual(sys.getSignalStrength("beacon1"), 1.0f), "Full signal");
    assertTrue(approxEqual(sys.getScanRange("beacon1"), 1000.0f), "Default scan range");
    assertTrue(sys.isPublic("beacon1"), "Public by default");
    assertTrue(sys.getTotalWarpsTo("beacon1") == 0, "No warps");
    assertTrue(sys.getTotalScans("beacon1") == 0, "No scans");
}

static void testBeaconNavigationPosition() {
    std::cout << "\n=== BeaconNavigation: Position ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    world.createEntity("beacon1");
    sys.initialize("beacon1", "nav_001", "player1", "sol");

    assertTrue(sys.setPosition("beacon1", 100.0, 200.0, 300.0), "Set position");
    assertTrue(sys.setLabel("beacon1", "Mining Belt Alpha"), "Set label");
    assertTrue(sys.getLabel("beacon1") == "Mining Belt Alpha", "Label matches");
}

static void testBeaconNavigationType() {
    std::cout << "\n=== BeaconNavigation: Type ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    world.createEntity("beacon1");
    sys.initialize("beacon1", "nav_001", "player1", "sol");

    assertTrue(sys.setType("beacon1", "FleetWarp"), "Set FleetWarp type");
    assertTrue(sys.getType("beacon1") == "FleetWarp", "Type is FleetWarp");

    assertTrue(sys.setType("beacon1", "Emergency"), "Set Emergency type");
    assertTrue(sys.getType("beacon1") == "Emergency", "Type is Emergency");

    assertTrue(sys.setType("beacon1", "Survey"), "Set Survey type");
    assertTrue(sys.getType("beacon1") == "Survey", "Type is Survey");
}

static void testBeaconNavigationDegradation() {
    std::cout << "\n=== BeaconNavigation: Degradation ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    world.createEntity("beacon1");
    sys.initialize("beacon1", "nav_001", "player1", "sol");

    // Degrade signal (default rate 0.0001/s)
    // After 2500s: 1.0 - (0.0001 * 2500) = 0.75 -> still Online
    sys.update(2500.0f);
    assertTrue(sys.getSignalStrength("beacon1") < 1.0f, "Signal degraded");
    assertTrue(sys.getState("beacon1") == "Online", "Still Online above 0.5");

    // After 5000 more seconds: 0.75 - 0.5 = 0.25 -> Degraded
    sys.update(5000.0f);
    assertTrue(sys.getState("beacon1") == "Degraded", "Degraded below 0.5");

    // Continue until offline
    sys.update(5000.0f); // should be ~0 or below
    assertTrue(sys.getState("beacon1") == "Offline", "Offline at 0 signal");
    assertTrue(sys.getSignalStrength("beacon1") == 0.0f, "Signal clamped to 0");
}

static void testBeaconNavigationWarpAndScan() {
    std::cout << "\n=== BeaconNavigation: WarpAndScan ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    world.createEntity("beacon1");
    sys.initialize("beacon1", "nav_001", "player1", "sol");

    assertTrue(sys.recordWarpTo("beacon1"), "Record warp");
    assertTrue(sys.recordWarpTo("beacon1"), "Record second warp");
    assertTrue(sys.getTotalWarpsTo("beacon1") == 2, "2 warps recorded");

    assertTrue(sys.recordScan("beacon1"), "Record scan");
    assertTrue(sys.getTotalScans("beacon1") == 1, "1 scan recorded");
}

static void testBeaconNavigationCannotWarpToOffline() {
    std::cout << "\n=== BeaconNavigation: CannotWarpToOffline ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    world.createEntity("beacon1");
    sys.initialize("beacon1", "nav_001", "player1", "sol");

    // Degrade to offline
    sys.update(15000.0f);
    assertTrue(sys.getState("beacon1") == "Offline", "Beacon is offline");
    assertTrue(!sys.recordWarpTo("beacon1"), "Cannot warp to offline beacon");
}

static void testBeaconNavigationRepair() {
    std::cout << "\n=== BeaconNavigation: Repair ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    world.createEntity("beacon1");
    sys.initialize("beacon1", "nav_001", "player1", "sol");

    // Degrade to offline
    sys.update(15000.0f);
    assertTrue(sys.getState("beacon1") == "Offline", "Offline before repair");

    assertTrue(sys.repair("beacon1"), "Repair succeeds");
    assertTrue(sys.getState("beacon1") == "Online", "Online after repair");
    assertTrue(approxEqual(sys.getSignalStrength("beacon1"), 1.0f), "Full signal after repair");
}

static void testBeaconNavigationPublicPrivate() {
    std::cout << "\n=== BeaconNavigation: PublicPrivate ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    world.createEntity("beacon1");
    sys.initialize("beacon1", "nav_001", "player1", "sol");

    assertTrue(sys.isPublic("beacon1"), "Public by default");
    assertTrue(sys.setPublic("beacon1", false), "Set private");
    assertTrue(!sys.isPublic("beacon1"), "Now private");
}

static void testBeaconNavigationMissing() {
    std::cout << "\n=== BeaconNavigation: Missing ===" << std::endl;
    ecs::World world;
    systems::BeaconNavigationSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "b", "o", "s"), "Init fails on missing");
    assertTrue(!sys.setPosition("nonexistent", 0.0, 0.0, 0.0), "SetPosition fails");
    assertTrue(!sys.setLabel("nonexistent", "test"), "SetLabel fails");
    assertTrue(!sys.setType("nonexistent", "Survey"), "SetType fails");
    assertTrue(!sys.setPublic("nonexistent", false), "SetPublic fails");
    assertTrue(!sys.recordWarpTo("nonexistent"), "RecordWarp fails");
    assertTrue(!sys.recordScan("nonexistent"), "RecordScan fails");
    assertTrue(!sys.repair("nonexistent"), "Repair fails");
    assertTrue(sys.getState("nonexistent") == "Unknown", "Unknown state");
    assertTrue(sys.getLabel("nonexistent") == "", "Empty label");
    assertTrue(sys.getType("nonexistent") == "Unknown", "Unknown type");
    assertTrue(sys.getSignalStrength("nonexistent") == 0.0f, "0 signal");
    assertTrue(sys.getScanRange("nonexistent") == 0.0f, "0 range");
    assertTrue(!sys.isPublic("nonexistent"), "Not public on missing");
    assertTrue(sys.getTotalWarpsTo("nonexistent") == 0, "0 warps");
    assertTrue(sys.getTotalScans("nonexistent") == 0, "0 scans");
}


void run_beacon_navigation_system_tests() {
    testBeaconNavigationCreate();
    testBeaconNavigationPosition();
    testBeaconNavigationType();
    testBeaconNavigationDegradation();
    testBeaconNavigationWarpAndScan();
    testBeaconNavigationCannotWarpToOffline();
    testBeaconNavigationRepair();
    testBeaconNavigationPublicPrivate();
    testBeaconNavigationMissing();
}
