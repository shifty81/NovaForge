// Tests for: HyperspaceRoutingSystem
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/hyperspace_routing_system.h"

using namespace atlas;

// ==================== HyperspaceRoutingSystem Tests ====================

static void testHyperspaceRoutingCreate() {
    std::cout << "\n=== HyperspaceRouting: Create ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(sys.getWaypointCount("ship1") == 0, "No waypoints initially");
    assertTrue(!sys.isRouteActive("ship1"), "No active route");
    assertTrue(sys.getTotalRoutesCalculated("ship1") == 0, "Zero routes calculated");
    assertTrue(sys.getTotalJumpsCompleted("ship1") == 0, "Zero jumps completed");
}

static void testHyperspaceRoutingInvalidInit() {
    std::cout << "\n=== HyperspaceRouting: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails for missing entity");
}

static void testHyperspaceRoutingGateConnections() {
    std::cout << "\n=== HyperspaceRouting: GateConnections ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);

    assertTrue(sys.addGateConnection("sys_a", "sys_b", "gate_ab", 10.0f), "Add gate A→B");
    assertTrue(sys.addGateConnection("sys_b", "sys_c", "gate_bc", 15.0f), "Add gate B→C");
    assertTrue(sys.getConnectionCount() == 2, "2 connections");

    // Duplicate rejected
    assertTrue(!sys.addGateConnection("sys_a", "sys_b", "gate_ab2", 5.0f), "Duplicate rejected");

    // Invalid connections rejected
    assertTrue(!sys.addGateConnection("", "sys_b", "gate", 5.0f), "Empty from rejected");
    assertTrue(!sys.addGateConnection("sys_a", "", "gate", 5.0f), "Empty to rejected");
    assertTrue(!sys.addGateConnection("sys_a", "sys_d", "", 5.0f), "Empty gate rejected");
    assertTrue(!sys.addGateConnection("sys_a", "sys_d", "gate", 0.0f), "Zero time rejected");
    assertTrue(!sys.addGateConnection("sys_a", "sys_d", "gate", -5.0f), "Negative time rejected");
    assertTrue(!sys.addGateConnection("sys_a", "sys_a", "gate", 5.0f), "Self-loop rejected");
}

static void testHyperspaceRoutingRemoveConnection() {
    std::cout << "\n=== HyperspaceRouting: RemoveConnection ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);

    sys.addGateConnection("sys_a", "sys_b", "gate_ab", 10.0f);
    sys.addGateConnection("sys_b", "sys_c", "gate_bc", 15.0f);

    assertTrue(sys.removeGateConnection("sys_a", "sys_b"), "Remove A→B succeeds");
    assertTrue(sys.getConnectionCount() == 1, "1 connection left");

    assertTrue(!sys.removeGateConnection("sys_a", "sys_b"), "Double remove fails");
    assertTrue(!sys.removeGateConnection("sys_x", "sys_y"), "Remove nonexistent fails");
}

static void testHyperspaceRoutingCalculateRoute() {
    std::cout << "\n=== HyperspaceRouting: CalculateRoute ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addGateConnection("sys_a", "sys_b", "gate_ab", 10.0f);
    sys.addGateConnection("sys_b", "sys_c", "gate_bc", 15.0f);
    sys.addGateConnection("sys_c", "sys_d", "gate_cd", 8.0f);

    assertTrue(sys.calculateRoute("ship1", "sys_a", "sys_d"), "Route A→D calculated");
    assertTrue(sys.getWaypointCount("ship1") == 3, "3 waypoints (B, C, D)");
    assertTrue(sys.isRouteActive("ship1"), "Route is active");
    assertTrue(sys.getTotalRoutesCalculated("ship1") == 1, "1 route calculated");
    assertTrue(sys.getDestination("ship1") == "sys_d", "Destination is sys_d");

    float est = sys.getTotalEstimatedTime("ship1");
    assertTrue(est > 32.9f && est < 33.1f, "Total estimated time ~33s");
}

static void testHyperspaceRoutingNoPath() {
    std::cout << "\n=== HyperspaceRouting: NoPath ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addGateConnection("sys_a", "sys_b", "gate_ab", 10.0f);
    // No connection from B to C

    assertTrue(!sys.calculateRoute("ship1", "sys_a", "sys_c"), "No path returns false");
}

