// Tests for: BikeGarageSystem Tests
#include "test_log.h"
#include "ecs/system.h"
#include "systems/bike_garage_system.h"

using namespace atlas;

// ==================== BikeGarageSystem Tests ====================

static void testBikeGarageInit() {
    std::cout << "\n=== Bike Garage: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    assertTrue(sys.initializeGarage("ship1", "ship_owner", 2), "Garage initialized");
    assertTrue(sys.getCapacity("ship1") == 2, "Capacity is 2");
    assertTrue(sys.getBikeCount("ship1") == 0, "No bikes initially");
    assertTrue(!sys.isFull("ship1"), "Not full");
    assertTrue(!sys.initializeGarage("ship1", "other_owner", 3), "Duplicate init rejected");
}

static void testBikeGarageStore() {
    std::cout << "\n=== Bike Garage: Store Bike ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);

    assertTrue(sys.storeBike("ship1", "bike1", 12345, "Solari"), "Bike stored");
    assertTrue(sys.getBikeCount("ship1") == 1, "1 bike");
    assertTrue(sys.hasBike("ship1", "bike1"), "Has bike1");
    assertTrue(!sys.storeBike("ship1", "bike1", 67890, "Veyren"), "Duplicate bike rejected");
}

static void testBikeGarageRetrieve() {
    std::cout << "\n=== Bike Garage: Retrieve Bike ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 12345, "Solari");

    assertTrue(sys.retrieveBike("ship1", "bike1"), "Bike retrieved");
    assertTrue(sys.getBikeCount("ship1") == 0, "0 bikes");
    assertTrue(!sys.hasBike("ship1", "bike1"), "No longer has bike1");
    assertTrue(!sys.retrieveBike("ship1", "bike1"), "Double retrieve fails");
}

static void testBikeGarageFull() {
    std::cout << "\n=== Bike Garage: Full ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 111, "Solari");
    sys.storeBike("ship1", "bike2", 222, "Veyren");

    assertTrue(sys.isFull("ship1"), "Garage is full");
    assertTrue(!sys.storeBike("ship1", "bike3", 333, "Aurelian"), "Third bike rejected");
}

static void testBikeGarageLock() {
    std::cout << "\n=== Bike Garage: Lock/Unlock ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 12345, "Solari");

    assertTrue(!sys.isBikeLocked("ship1", "bike1"), "Not locked initially");
    assertTrue(sys.lockBike("ship1", "bike1"), "Locked");
    assertTrue(sys.isBikeLocked("ship1", "bike1"), "Is locked");
    assertTrue(!sys.retrieveBike("ship1", "bike1"), "Cannot retrieve locked");
    assertTrue(sys.unlockBike("ship1", "bike1"), "Unlocked");
    assertTrue(sys.retrieveBike("ship1", "bike1"), "Can retrieve after unlock");
}

static void testBikeGarageFuel() {
    std::cout << "\n=== Bike Garage: Fuel ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 12345, "Solari");

    assertTrue(approxEqual(sys.getBikeFuel("ship1", "bike1"), 100.0f), "Full fuel");
    assertTrue(sys.setBikeFuel("ship1", "bike1", 50.0f), "Set fuel");
    assertTrue(approxEqual(sys.getBikeFuel("ship1", "bike1"), 50.0f), "Fuel at 50%");
}

static void testBikeGarageHull() {
    std::cout << "\n=== Bike Garage: Hull Integrity ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);
    sys.storeBike("ship1", "bike1", 12345, "Solari");

    assertTrue(approxEqual(sys.getBikeHullIntegrity("ship1", "bike1"), 100.0f), "Full hull");
    assertTrue(sys.setBikeHullIntegrity("ship1", "bike1", 75.0f), "Set hull");
    assertTrue(approxEqual(sys.getBikeHullIntegrity("ship1", "bike1"), 75.0f), "Hull at 75%");
}

static void testBikeGarageDoor() {
    std::cout << "\n=== Bike Garage: Door ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);

    assertTrue(!sys.isDoorOpen("ship1"), "Door closed initially");
    assertTrue(sys.openDoor("ship1"), "Door opened");
    assertTrue(sys.isDoorOpen("ship1"), "Door is open");

    sys.update(2.0f); // Animate door
    assertTrue(sys.getDoorProgress("ship1") > 0.0f, "Door progress > 0");

    assertTrue(sys.closeDoor("ship1"), "Door closed");
    assertTrue(!sys.isDoorOpen("ship1"), "Door is closed");
}

static void testBikeGaragePower() {
    std::cout << "\n=== Bike Garage: Power ===" << std::endl;
    ecs::World world;
    world.createEntity("ship1");

    systems::BikeGarageSystem sys(&world);
    sys.initializeGarage("ship1", "owner", 2);

    assertTrue(sys.isPowerEnabled("ship1"), "Power enabled initially");
    assertTrue(sys.setPowerEnabled("ship1", false), "Power disabled");
    assertTrue(!sys.isPowerEnabled("ship1"), "Power is off");
    assertTrue(!sys.openDoor("ship1"), "Cannot open door without power");
    assertTrue(!sys.storeBike("ship1", "bike1", 111, "Solari"), "Cannot store without power");
}

static void testBikeGarageMissing() {
    std::cout << "\n=== Bike Garage: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::BikeGarageSystem sys(&world);
    assertTrue(sys.getBikeCount("nonexistent") == 0, "No bikes on missing");
    assertTrue(sys.getCapacity("nonexistent") == 0, "No capacity on missing");
    assertTrue(sys.isFull("nonexistent"), "Full on missing (default true)");
    assertTrue(!sys.hasBike("nonexistent", "bike1"), "No bike on missing");
}


void run_bike_garage_system_tests() {
    testBikeGarageInit();
    testBikeGarageStore();
    testBikeGarageRetrieve();
    testBikeGarageFull();
    testBikeGarageLock();
    testBikeGarageFuel();
    testBikeGarageHull();
    testBikeGarageDoor();
    testBikeGaragePower();
    testBikeGarageMissing();
}
