// Tests for: Interior Door System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/fps_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/movement_system.h"
#include "systems/interior_door_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Interior Door System Tests ====================

static void testInteriorDoorCreate() {
    std::cout << "\n=== Interior Door Create ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);

    assertTrue(sys.createDoor("door1", "interior1", "roomA", "roomB",
               static_cast<int>(components::InteriorDoor::DoorType::Standard)),
               "Door created");
    assertTrue(!sys.createDoor("door1", "interior1", "roomA", "roomB"),
               "Duplicate door fails");
}

static void testInteriorDoorOpenClose() {
    std::cout << "\n=== Interior Door Open/Close ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("door1", "interior1", "roomA", "roomB");

    assertTrue(sys.getDoorState("door1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Closed),
               "Starts closed");
    assertTrue(sys.requestOpen("door1"), "Open request succeeds");
    assertTrue(sys.getDoorState("door1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Opening),
               "State is Opening");

    // Simulate until fully open
    for (int i = 0; i < 30; ++i) sys.update(0.1f);

    assertTrue(sys.getDoorState("door1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Open),
               "Door fully open");
    assertTrue(approxEqual(sys.getOpenProgress("door1"), 1.0f), "Progress is 1.0");
}

static void testInteriorDoorAutoClose() {
    std::cout << "\n=== Interior Door Auto-Close ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("door1", "interior1", "roomA", "roomB");

    sys.requestOpen("door1");
    // Open fully
    for (int i = 0; i < 30; ++i) sys.update(0.1f);

    // Wait for auto-close (default 5 seconds)
    for (int i = 0; i < 60; ++i) sys.update(0.1f);

    // Should be closing or closed by now
    int state = sys.getDoorState("door1");
    assertTrue(state == static_cast<int>(components::InteriorDoor::DoorState::Closing) ||
               state == static_cast<int>(components::InteriorDoor::DoorState::Closed),
               "Door auto-closing after delay");
}

static void testInteriorDoorLock() {
    std::cout << "\n=== Interior Door Lock ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("door1", "interior1", "roomA", "roomB");

    assertTrue(sys.lockDoor("door1"), "Lock succeeds when closed");
    assertTrue(sys.isLocked("door1"), "Door is locked");
    assertTrue(sys.getDoorState("door1") ==
               static_cast<int>(components::InteriorDoor::DoorState::Locked),
               "State is Locked");
    assertTrue(!sys.requestOpen("door1"), "Can't open locked door");

    assertTrue(sys.unlockDoor("door1"), "Unlock succeeds");
    assertTrue(!sys.isLocked("door1"), "Door is unlocked");
    assertTrue(sys.requestOpen("door1"), "Can open after unlock");
}

static void testInteriorDoorAirlockPressure() {
    std::cout << "\n=== Interior Door Airlock Pressure ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("airlock1", "interior1", "roomA", "roomB",
                   static_cast<int>(components::InteriorDoor::DoorType::Airlock));

    // Normal pressure — should open
    sys.setPressure("airlock1", 1.0f, 1.0f);
    assertTrue(sys.requestOpen("airlock1"), "Opens with equal pressure");

    // Reset to closed for next test
    ecs::World world2;
    systems::InteriorDoorSystem sys2(&world2);
    sys2.createDoor("airlock2", "interior1", "roomA", "roomB",
                    static_cast<int>(components::InteriorDoor::DoorType::Airlock));

    // Vacuum on one side — should fail
    sys2.setPressure("airlock2", 1.0f, 0.0f);
    assertTrue(!sys2.requestOpen("airlock2"), "Refuses to open with pressure diff");
    assertTrue(sys2.hasPressureWarning("airlock2") == false,
               "No warning until update");

    sys2.update(0.0f);  // Trigger pressure warning calculation
    assertTrue(sys2.hasPressureWarning("airlock2"), "Pressure warning after update");
}

static void testInteriorDoorSecurityAccess() {
    std::cout << "\n=== Interior Door Security Access ===" << std::endl;
    ecs::World world;
    systems::InteriorDoorSystem sys(&world);
    sys.createDoor("secDoor1", "interior1", "roomA", "roomB",
                   static_cast<int>(components::InteriorDoor::DoorType::Security));

    // Set access requirement
    auto* entity = world.getEntity("secDoor1");
    auto* door = entity->getComponent<components::InteriorDoor>();
    door->required_access = "engineer";

    assertTrue(!sys.requestOpen("secDoor1", ""), "No access denied");
    assertTrue(!sys.requestOpen("secDoor1", "civilian"), "Wrong access denied");
    assertTrue(sys.requestOpen("secDoor1", "engineer"), "Correct access granted");
}

static void testInteriorDoorStateNames() {
    std::cout << "\n=== Interior Door State Names ===" << std::endl;
    assertTrue(systems::InteriorDoorSystem::stateName(0) == "Closed", "Closed name");
    assertTrue(systems::InteriorDoorSystem::stateName(1) == "Opening", "Opening name");
    assertTrue(systems::InteriorDoorSystem::stateName(2) == "Open", "Open name");
    assertTrue(systems::InteriorDoorSystem::stateName(3) == "Closing", "Closing name");
    assertTrue(systems::InteriorDoorSystem::stateName(4) == "Locked", "Locked name");
}

static void testInteriorDoorTypeNames() {
    std::cout << "\n=== Interior Door Type Names ===" << std::endl;
    assertTrue(systems::InteriorDoorSystem::typeName(0) == "Standard", "Standard name");
    assertTrue(systems::InteriorDoorSystem::typeName(1) == "Airlock", "Airlock name");
    assertTrue(systems::InteriorDoorSystem::typeName(2) == "Security", "Security name");
    assertTrue(systems::InteriorDoorSystem::typeName(99) == "Unknown", "Unknown type");
}

static void testInteriorDoorComponentDefaults() {
    std::cout << "\n=== Interior Door Component Defaults ===" << std::endl;
    components::InteriorDoor d;
    assertTrue(d.door_type == 0, "Default type Standard");
    assertTrue(d.door_state == 0, "Default state Closed");
    assertTrue(approxEqual(d.open_progress, 0.0f), "Default progress 0");
    assertTrue(approxEqual(d.pressure_a, 1.0f), "Default pressure_a 1.0");
    assertTrue(approxEqual(d.pressure_b, 1.0f), "Default pressure_b 1.0");
    assertTrue(!d.is_locked, "Default not locked");
}


void run_interior_door_system_tests() {
    testInteriorDoorCreate();
    testInteriorDoorOpenClose();
    testInteriorDoorAutoClose();
    testInteriorDoorLock();
    testInteriorDoorAirlockPressure();
    testInteriorDoorSecurityAccess();
    testInteriorDoorStateNames();
    testInteriorDoorTypeNames();
    testInteriorDoorComponentDefaults();
}
