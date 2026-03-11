// Tests for: NpcPatrolRouteSystem
#include "test_log.h"
#include "components/navigation_components.h"
#include "ecs/system.h"
#include "systems/npc_patrol_route_system.h"

using namespace atlas;

// ==================== NpcPatrolRouteSystem Tests ====================

static void testNpcPatrolRouteInit() {
    std::cout << "\n=== NpcPatrolRoute: Init ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getWaypointCount("e1") == 0, "Zero waypoints initially");
    assertTrue(sys.getStatus("e1") == components::NpcPatrolRoute::Status::Idle, "Status is Idle initially");
    assertTrue(!sys.isPatrolling("e1"), "Not patrolling initially");
    assertTrue(sys.getCurrentWaypointIndex("e1") == 0, "Current index is 0 initially");
    assertTrue(sys.getCurrentWaypointId("e1") == "", "No current waypoint id initially");
    assertTrue(sys.getTotalCircuits("e1") == 0, "Zero circuits initially");
    assertTrue(sys.getTotalWaypointsVisited("e1") == 0, "Zero waypoints visited initially");
    assertTrue(approxEqual(sys.getSpeed("e1"), 1.0f), "Default speed is 1.0");
    assertTrue(approxEqual(sys.getDwellTimer("e1"), 0.0f), "Dwell timer is 0 initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testNpcPatrolRouteAddWaypoint() {
    std::cout << "\n=== NpcPatrolRoute: AddWaypoint ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.addWaypoint("e1", "wp1", 10.0f, 0.0f, 0.0f), "Add first waypoint");
    assertTrue(sys.addWaypoint("e1", "wp2", 20.0f, 5.0f, 0.0f, 2.0f), "Add second waypoint with dwell");
    assertTrue(sys.getWaypointCount("e1") == 2, "Two waypoints stored");
    assertTrue(!sys.addWaypoint("e1", "wp1", 99.0f, 0.0f, 0.0f), "Duplicate id rejected");
    assertTrue(!sys.addWaypoint("e1", "", 0.0f, 0.0f, 0.0f), "Empty id rejected");
    assertTrue(!sys.addWaypoint("e1", "wp_neg", 0.0f, 0.0f, 0.0f, -1.0f), "Negative dwell rejected");
    assertTrue(sys.getWaypointCount("e1") == 2, "Count unchanged after rejections");
    assertTrue(!sys.addWaypoint("nonexistent", "wp1", 0.0f, 0.0f, 0.0f), "Add fails on missing entity");
}

static void testNpcPatrolRouteMaxWaypoints() {
    std::cout << "\n=== NpcPatrolRoute: MaxWaypoints ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    for (int i = 0; i < 20; i++) {
        std::string id = "wp" + std::to_string(i);
        assertTrue(sys.addWaypoint("e1", id, static_cast<float>(i), 0.0f, 0.0f),
                   "Waypoint added within limit");
    }
    assertTrue(sys.getWaypointCount("e1") == 20, "Count is 20 at max");
    assertTrue(!sys.addWaypoint("e1", "wp_over", 0.0f, 0.0f, 0.0f), "Blocked at max");
    assertTrue(sys.getWaypointCount("e1") == 20, "Count unchanged after block");
}

static void testNpcPatrolRouteRemoveWaypoint() {
    std::cout << "\n=== NpcPatrolRoute: RemoveWaypoint ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addWaypoint("e1", "wp1", 10.0f, 0.0f, 0.0f);
    sys.addWaypoint("e1", "wp2", 20.0f, 0.0f, 0.0f);
    sys.addWaypoint("e1", "wp3", 30.0f, 0.0f, 0.0f);

    assertTrue(sys.removeWaypoint("e1", "wp2"), "Remove existing waypoint succeeds");
    assertTrue(sys.getWaypointCount("e1") == 2, "2 waypoints remain");
    assertTrue(!sys.removeWaypoint("e1", "wp2"), "Remove nonexistent fails");
    assertTrue(!sys.removeWaypoint("e1", "wpX"), "Remove unknown id fails");
    assertTrue(!sys.removeWaypoint("nonexistent", "wp1"), "Remove fails on missing entity");
    assertTrue(sys.getWaypointCount("e1") == 2, "Count still 2");
}

