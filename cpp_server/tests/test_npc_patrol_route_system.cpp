// Tests for: NPC Patrol Route System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/npc_patrol_route_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== NPC Patrol Route System Tests ====================

static void testNPCPatrolRouteCreate() {
    std::cout << "\n=== NPCPatrolRoute: Create ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    assertTrue(sys.initialize("patrol1"), "Init succeeds");
    assertTrue(sys.getWaypointCount("patrol1") == 0, "No waypoints initially");
    assertTrue(sys.getState("patrol1") == "Idle", "Initial state is Idle");
    assertTrue(sys.getTotalPatrolsCompleted("patrol1") == 0, "No patrols completed");
    assertTrue(sys.getTotalAlertsTriggered("patrol1") == 0, "No alerts triggered");
    assertTrue(sys.getWaypointsVisited("patrol1") == 0, "No waypoints visited");
}

static void testNPCPatrolRouteAddWaypoints() {
    std::cout << "\n=== NPCPatrolRoute: AddWaypoints ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");
    assertTrue(sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 5.0f, 1000.0f), "Add wp1");
    assertTrue(sys.addWaypoint("patrol1", "wp2", 200.0f, 0.0f, 0.0f, 3.0f, 1500.0f), "Add wp2");
    assertTrue(sys.addWaypoint("patrol1", "wp3", 300.0f, 0.0f, 0.0f, 2.0f, 800.0f), "Add wp3");
    assertTrue(sys.getWaypointCount("patrol1") == 3, "3 waypoints added");
}

static void testNPCPatrolRouteDuplicate() {
    std::cout << "\n=== NPCPatrolRoute: Duplicate ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");
    sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 5.0f, 1000.0f);
    assertTrue(!sys.addWaypoint("patrol1", "wp1", 200.0f, 0.0f, 0.0f, 3.0f, 500.0f), "Duplicate rejected");
    assertTrue(sys.getWaypointCount("patrol1") == 1, "Still 1 waypoint");
}

static void testNPCPatrolRouteRemove() {
    std::cout << "\n=== NPCPatrolRoute: Remove ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");
    sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 5.0f, 1000.0f);
    sys.addWaypoint("patrol1", "wp2", 200.0f, 0.0f, 0.0f, 3.0f, 1500.0f);
    assertTrue(sys.removeWaypoint("patrol1", "wp1"), "Remove wp1 succeeds");
    assertTrue(sys.getWaypointCount("patrol1") == 1, "1 waypoint after remove");
    assertTrue(!sys.removeWaypoint("patrol1", "wp1"), "Double remove fails");
}

static void testNPCPatrolRouteStartStop() {
    std::cout << "\n=== NPCPatrolRoute: StartStop ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");
    assertTrue(!sys.startPatrol("patrol1"), "Can't start with no waypoints");
    sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 5.0f, 1000.0f);
    assertTrue(sys.startPatrol("patrol1"), "Start patrol succeeds");
    assertTrue(sys.getState("patrol1") == "Travelling", "State is Travelling");
    assertTrue(sys.stopPatrol("patrol1"), "Stop patrol succeeds");
    assertTrue(sys.getState("patrol1") == "Idle", "State is Idle after stop");
}

static void testNPCPatrolRouteAlert() {
    std::cout << "\n=== NPCPatrolRoute: Alert ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");
    sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 5.0f, 1000.0f);
    sys.startPatrol("patrol1");
    assertTrue(sys.triggerAlert("patrol1", "pirate1"), "Alert triggered");
    assertTrue(sys.getState("patrol1") == "Alert", "State is Alert");
    assertTrue(sys.isHostileDetected("patrol1"), "Hostile detected");
    assertTrue(sys.getTotalAlertsTriggered("patrol1") == 1, "1 alert triggered");
    assertTrue(sys.clearAlert("patrol1"), "Clear alert succeeds");
    assertTrue(sys.getState("patrol1") == "Travelling", "Back to Travelling after clear");
    assertTrue(!sys.isHostileDetected("patrol1"), "No hostile after clear");
}

static void testNPCPatrolRouteWaiting() {
    std::cout << "\n=== NPCPatrolRoute: Waiting ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");
    sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 10.0f, 1000.0f);
    sys.addWaypoint("patrol1", "wp2", 200.0f, 0.0f, 0.0f, 5.0f, 1000.0f);

    auto* entity = world.getEntity("patrol1");
    auto* comp = entity->getComponent<components::NPCPatrolRoute>();
    comp->travel_speed = 1000.0f; // fast travel

    sys.startPatrol("patrol1");
    // Travel to first waypoint (distance 500 default, speed 1000 = 0.5s)
    sys.update(1.0f);
    assertTrue(sys.getState("patrol1") == "Waiting", "Waiting at first waypoint");
    assertTrue(sys.getWaypointsVisited("patrol1") == 1, "1 waypoint visited");
}

