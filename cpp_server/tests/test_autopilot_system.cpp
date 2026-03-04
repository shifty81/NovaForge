// Tests for: Autopilot System Tests
#include "test_log.h"
#include "components/navigation_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/autopilot_system.h"

using namespace atlas;

// ==================== Autopilot System Tests ====================

static void testAutopilotCreate() {
    std::cout << "\n=== Autopilot: Create ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initializeAutopilot("ship1", "player1"), "Init autopilot succeeds");
    assertTrue(sys.getWaypointCount("ship1") == 0, "No waypoints initially");
    assertTrue(!sys.isEngaged("ship1"), "Not engaged initially");
    assertTrue(sys.getWaypointsReached("ship1") == 0, "No waypoints reached");
}

static void testAutopilotAddWaypoint() {
    std::cout << "\n=== Autopilot: AddWaypoint ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    assertTrue(sys.addWaypoint("ship1", "wp1", "Station Alpha", 1000.0f, 0.0f, 0.0f), "Add waypoint succeeds");
    assertTrue(sys.getWaypointCount("ship1") == 1, "1 waypoint");
    assertTrue(!sys.addWaypoint("ship1", "wp1", "Dup", 0.0f, 0.0f, 0.0f), "Duplicate rejected");
}

static void testAutopilotEngage() {
    std::cout << "\n=== Autopilot: Engage ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    assertTrue(!sys.engage("ship1"), "Engage fails with no waypoints");
    sys.addWaypoint("ship1", "wp1", "Gate A", 1000.0f, 0.0f, 0.0f);
    assertTrue(sys.engage("ship1"), "Engage succeeds with waypoints");
    assertTrue(sys.isEngaged("ship1"), "Autopilot engaged");
}

static void testAutopilotDisengage() {
    std::cout << "\n=== Autopilot: Disengage ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 1000.0f, 0.0f, 0.0f);
    sys.engage("ship1");
    assertTrue(sys.disengage("ship1"), "Disengage succeeds");
    assertTrue(!sys.isEngaged("ship1"), "No longer engaged");
}

static void testAutopilotNavigation() {
    std::cout << "\n=== Autopilot: Navigation ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 1000.0f, 0.0f, 0.0f);
    sys.engage("ship1");
    // speed=100 m/s, distance=1000m, travel for 10s → 1000m traveled
    // Arrival distance = 50m, so after 10s (1000m traveled), distance_to_next = 1000-1000 = 0 → arrived
    sys.update(10.0f);
    assertTrue(sys.getWaypointsReached("ship1") == 1, "Waypoint reached");
    assertTrue(sys.getCurrentWaypointIndex("ship1") == 1, "Index advanced to 1");
}

static void testAutopilotRouteComplete() {
    std::cout << "\n=== Autopilot: RouteComplete ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 500.0f, 0.0f, 0.0f);
    sys.engage("ship1");
    sys.update(5.0f); // 100 m/s * 5s = 500m, arrives at wp1
    assertTrue(sys.isRouteComplete("ship1"), "Route complete after all waypoints");
}

static void testAutopilotLoop() {
    std::cout << "\n=== Autopilot: Loop ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 500.0f, 0.0f, 0.0f);
    assertTrue(sys.setLoop("ship1", true), "Set loop succeeds");
    sys.engage("ship1");
    sys.update(5.0f); // reach wp1
    assertTrue(!sys.isRouteComplete("ship1"), "Route not complete in loop mode");
    assertTrue(sys.getCurrentWaypointIndex("ship1") == 0, "Index reset to 0 for loop");
}

static void testAutopilotRemoveWaypoint() {
    std::cout << "\n=== Autopilot: RemoveWaypoint ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    sys.addWaypoint("ship1", "wp1", "Gate A", 1000.0f, 0.0f, 0.0f);
    sys.addWaypoint("ship1", "wp2", "Gate B", 2000.0f, 0.0f, 0.0f);
    assertTrue(sys.removeWaypoint("ship1", "wp1"), "Remove succeeds");
    assertTrue(sys.getWaypointCount("ship1") == 1, "1 waypoint remaining");
    assertTrue(!sys.removeWaypoint("ship1", "wp1"), "Remove nonexistent fails");
}

static void testAutopilotMaxWaypoints() {
    std::cout << "\n=== Autopilot: MaxWaypoints ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    world.createEntity("ship1");
    sys.initializeAutopilot("ship1", "player1");
    auto* entity = world.getEntity("ship1");
    auto* ap = entity->getComponent<components::Autopilot>();
    ap->max_waypoints = 3;
    sys.addWaypoint("ship1", "w1", "A", 100.0f, 0.0f, 0.0f);
    sys.addWaypoint("ship1", "w2", "B", 200.0f, 0.0f, 0.0f);
    sys.addWaypoint("ship1", "w3", "C", 300.0f, 0.0f, 0.0f);
    assertTrue(!sys.addWaypoint("ship1", "w4", "D", 400.0f, 0.0f, 0.0f), "Max waypoints enforced");
    assertTrue(sys.getWaypointCount("ship1") == 3, "Still 3 waypoints");
}

static void testAutopilotMissing() {
    std::cout << "\n=== Autopilot: Missing ===" << std::endl;
    ecs::World world;
    systems::AutopilotSystem sys(&world);
    assertTrue(!sys.initializeAutopilot("nonexistent", "p1"), "Init fails on missing");
    assertTrue(!sys.addWaypoint("nonexistent", "w1", "A", 0, 0, 0), "Add fails on missing");
    assertTrue(!sys.removeWaypoint("nonexistent", "w1"), "Remove fails on missing");
    assertTrue(!sys.engage("nonexistent"), "Engage fails on missing");
    assertTrue(!sys.disengage("nonexistent"), "Disengage fails on missing");
    assertTrue(!sys.setLoop("nonexistent", true), "Set loop fails on missing");
    assertTrue(!sys.setSpeed("nonexistent", 200.0f), "Set speed fails on missing");
    assertTrue(sys.getWaypointCount("nonexistent") == 0, "0 waypoints on missing");
    assertTrue(sys.getCurrentWaypointIndex("nonexistent") == 0, "0 index on missing");
    assertTrue(sys.getWaypointsReached("nonexistent") == 0, "0 reached on missing");
    assertTrue(approxEqual(sys.getTotalDistanceTraveled("nonexistent"), 0.0f), "0 distance on missing");
    assertTrue(!sys.isEngaged("nonexistent"), "Not engaged on missing");
    assertTrue(!sys.isRouteComplete("nonexistent"), "Not complete on missing");
}


void run_autopilot_system_tests() {
    testAutopilotCreate();
    testAutopilotAddWaypoint();
    testAutopilotEngage();
    testAutopilotDisengage();
    testAutopilotNavigation();
    testAutopilotRouteComplete();
    testAutopilotLoop();
    testAutopilotRemoveWaypoint();
    testAutopilotMaxWaypoints();
    testAutopilotMissing();
}