static void testNpcPatrolRouteClearWaypoints() {
    std::cout << "\n=== NpcPatrolRoute: ClearWaypoints ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addWaypoint("e1", "wp1", 10.0f, 0.0f, 0.0f);
    sys.addWaypoint("e1", "wp2", 20.0f, 0.0f, 0.0f);
    sys.startPatrol("e1");
    assertTrue(sys.isPatrolling("e1"), "Patrolling before clear");

    assertTrue(sys.clearWaypoints("e1"), "Clear waypoints succeeds");
    assertTrue(sys.getWaypointCount("e1") == 0, "Zero waypoints after clear");
    assertTrue(sys.getStatus("e1") == components::NpcPatrolRoute::Status::Idle, "Status Idle after clear");
    assertTrue(!sys.isPatrolling("e1"), "Not patrolling after clear");
    assertTrue(!sys.clearWaypoints("nonexistent"), "Clear fails on missing entity");
}

static void testNpcPatrolRouteStartStop() {
    std::cout << "\n=== NpcPatrolRoute: StartStop ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.startPatrol("e1"), "Start fails with no waypoints");
    sys.addWaypoint("e1", "wp1", 10.0f, 0.0f, 0.0f);
    sys.addWaypoint("e1", "wp2", 20.0f, 0.0f, 0.0f);

    assertTrue(sys.startPatrol("e1"), "Start patrol succeeds");
    assertTrue(sys.getStatus("e1") == components::NpcPatrolRoute::Status::Traveling, "Status is Traveling");
    assertTrue(sys.isPatrolling("e1"), "isPatrolling returns true");
    assertTrue(!sys.startPatrol("e1"), "Cannot start while already patrolling");

    assertTrue(sys.stopPatrol("e1"), "Stop patrol succeeds");
    assertTrue(sys.getStatus("e1") == components::NpcPatrolRoute::Status::Idle, "Status is Idle after stop");
    assertTrue(!sys.isPatrolling("e1"), "Not patrolling after stop");
    assertTrue(!sys.stopPatrol("e1"), "Cannot stop when already Idle");

    // Re-start after stop
    assertTrue(sys.startPatrol("e1"), "Re-start after stop succeeds");
    assertTrue(sys.isPatrolling("e1"), "Patrolling again after re-start");

    assertTrue(!sys.startPatrol("nonexistent"), "Start fails on missing entity");
    assertTrue(!sys.stopPatrol("nonexistent"), "Stop fails on missing entity");
}

static void testNpcPatrolRouteAdvanceLoop() {
    std::cout << "\n=== NpcPatrolRoute: AdvanceLoop ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addWaypoint("e1", "wp0", 0.0f, 0.0f, 0.0f);
    sys.addWaypoint("e1", "wp1", 10.0f, 0.0f, 0.0f);
    sys.addWaypoint("e1", "wp2", 20.0f, 0.0f, 0.0f);
    sys.setPatrolMode("e1", components::NpcPatrolRoute::PatrolMode::Loop);
    sys.startPatrol("e1");

    assertTrue(sys.getCurrentWaypointIndex("e1") == 0, "Starts at index 0");
    assertTrue(sys.getCurrentWaypointId("e1") == "wp0", "Current id is wp0");
    assertTrue(sys.getTotalWaypointsVisited("e1") == 1, "1 waypoint visited on start");

    assertTrue(sys.advanceWaypoint("e1"), "Advance to index 1");
    assertTrue(sys.getCurrentWaypointIndex("e1") == 1, "Now at index 1");
    assertTrue(sys.getCurrentWaypointId("e1") == "wp1", "Current id is wp1");
    assertTrue(sys.getTotalCircuits("e1") == 0, "No circuits yet");

    assertTrue(sys.advanceWaypoint("e1"), "Advance to index 2");
    assertTrue(sys.getCurrentWaypointIndex("e1") == 2, "Now at index 2");
    assertTrue(sys.getTotalCircuits("e1") == 0, "Still no circuit");

    assertTrue(sys.advanceWaypoint("e1"), "Advance wraps to index 0");
    assertTrue(sys.getCurrentWaypointIndex("e1") == 0, "Wrapped back to index 0");
    assertTrue(sys.getTotalCircuits("e1") == 1, "1 circuit completed on wrap");

    assertTrue(!sys.advanceWaypoint("nonexistent"), "Advance fails on missing entity");
}

