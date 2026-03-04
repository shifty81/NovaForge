// Tests for: RoverInteriorSystem Tests
#include "test_log.h"
#include "components/exploration_components.h"
#include "components/fps_components.h"
#include "components/mission_components.h"
#include "ecs/system.h"
#include "systems/rover_interior_system.h"

using namespace atlas;

// ==================== RoverInteriorSystem Tests ====================

static void testRoverInteriorInit() {
    std::cout << "\n=== Rover Interior: Initialize ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    assertTrue(sys.initializeInterior("rover1", "rover_001"), "Interior initialized");
    assertTrue(sys.isPressurized("rover1"), "Initially pressurized");
    assertTrue(approxEqual(sys.getOxygenLevel("rover1"), 100.0f), "Oxygen at 100%");
    assertTrue(!sys.initializeInterior("rover1", "rover_002"), "Duplicate init rejected");
}

static void testRoverInteriorAddRoom() {
    std::cout << "\n=== Rover Interior: Add Room ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");

    assertTrue(sys.addRoom("rover1", "cockpit", components::RoverInterior::RoomType::Cockpit), "Cockpit added");
    assertTrue(sys.getRoomCount("rover1") == 1, "1 room");
    assertTrue(sys.getRoomType("rover1", "cockpit") == "cockpit", "Room type is cockpit");
    assertTrue(!sys.addRoom("rover1", "cockpit", components::RoverInterior::RoomType::CargoHold), "Duplicate room rejected");
}

static void testRoverInteriorRigLocker() {
    std::cout << "\n=== Rover Interior: Rig Locker ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");

    assertTrue(!sys.hasRigLocker("rover1"), "No rig locker initially");
    assertTrue(!sys.storeRig("rover1", "rig1"), "Cannot store without locker");

    sys.addRoom("rover1", "locker", components::RoverInterior::RoomType::RigLocker);
    sys.update(0.1f);  // Update to detect rig locker
    assertTrue(sys.hasRigLocker("rover1"), "Has rig locker after adding");
    assertTrue(sys.storeRig("rover1", "rig1"), "Rig stored");
    assertTrue(sys.getStoredRigCount("rover1") == 1, "1 rig stored");
    assertTrue(sys.retrieveRig("rover1", "rig1"), "Rig retrieved");
    assertTrue(sys.getStoredRigCount("rover1") == 0, "0 rigs after retrieval");
}

static void testRoverInteriorEquipment() {
    std::cout << "\n=== Rover Interior: Equipment ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");
    sys.addRoom("rover1", "bay", components::RoverInterior::RoomType::EquipmentBay);

    assertTrue(sys.installEquipment("rover1", "bay", "equip1"), "Equipment installed");
    assertTrue(sys.getEquipmentCount("rover1", "bay") == 1, "1 equipment");
    assertTrue(sys.removeEquipment("rover1", "bay", "equip1"), "Equipment removed");
    assertTrue(sys.getEquipmentCount("rover1", "bay") == 0, "0 equipment after removal");
}

static void testRoverInteriorPressurization() {
    std::cout << "\n=== Rover Interior: Pressurization ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");
    sys.addRoom("rover1", "room", components::RoverInterior::RoomType::CargoHold);

    assertTrue(sys.isPressurized("rover1"), "Initially pressurized");
    assertTrue(sys.setPressurized("rover1", false), "Depressurize");
    assertTrue(!sys.isPressurized("rover1"), "Not pressurized");

    sys.update(10.0f); // 10 seconds of oxygen leak
    assertTrue(sys.getOxygenLevel("rover1") < 100.0f, "Oxygen decreased");
}

static void testRoverInteriorVolume() {
    std::cout << "\n=== Rover Interior: Volume ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");
    sys.addRoom("rover1", "cargo", components::RoverInterior::RoomType::CargoHold);
    sys.update(0.1f);

    assertTrue(sys.getTotalVolume("rover1") > 0.0f, "Volume > 0 after adding room");
}

static void testRoverInteriorMaxRooms() {
    std::cout << "\n=== Rover Interior: Max Rooms ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");

    // Default max is 4
    sys.addRoom("rover1", "r1", components::RoverInterior::RoomType::Cockpit);
    sys.addRoom("rover1", "r2", components::RoverInterior::RoomType::CargoHold);
    sys.addRoom("rover1", "r3", components::RoverInterior::RoomType::Scanner);
    sys.addRoom("rover1", "r4", components::RoverInterior::RoomType::Airlock);
    assertTrue(sys.getRoomCount("rover1") == 4, "4 rooms");
    assertTrue(!sys.addRoom("rover1", "r5", components::RoverInterior::RoomType::EquipmentBay), "5th room rejected");
}

static void testRoverInteriorRemoveRoom() {
    std::cout << "\n=== Rover Interior: Remove Room ===" << std::endl;
    ecs::World world;
    world.createEntity("rover1");

    systems::RoverInteriorSystem sys(&world);
    sys.initializeInterior("rover1", "rover_001");
    sys.addRoom("rover1", "cargo", components::RoverInterior::RoomType::CargoHold);
    assertTrue(sys.getRoomCount("rover1") == 1, "1 room");
    assertTrue(sys.removeRoom("rover1", "cargo"), "Room removed");
    assertTrue(sys.getRoomCount("rover1") == 0, "0 rooms");
    assertTrue(!sys.removeRoom("rover1", "cargo"), "Double remove fails");
}

static void testRoverInteriorMissing() {
    std::cout << "\n=== Rover Interior: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::RoverInteriorSystem sys(&world);
    assertTrue(sys.getRoomCount("nonexistent") == 0, "No rooms on missing");
    assertTrue(!sys.isPressurized("nonexistent"), "Not pressurized on missing");
    assertTrue(approxEqual(sys.getOxygenLevel("nonexistent"), 0.0f), "Zero oxygen on missing");
    assertTrue(!sys.hasRigLocker("nonexistent"), "No locker on missing");
}


void run_rover_interior_system_tests() {
    testRoverInteriorInit();
    testRoverInteriorAddRoom();
    testRoverInteriorRigLocker();
    testRoverInteriorEquipment();
    testRoverInteriorPressurization();
    testRoverInteriorVolume();
    testRoverInteriorMaxRooms();
    testRoverInteriorRemoveRoom();
    testRoverInteriorMissing();
}