static void testNPCPatrolRouteLoop() {
    std::cout << "\n=== NPCPatrolRoute: Loop ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");
    sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 0.0f, 1000.0f); // no idle

    auto* entity = world.getEntity("patrol1");
    auto* comp = entity->getComponent<components::NPCPatrolRoute>();
    comp->travel_speed = 1000.0f;
    comp->loop = true;

    sys.startPatrol("patrol1");
    sys.update(1.0f); // arrives at wp1, no idle, loops
    assertTrue(sys.getTotalPatrolsCompleted("patrol1") >= 1, "At least 1 patrol completed via loop");
    assertTrue(sys.getState("patrol1") == "Travelling", "Still travelling after loop");
}

static void testNPCPatrolRouteNoLoop() {
    std::cout << "\n=== NPCPatrolRoute: NoLoop ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");
    sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 0.0f, 1000.0f);

    auto* entity = world.getEntity("patrol1");
    auto* comp = entity->getComponent<components::NPCPatrolRoute>();
    comp->travel_speed = 1000.0f;
    comp->loop = false;

    sys.startPatrol("patrol1");
    sys.update(1.0f);
    assertTrue(sys.getState("patrol1") == "Complete", "State is Complete without loop");
    assertTrue(sys.getTotalPatrolsCompleted("patrol1") == 1, "1 patrol completed");
}

static void testNPCPatrolRouteMaxLimit() {
    std::cout << "\n=== NPCPatrolRoute: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    world.createEntity("patrol1");
    sys.initialize("patrol1");

    auto* entity = world.getEntity("patrol1");
    auto* comp = entity->getComponent<components::NPCPatrolRoute>();
    comp->max_waypoints = 2;

    sys.addWaypoint("patrol1", "wp1", 100.0f, 0.0f, 0.0f, 5.0f, 1000.0f);
    sys.addWaypoint("patrol1", "wp2", 200.0f, 0.0f, 0.0f, 3.0f, 1500.0f);
    assertTrue(!sys.addWaypoint("patrol1", "wp3", 300.0f, 0.0f, 0.0f, 2.0f, 800.0f), "Max limit enforced");
    assertTrue(sys.getWaypointCount("patrol1") == 2, "Still 2 waypoints");
}

static void testNPCPatrolRouteMissing() {
    std::cout << "\n=== NPCPatrolRoute: Missing ===" << std::endl;
    ecs::World world;
    systems::NPCPatrolRouteSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addWaypoint("nonexistent", "wp1", 0.0f, 0.0f, 0.0f, 1.0f, 100.0f), "Add fails on missing");
    assertTrue(!sys.removeWaypoint("nonexistent", "wp1"), "Remove fails on missing");
    assertTrue(!sys.startPatrol("nonexistent"), "Start fails on missing");
    assertTrue(!sys.stopPatrol("nonexistent"), "Stop fails on missing");
    assertTrue(!sys.triggerAlert("nonexistent", "h1"), "Alert fails on missing");
    assertTrue(!sys.clearAlert("nonexistent"), "ClearAlert fails on missing");
    assertTrue(sys.getWaypointCount("nonexistent") == 0, "0 waypoints on missing");
    assertTrue(sys.getCurrentWaypointIndex("nonexistent") == -1, "-1 index on missing");
    assertTrue(sys.getState("nonexistent") == "Unknown", "Unknown state on missing");
    assertTrue(!sys.isHostileDetected("nonexistent"), "No hostile on missing");
    assertTrue(sys.getTotalPatrolsCompleted("nonexistent") == 0, "0 patrols on missing");
    assertTrue(sys.getTotalAlertsTriggered("nonexistent") == 0, "0 alerts on missing");
    assertTrue(sys.getWaypointsVisited("nonexistent") == 0, "0 visited on missing");
}


void run_npc_patrol_route_system_tests() {
    testNPCPatrolRouteCreate();
    testNPCPatrolRouteAddWaypoints();
    testNPCPatrolRouteDuplicate();
    testNPCPatrolRouteRemove();
    testNPCPatrolRouteStartStop();
    testNPCPatrolRouteAlert();
    testNPCPatrolRouteWaiting();
    testNPCPatrolRouteLoop();
    testNPCPatrolRouteNoLoop();
    testNPCPatrolRouteMaxLimit();
    testNPCPatrolRouteMissing();
}