static void testNpcPatrolRoutePingPong() {
    std::cout << "\n=== NpcPatrolRoute: PingPong ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addWaypoint("e1", "wp0", 0.0f, 0.0f, 0.0f);
    sys.addWaypoint("e1", "wp1", 10.0f, 0.0f, 0.0f);
    sys.addWaypoint("e1", "wp2", 20.0f, 0.0f, 0.0f);
    sys.setPatrolMode("e1", components::NpcPatrolRoute::PatrolMode::PingPong);
    assertTrue(sys.getPatrolMode("e1") == components::NpcPatrolRoute::PatrolMode::PingPong, "Mode is PingPong");
    sys.startPatrol("e1");

    assertTrue(sys.getCurrentWaypointIndex("e1") == 0, "Starts at 0");

    sys.advanceWaypoint("e1");
    assertTrue(sys.getCurrentWaypointIndex("e1") == 1, "Forward to 1");

    sys.advanceWaypoint("e1");
    assertTrue(sys.getCurrentWaypointIndex("e1") == 2, "Forward to 2");

    sys.advanceWaypoint("e1"); // hits end, reverses
    assertTrue(sys.getCurrentWaypointIndex("e1") == 1, "Reversed to 1");
    assertTrue(sys.getTotalCircuits("e1") == 1, "1 circuit on reversal at end");

    sys.advanceWaypoint("e1");
    assertTrue(sys.getCurrentWaypointIndex("e1") == 0, "Backward to 0");

    sys.advanceWaypoint("e1"); // hits start, reverses
    assertTrue(sys.getCurrentWaypointIndex("e1") == 1, "Reversed forward to 1");
    assertTrue(sys.getTotalCircuits("e1") == 2, "2 circuits after second reversal");
}

static void testNpcPatrolRouteDwell() {
    std::cout << "\n=== NpcPatrolRoute: Dwell ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    sys.addWaypoint("e1", "wp0", 0.0f, 0.0f, 0.0f, 5.0f); // dwell 5s at wp0
    sys.addWaypoint("e1", "wp1", 10.0f, 0.0f, 0.0f, 0.0f); // no dwell at wp1

    sys.startPatrol("e1");
    // wp0 has dwell, so status should be Dwelling immediately
    assertTrue(sys.getStatus("e1") == components::NpcPatrolRoute::Status::Dwelling, "Dwelling at wp0");
    assertTrue(approxEqual(sys.getDwellTimer("e1"), 5.0f), "Dwell timer is 5.0");

    // Tick 2 seconds: still dwelling
    sys.update(2.0f);
    assertTrue(sys.getStatus("e1") == components::NpcPatrolRoute::Status::Dwelling, "Still dwelling after 2s");
    assertTrue(approxEqual(sys.getDwellTimer("e1"), 3.0f), "Dwell timer reduced to 3.0");

    // Tick 4 more seconds: dwell expires, advances to wp1
    sys.update(4.0f);
    assertTrue(sys.getStatus("e1") == components::NpcPatrolRoute::Status::Traveling, "Traveling after dwell expires");
    assertTrue(sys.getCurrentWaypointIndex("e1") == 1, "Now at wp1");
    assertTrue(approxEqual(sys.getDwellTimer("e1"), 0.0f), "Dwell timer reset to 0");
}

static void testNpcPatrolRouteSetSpeed() {
    std::cout << "\n=== NpcPatrolRoute: SetSpeed ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.setSpeed("e1", 5.0f), "Set speed 5.0 succeeds");
    assertTrue(approxEqual(sys.getSpeed("e1"), 5.0f), "Speed is 5.0");

    assertTrue(!sys.setSpeed("e1", 0.0f), "Zero speed rejected");
    assertTrue(!sys.setSpeed("e1", -1.0f), "Negative speed rejected");
    assertTrue(approxEqual(sys.getSpeed("e1"), 5.0f), "Speed unchanged after rejection");

    assertTrue(!sys.setSpeed("nonexistent", 2.0f), "Set speed fails on missing entity");
}

