// Tests for: Transit Planner System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/transit_planner_system.h"

using namespace atlas;

// ==================== Transit Planner System Tests ====================

static void testTransitPlannerCreate() {
    std::cout << "\n=== TransitPlanner: Create ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_001"), "Init succeeds");
    assertTrue(sys.getWaypointCount("p1") == 0, "No waypoints");
    assertTrue(sys.getCurrentWaypointIndex("p1") == 0, "Index at 0");
    assertTrue(approxEqual(sys.getTotalTravelTime("p1"), 0.0f), "0 travel time");
    assertTrue(approxEqual(sys.getTotalFuelCost("p1"), 0.0f), "0 fuel cost");
    assertTrue(sys.isRouteComplete("p1"), "Empty route is complete");
}

static void testTransitPlannerAddWaypoints() {
    std::cout << "\n=== TransitPlanner: AddWaypoints ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.addWaypoint("p1", "wp1", "Jita", 60.0f, 10.0f), "Add Jita");
    assertTrue(sys.addWaypoint("p1", "wp2", "Amarr", 120.0f, 25.0f), "Add Amarr");
    assertTrue(sys.addWaypoint("p1", "wp3", "Dodixie", 90.0f, 15.0f), "Add Dodixie");
    assertTrue(sys.getWaypointCount("p1") == 3, "3 waypoints");
    // Duplicate ID
    assertTrue(!sys.addWaypoint("p1", "wp1", "Jita Again", 10.0f, 5.0f), "Duplicate rejected");
    assertTrue(sys.getWaypointCount("p1") == 3, "Still 3");
}

static void testTransitPlannerWaypointMax() {
    std::cout << "\n=== TransitPlanner: WaypointMax ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::TransitPlannerState>();
    state->max_waypoints = 2;
    sys.addWaypoint("p1", "wp1", "A", 10.0f, 5.0f);
    sys.addWaypoint("p1", "wp2", "B", 20.0f, 10.0f);
    assertTrue(!sys.addWaypoint("p1", "wp3", "C", 30.0f, 15.0f), "Max waypoints enforced");
    assertTrue(sys.getWaypointCount("p1") == 2, "Still 2");
}

static void testTransitPlannerRemoveWaypoint() {
    std::cout << "\n=== TransitPlanner: RemoveWaypoint ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addWaypoint("p1", "wp1", "Jita", 60.0f, 10.0f);
    sys.addWaypoint("p1", "wp2", "Amarr", 120.0f, 25.0f);
    assertTrue(sys.removeWaypoint("p1", "wp1"), "Remove Jita");
    assertTrue(sys.getWaypointCount("p1") == 1, "1 waypoint");
    assertTrue(sys.getCurrentWaypointName("p1") == "Amarr", "Amarr is now current");
    assertTrue(!sys.removeWaypoint("p1", "unknown"), "Unknown removal fails");
}

static void testTransitPlannerAdvance() {
    std::cout << "\n=== TransitPlanner: Advance ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addWaypoint("p1", "wp1", "Jita", 60.0f, 10.0f);
    sys.addWaypoint("p1", "wp2", "Amarr", 120.0f, 25.0f);
    sys.addWaypoint("p1", "wp3", "Dodixie", 90.0f, 15.0f);
    assertTrue(!sys.isRouteComplete("p1"), "Route not complete");
    assertTrue(sys.getCurrentWaypointName("p1") == "Jita", "Current is Jita");
    assertTrue(sys.getCurrentWaypointIndex("p1") == 0, "Index 0");
    assertTrue(sys.advanceToNextWaypoint("p1"), "Advance to leg 2");
    assertTrue(sys.getCurrentWaypointName("p1") == "Amarr", "Current is Amarr");
    assertTrue(sys.getCurrentWaypointIndex("p1") == 1, "Index 1");
    assertTrue(sys.advanceToNextWaypoint("p1"), "Advance to leg 3");
    assertTrue(sys.getCurrentWaypointName("p1") == "Dodixie", "Current is Dodixie");
    assertTrue(sys.advanceToNextWaypoint("p1"), "Complete route");
    assertTrue(sys.isRouteComplete("p1"), "Route complete");
    assertTrue(!sys.advanceToNextWaypoint("p1"), "Cannot advance past end");
}