static void testHyperspaceRoutingInvalidCalc() {
    std::cout << "\n=== HyperspaceRouting: InvalidCalc ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    assertTrue(!sys.calculateRoute("ship1", "", "sys_b"), "Empty origin rejected");
    assertTrue(!sys.calculateRoute("ship1", "sys_a", ""), "Empty destination rejected");
    assertTrue(!sys.calculateRoute("ship1", "sys_a", "sys_a"), "Same origin/dest rejected");
    assertTrue(!sys.calculateRoute("nonexistent", "sys_a", "sys_b"), "Missing entity rejected");
}

static void testHyperspaceRoutingAdvanceWaypoint() {
    std::cout << "\n=== HyperspaceRouting: AdvanceWaypoint ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addGateConnection("sys_a", "sys_b", "gate_ab", 10.0f);
    sys.addGateConnection("sys_b", "sys_c", "gate_bc", 15.0f);
    sys.calculateRoute("ship1", "sys_a", "sys_c");

    assertTrue(sys.getCurrentWaypointIndex("ship1") == 0, "Start at waypoint 0");
    assertTrue(sys.getRemainingJumps("ship1") == 1, "1 remaining jump");

    assertTrue(sys.advanceWaypoint("ship1"), "Advance waypoint 1");
    assertTrue(sys.getTotalJumpsCompleted("ship1") == 1, "1 jump completed");

    assertTrue(sys.advanceWaypoint("ship1"), "Advance waypoint 2");
    assertTrue(sys.getTotalJumpsCompleted("ship1") == 2, "2 jumps completed");
    assertTrue(sys.isRouteComplete("ship1"), "Route complete");
    assertTrue(!sys.isRouteActive("ship1"), "Route no longer active");

    assertTrue(!sys.advanceWaypoint("ship1"), "Can't advance past end");
}

static void testHyperspaceRoutingClearRoute() {
    std::cout << "\n=== HyperspaceRouting: ClearRoute ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addGateConnection("sys_a", "sys_b", "gate_ab", 10.0f);
    sys.calculateRoute("ship1", "sys_a", "sys_b");

    assertTrue(sys.clearRoute("ship1"), "Clear route succeeds");
    assertTrue(sys.getWaypointCount("ship1") == 0, "No waypoints after clear");
    assertTrue(!sys.isRouteActive("ship1"), "Route inactive after clear");
    assertTrue(sys.getDestination("ship1").empty(), "No destination after clear");
}

static void testHyperspaceRoutingUpdate() {
    std::cout << "\n=== HyperspaceRouting: Update ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    sys.addGateConnection("sys_a", "sys_b", "gate_ab", 10.0f);
    sys.calculateRoute("ship1", "sys_a", "sys_b");

    sys.update(5.0f);
    float elapsed = sys.getElapsedTravelTime("ship1");
    assertTrue(elapsed > 4.9f && elapsed < 5.1f, "Elapsed ~5s after update");
}

static void testHyperspaceRoutingShortestPath() {
    std::cout << "\n=== HyperspaceRouting: ShortestPath ===" << std::endl;
    ecs::World world;
    systems::HyperspaceRoutingSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    // Direct path: A→D via two jumps
    sys.addGateConnection("sys_a", "sys_b", "gate_ab", 10.0f);
    sys.addGateConnection("sys_b", "sys_d", "gate_bd", 10.0f);
    // Longer path: A→C→D via two jumps but A→B→D is also 2 jumps
    sys.addGateConnection("sys_a", "sys_c", "gate_ac", 100.0f);
    sys.addGateConnection("sys_c", "sys_d", "gate_cd", 100.0f);

    sys.calculateRoute("ship1", "sys_a", "sys_d");
    // BFS finds shortest by jumps (both are 2 jumps), should find one of them
    assertTrue(sys.getWaypointCount("ship1") == 2, "2 waypoints for 2-jump route");
}

void run_hyperspace_routing_system_tests() {
    testHyperspaceRoutingCreate();
    testHyperspaceRoutingInvalidInit();
    testHyperspaceRoutingGateConnections();
    testHyperspaceRoutingRemoveConnection();
    testHyperspaceRoutingCalculateRoute();
    testHyperspaceRoutingNoPath();
    testHyperspaceRoutingInvalidCalc();
    testHyperspaceRoutingAdvanceWaypoint();
    testHyperspaceRoutingClearRoute();
    testHyperspaceRoutingUpdate();
    testHyperspaceRoutingShortestPath();
}