static void testNpcPatrolRouteSetPatrolMode() {
    std::cout << "\n=== NpcPatrolRoute: SetPatrolMode ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.getPatrolMode("e1") == components::NpcPatrolRoute::PatrolMode::Loop, "Default mode is Loop");
    assertTrue(sys.setPatrolMode("e1", components::NpcPatrolRoute::PatrolMode::PingPong), "Set PingPong succeeds");
    assertTrue(sys.getPatrolMode("e1") == components::NpcPatrolRoute::PatrolMode::PingPong, "Mode is PingPong");
    assertTrue(sys.setPatrolMode("e1", components::NpcPatrolRoute::PatrolMode::Loop), "Set Loop succeeds");
    assertTrue(sys.getPatrolMode("e1") == components::NpcPatrolRoute::PatrolMode::Loop, "Mode is Loop");
    assertTrue(!sys.setPatrolMode("nonexistent", components::NpcPatrolRoute::PatrolMode::Loop), "Set mode fails on missing entity");
}

static void testNpcPatrolRouteMissing() {
    std::cout << "\n=== NpcPatrolRoute: Missing ===" << std::endl;
    ecs::World world;
    systems::NpcPatrolRouteSystem sys(&world);

    assertTrue(!sys.initialize("none"), "Init fails on missing");
    assertTrue(!sys.addWaypoint("none", "wp1", 0.0f, 0.0f, 0.0f), "Add fails on missing");
    assertTrue(!sys.removeWaypoint("none", "wp1"), "Remove fails on missing");
    assertTrue(!sys.clearWaypoints("none"), "Clear fails on missing");
    assertTrue(!sys.startPatrol("none"), "Start fails on missing");
    assertTrue(!sys.stopPatrol("none"), "Stop fails on missing");
    assertTrue(!sys.setPatrolMode("none", components::NpcPatrolRoute::PatrolMode::Loop), "SetMode fails on missing");
    assertTrue(!sys.setSpeed("none", 1.0f), "SetSpeed fails on missing");
    assertTrue(!sys.advanceWaypoint("none"), "Advance fails on missing");

    assertTrue(sys.getWaypointCount("none") == 0, "0 waypoints on missing");
    assertTrue(sys.getCurrentWaypointId("none") == "", "Empty id on missing");
    assertTrue(sys.getCurrentWaypointIndex("none") == 0, "0 index on missing");
    assertTrue(sys.getStatus("none") == components::NpcPatrolRoute::Status::Idle, "Idle status on missing");
    assertTrue(sys.getPatrolMode("none") == components::NpcPatrolRoute::PatrolMode::Loop, "Loop mode on missing");
    assertTrue(!sys.isPatrolling("none"), "Not patrolling on missing");
    assertTrue(approxEqual(sys.getSpeed("none"), 0.0f), "0 speed on missing");
    assertTrue(approxEqual(sys.getDwellTimer("none"), 0.0f), "0 dwell timer on missing");
    assertTrue(sys.getTotalCircuits("none") == 0, "0 circuits on missing");
    assertTrue(sys.getTotalWaypointsVisited("none") == 0, "0 visited on missing");
}

void run_npc_patrol_route_system_tests() {
    testNpcPatrolRouteInit();
    testNpcPatrolRouteAddWaypoint();
    testNpcPatrolRouteMaxWaypoints();
    testNpcPatrolRouteRemoveWaypoint();
    testNpcPatrolRouteClearWaypoints();
    testNpcPatrolRouteStartStop();
    testNpcPatrolRouteAdvanceLoop();
    testNpcPatrolRoutePingPong();
    testNpcPatrolRouteDwell();
    testNpcPatrolRouteSetSpeed();
    testNpcPatrolRouteSetPatrolMode();
    testNpcPatrolRouteMissing();
}