static void testTransitPlannerTravelTime() {
    std::cout << "\n=== TransitPlanner: TravelTime ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addWaypoint("p1", "wp1", "A", 60.0f, 10.0f);
    sys.addWaypoint("p1", "wp2", "B", 120.0f, 25.0f);
    sys.addWaypoint("p1", "wp3", "C", 90.0f, 15.0f);
    assertTrue(approxEqual(sys.getTotalTravelTime("p1"), 270.0f), "270s total");
    assertTrue(approxEqual(sys.getRemainingTravelTime("p1"), 270.0f), "270s remaining");
    assertTrue(approxEqual(sys.getElapsedTravelTime("p1"), 0.0f), "0s elapsed");
    sys.advanceToNextWaypoint("p1"); // Complete leg 1 (60s)
    assertTrue(approxEqual(sys.getRemainingTravelTime("p1"), 210.0f), "210s remaining");
    assertTrue(approxEqual(sys.getElapsedTravelTime("p1"), 60.0f), "60s elapsed");
    sys.advanceToNextWaypoint("p1"); // Complete leg 2 (120s)
    assertTrue(approxEqual(sys.getRemainingTravelTime("p1"), 90.0f), "90s remaining");
    assertTrue(approxEqual(sys.getElapsedTravelTime("p1"), 180.0f), "180s elapsed");
}

static void testTransitPlannerFuelCost() {
    std::cout << "\n=== TransitPlanner: FuelCost ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addWaypoint("p1", "wp1", "A", 60.0f, 10.0f);
    sys.addWaypoint("p1", "wp2", "B", 120.0f, 25.0f);
    assertTrue(approxEqual(sys.getTotalFuelCost("p1"), 35.0f), "35 total fuel");
    assertTrue(approxEqual(sys.getRemainingFuelCost("p1"), 35.0f), "35 remaining fuel");
    sys.advanceToNextWaypoint("p1");
    assertTrue(approxEqual(sys.getRemainingFuelCost("p1"), 25.0f), "25 remaining fuel");
}

static void testTransitPlannerLegs() {
    std::cout << "\n=== TransitPlanner: Legs ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addWaypoint("p1", "wp1", "A", 10.0f, 5.0f);
    sys.addWaypoint("p1", "wp2", "B", 20.0f, 10.0f);
    sys.addWaypoint("p1", "wp3", "C", 30.0f, 15.0f);
    assertTrue(sys.getCompletedLegs("p1") == 0, "0 completed");
    assertTrue(sys.getRemainingLegs("p1") == 3, "3 remaining");
    sys.advanceToNextWaypoint("p1");
    assertTrue(sys.getCompletedLegs("p1") == 1, "1 completed");
    assertTrue(sys.getRemainingLegs("p1") == 2, "2 remaining");
    sys.advanceToNextWaypoint("p1");
    sys.advanceToNextWaypoint("p1");
    assertTrue(sys.getCompletedLegs("p1") == 3, "3 completed");
    assertTrue(sys.getRemainingLegs("p1") == 0, "0 remaining");
}

static void testTransitPlannerUpdate() {
    std::cout << "\n=== TransitPlanner: Update ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::TransitPlannerState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testTransitPlannerMissing() {
    std::cout << "\n=== TransitPlanner: Missing ===" << std::endl;
    ecs::World world;
    systems::TransitPlannerSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails");
    assertTrue(!sys.addWaypoint("nonexistent", "w", "n", 0, 0), "addWaypoint fails");
    assertTrue(!sys.removeWaypoint("nonexistent", "w"), "removeWaypoint fails");
    assertTrue(sys.getWaypointCount("nonexistent") == 0, "0 waypoints");
    assertTrue(!sys.advanceToNextWaypoint("nonexistent"), "advance fails");
    assertTrue(sys.getCurrentWaypointIndex("nonexistent") == -1, "-1 index");
    assertTrue(sys.getCurrentWaypointName("nonexistent") == "", "Empty name");
    assertTrue(!sys.isRouteComplete("nonexistent"), "Not complete");
    assertTrue(approxEqual(sys.getTotalTravelTime("nonexistent"), 0.0f), "0 total time");
    assertTrue(approxEqual(sys.getRemainingTravelTime("nonexistent"), 0.0f), "0 remaining time");
    assertTrue(approxEqual(sys.getElapsedTravelTime("nonexistent"), 0.0f), "0 elapsed time");
    assertTrue(approxEqual(sys.getTotalFuelCost("nonexistent"), 0.0f), "0 total fuel");
    assertTrue(approxEqual(sys.getRemainingFuelCost("nonexistent"), 0.0f), "0 remaining fuel");
    assertTrue(sys.getCompletedLegs("nonexistent") == 0, "0 completed");
    assertTrue(sys.getRemainingLegs("nonexistent") == 0, "0 remaining");
}

void run_transit_planner_system_tests() {
    testTransitPlannerCreate();
    testTransitPlannerAddWaypoints();
    testTransitPlannerWaypointMax();
    testTransitPlannerRemoveWaypoint();
    testTransitPlannerAdvance();
    testTransitPlannerTravelTime();
    testTransitPlannerFuelCost();
    testTransitPlannerLegs();
    testTransitPlannerUpdate();
    testTransitPlannerMissing();
}
