// Tests for: FleetDoctrineSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fleet_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "pcg/fleet_doctrine.h"
#include "systems/fleet_doctrine_system.h"

using namespace atlas;

// ==================== FleetDoctrineSystem Tests ====================

static void testFleetDoctrineCreate() {
    std::cout << "\n=== Fleet Doctrine: Create ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    assertTrue(sys.createDoctrine("fleet1", "doctrine1", "Shield Fleet"), "Doctrine created");
    assertTrue(!sys.createDoctrine("fleet1", "doctrine2", "Another"), "Duplicate doctrine rejected");
    assertTrue(sys.getDoctrineName("fleet1") == "Shield Fleet", "Doctrine name matches");
}

static void testFleetDoctrineAddSlot() {
    std::cout << "\n=== Fleet Doctrine: Add Slot ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Armor Fleet");
    assertTrue(sys.addSlot("fleet1", "DPS", "Battleship", 3, 5, true), "DPS slot added");
    assertTrue(sys.addSlot("fleet1", "Logistics", "Cruiser", 2, 3, true), "Logi slot added");
    assertTrue(sys.addSlot("fleet1", "Tackle", "Frigate", 1, 4, false), "Tackle slot added");
    assertTrue(!sys.addSlot("fleet1", "DPS", "Frigate", 1, 2, false), "Duplicate role rejected");
    assertTrue(sys.getSlotCount("fleet1") == 3, "Slot count is 3");
}

static void testFleetDoctrineRemoveSlot() {
    std::cout << "\n=== Fleet Doctrine: Remove Slot ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 3, 5, true);
    sys.addSlot("fleet1", "Logistics", "Cruiser", 2, 3, true);
    assertTrue(sys.removeSlot("fleet1", "DPS"), "Slot removed");
    assertTrue(sys.getSlotCount("fleet1") == 1, "Slot count after removal");
    assertTrue(!sys.removeSlot("fleet1", "nonexistent"), "Nonexistent slot removal fails");
}

static void testFleetDoctrineAssignShip() {
    std::cout << "\n=== Fleet Doctrine: Assign Ship ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 2, 3, true);
    assertTrue(sys.assignShip("fleet1", "DPS"), "Ship assigned to DPS");
    assertTrue(sys.assignShip("fleet1", "DPS"), "Second ship assigned");
    assertTrue(sys.assignShip("fleet1", "DPS"), "Third ship assigned (max)");
    assertTrue(!sys.assignShip("fleet1", "DPS"), "Fourth ship rejected (over max)");
}

static void testFleetDoctrineUnassignShip() {
    std::cout << "\n=== Fleet Doctrine: Unassign Ship ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 1, 3, true);
    sys.assignShip("fleet1", "DPS");
    sys.assignShip("fleet1", "DPS");
    assertTrue(sys.unassignShip("fleet1", "DPS"), "Ship unassigned");
    assertTrue(sys.unassignShip("fleet1", "DPS"), "Second ship unassigned");
    assertTrue(!sys.unassignShip("fleet1", "DPS"), "Unassign from empty fails");
}

static void testFleetDoctrineReadiness() {
    std::cout << "\n=== Fleet Doctrine: Readiness ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 2, 5, true);
    sys.addSlot("fleet1", "Logistics", "Cruiser", 1, 3, true);
    sys.addSlot("fleet1", "Tackle", "Frigate", 0, 4, false);  // optional

    // Initially 0% ready (0 of 2 required slots filled)
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getReadiness("fleet1"), 0.0f), "Initially not ready");
    assertTrue(!sys.isReady("fleet1"), "isReady false initially");

    // Fill DPS (still missing Logistics)
    sys.assignShip("fleet1", "DPS");
    sys.assignShip("fleet1", "DPS");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getReadiness("fleet1"), 0.5f), "50% ready with DPS filled");

    // Fill Logistics
    sys.assignShip("fleet1", "Logistics");
    sys.update(0.0f);
    assertTrue(approxEqual(sys.getReadiness("fleet1"), 1.0f), "100% ready");
    assertTrue(sys.isReady("fleet1"), "isReady true when all required filled");
}

static void testFleetDoctrineLock() {
    std::cout << "\n=== Fleet Doctrine: Lock ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 2, 5, true);

    sys.lockDoctrine("fleet1", true);
    assertTrue(sys.isLocked("fleet1"), "Doctrine is locked");
    assertTrue(!sys.addSlot("fleet1", "Logistics", "Cruiser", 1, 3, true), "Cannot add slot when locked");
    assertTrue(!sys.removeSlot("fleet1", "DPS"), "Cannot remove slot when locked");

    sys.lockDoctrine("fleet1", false);
    assertTrue(!sys.isLocked("fleet1"), "Doctrine unlocked");
    assertTrue(sys.addSlot("fleet1", "Logistics", "Cruiser", 1, 3, true), "Can add slot after unlock");
}

static void testFleetDoctrineShipCounts() {
    std::cout << "\n=== Fleet Doctrine: Ship Counts ===" << std::endl;
    ecs::World world;
    world.createEntity("fleet1");

    systems::FleetDoctrineSystem sys(&world);
    sys.createDoctrine("fleet1", "d1", "Test");
    sys.addSlot("fleet1", "DPS", "Battleship", 2, 5, true);
    sys.addSlot("fleet1", "Logistics", "Cruiser", 1, 3, true);
    sys.assignShip("fleet1", "DPS");
    sys.assignShip("fleet1", "DPS");
    sys.assignShip("fleet1", "Logistics");
    sys.update(0.0f);

    assertTrue(sys.getTargetShipCount("fleet1") == 8, "Target ship count is max_count sum (5+3)");
    assertTrue(sys.getCurrentShipCount("fleet1") == 3, "Current ship count is 3");
}

static void testFleetDoctrineMissing() {
    std::cout << "\n=== Fleet Doctrine: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::FleetDoctrineSystem sys(&world);
    assertTrue(approxEqual(sys.getReadiness("nonexistent"), 0.0f), "Default readiness for missing");
    assertTrue(sys.getSlotCount("nonexistent") == 0, "Default slot count for missing");
    assertTrue(!sys.isReady("nonexistent"), "Default not ready for missing");
    assertTrue(sys.getDoctrineName("nonexistent").empty(), "Default name empty for missing");
}

static void testFleetDoctrineComponentDefaults() {
    std::cout << "\n=== Fleet Doctrine: Component Defaults ===" << std::endl;
    components::FleetDoctrine doctrine;
    assertTrue(doctrine.slots.empty(), "Default slots empty");
    assertTrue(approxEqual(doctrine.readiness, 0.0f), "Default readiness is 0.0");
    assertTrue(!doctrine.is_locked, "Default not locked");
    assertTrue(doctrine.total_ship_target == 0, "Default target 0");
    assertTrue(doctrine.current_ship_count == 0, "Default current 0");
}


void run_fleet_doctrine_system_tests() {
    testFleetDoctrineCreate();
    testFleetDoctrineAddSlot();
    testFleetDoctrineRemoveSlot();
    testFleetDoctrineAssignShip();
    testFleetDoctrineUnassignShip();
    testFleetDoctrineReadiness();
    testFleetDoctrineLock();
    testFleetDoctrineShipCounts();
    testFleetDoctrineMissing();
    testFleetDoctrineComponentDefaults();
}
