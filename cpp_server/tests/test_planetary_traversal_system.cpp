// Tests for: Planetary Traversal System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "ecs/system.h"
#include "systems/planetary_traversal_system.h"

using namespace atlas;

// ==================== Planetary Traversal System Tests ====================

static void testTraversalInit() {
    std::cout << "\n=== Planetary Traversal: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    assertTrue(sys.initializeTraversal("explorer1", "planet_001", 10.0f, 20.0f), "Traversal initialized");
    assertTrue(approxEqual(sys.getPositionX("explorer1"), 10.0f), "Start X correct");
    assertTrue(approxEqual(sys.getPositionY("explorer1"), 20.0f), "Start Y correct");
    assertTrue(!sys.initializeTraversal("explorer1", "planet_002", 0.0f, 0.0f), "Duplicate init rejected");
}

static void testTraversalSetDestination() {
    std::cout << "\n=== Planetary Traversal: Set Destination ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);

    assertTrue(sys.setDestination("explorer1", 100.0f, 100.0f), "Destination set");
    assertTrue(sys.isTraversing("explorer1"), "Is traversing");
    assertTrue(sys.getDistanceToDestination("explorer1") > 0.0f, "Distance to dest > 0");
}

static void testTraversalMovement() {
    std::cout << "\n=== Planetary Traversal: Movement ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setDestination("explorer1", 100.0f, 0.0f);

    sys.update(1.0f);
    assertTrue(sys.getPositionX("explorer1") > 0.0f, "Position X advanced");
    assertTrue(sys.isTraversing("explorer1"), "Still traversing");
    assertTrue(sys.getSpeed("explorer1") > 0.0f, "Speed > 0 while moving");
}

static void testTraversalArrival() {
    std::cout << "\n=== Planetary Traversal: Arrival ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setDestination("explorer1", 0.5f, 0.0f);  // very close

    sys.update(1.0f);
    assertTrue(!sys.isTraversing("explorer1"), "Arrived at destination");
    assertTrue(approxEqual(sys.getPositionX("explorer1"), 0.5f), "At destination X");
}

static void testTraversalVehicle() {
    std::cout << "\n=== Planetary Traversal: Vehicle ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);

    assertTrue(sys.setVehicle("explorer1", "rover_001", 20.0f), "Vehicle set");
    sys.setDestination("explorer1", 100.0f, 0.0f);
    sys.update(1.0f);
    assertTrue(sys.getSpeed("explorer1") > 5.0f, "Speed increased with vehicle");
}

static void testTraversalDismount() {
    std::cout << "\n=== Planetary Traversal: Dismount ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setVehicle("explorer1", "rover_001", 20.0f);

    assertTrue(sys.dismountVehicle("explorer1"), "Dismounted");
    sys.setDestination("explorer1", 100.0f, 0.0f);
    sys.update(1.0f);
    assertTrue(approxEqual(sys.getSpeed("explorer1"), 5.0f), "Speed reset to foot speed");
}

static void testTraversalTerrain() {
    std::cout << "\n=== Planetary Traversal: Terrain ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);

    assertTrue(sys.setTerrainType("explorer1", components::PlanetaryTraversal::TerrainType::Mountains), "Terrain set");
    assertTrue(sys.getTerrainTypeStr("explorer1") == "mountains", "Terrain is mountains");
    sys.setDestination("explorer1", 100.0f, 0.0f);
    sys.update(1.0f);
    assertTrue(sys.getSpeed("explorer1") < 5.0f, "Speed reduced on mountains");
}

static void testTraversalDistance() {
    std::cout << "\n=== Planetary Traversal: Distance ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setDestination("explorer1", 100.0f, 0.0f);

    sys.update(1.0f);
    sys.update(1.0f);
    assertTrue(sys.getDistanceTraveled("explorer1") > 0.0f, "Distance accumulated");
    float dist = sys.getDistanceTraveled("explorer1");
    sys.update(1.0f);
    assertTrue(sys.getDistanceTraveled("explorer1") > dist, "Distance keeps increasing");
}

static void testTraversalClear() {
    std::cout << "\n=== Planetary Traversal: Clear Destination ===" << std::endl;
    ecs::World world;
    world.createEntity("explorer1");

    systems::PlanetaryTraversalSystem sys(&world);
    sys.initializeTraversal("explorer1", "planet_001", 0.0f, 0.0f);
    sys.setDestination("explorer1", 100.0f, 100.0f);

    assertTrue(sys.isTraversing("explorer1"), "Traversing after set");
    assertTrue(sys.clearDestination("explorer1"), "Cleared destination");
    assertTrue(!sys.isTraversing("explorer1"), "Not traversing after clear");
}

static void testTraversalMissing() {
    std::cout << "\n=== Planetary Traversal: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::PlanetaryTraversalSystem sys(&world);
    assertTrue(!sys.initializeTraversal("nonexistent", "planet", 0.0f, 0.0f), "Init fails on missing");
    assertTrue(!sys.setDestination("nonexistent", 10.0f, 10.0f), "SetDest fails on missing");
    assertTrue(approxEqual(sys.getPositionX("nonexistent"), 0.0f), "PosX 0 on missing");
    assertTrue(approxEqual(sys.getDistanceTraveled("nonexistent"), 0.0f), "Distance 0 on missing");
    assertTrue(!sys.isTraversing("nonexistent"), "Not traversing on missing");
}


void run_planetary_traversal_system_tests() {
    testTraversalInit();
    testTraversalSetDestination();
    testTraversalMovement();
    testTraversalArrival();
    testTraversalVehicle();
    testTraversalDismount();
    testTraversalTerrain();
    testTraversalDistance();
    testTraversalClear();
    testTraversalMissing();
}
