// Tests for: DockingBayAllocationSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/docking_bay_allocation_system.h"

using namespace atlas;

// ==================== DockingBayAllocationSystem Tests ====================

static void testDockingBayAllocationCreate() {
    std::cout << "\n=== DockingBayAllocation: Create ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    assertTrue(sys.initialize("station1", "hub_alpha"), "Init with station ID succeeds");
    assertTrue(sys.getStationId("station1") == "hub_alpha", "Station ID matches");
    assertTrue(sys.getTotalBays("station1") == 0, "Zero bays initially");
    assertTrue(sys.getOccupiedBays("station1") == 0, "Zero occupied");
    assertTrue(sys.getQueueLength("station1") == 0, "Empty queue");
    assertTrue(sys.getTotalDockings("station1") == 0, "Zero dockings");
    assertTrue(sys.getTotalUndockings("station1") == 0, "Zero undockings");
}

static void testDockingBayAllocationInvalidInit() {
    std::cout << "\n=== DockingBayAllocation: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    assertTrue(!sys.initialize("missing", "hub"), "Missing entity fails");
    world.createEntity("station1");
    assertTrue(!sys.initialize("station1", ""), "Empty station ID fails");
}

static void testDockingBayAllocationAddBay() {
    std::cout << "\n=== DockingBayAllocation: AddBay ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    assertTrue(sys.addBay("station1", "bay_1", "small"), "Add small bay");
    assertTrue(sys.addBay("station1", "bay_2", "medium"), "Add medium bay");
    assertTrue(sys.addBay("station1", "bay_3", "large"), "Add large bay");
    assertTrue(sys.getTotalBays("station1") == 3, "3 bays");

    // Duplicate rejected
    assertTrue(!sys.addBay("station1", "bay_1", "small"), "Duplicate bay ID rejected");

    // Invalid size rejected
    assertTrue(!sys.addBay("station1", "bay_4", "huge"), "Invalid size rejected");
    assertTrue(!sys.addBay("station1", "", "small"), "Empty bay ID rejected");
}

static void testDockingBayAllocationRemoveBay() {
    std::cout << "\n=== DockingBayAllocation: RemoveBay ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    sys.addBay("station1", "bay_1", "small");
    sys.addBay("station1", "bay_2", "medium");

    assertTrue(sys.removeBay("station1", "bay_1"), "Remove bay 1 succeeds");
    assertTrue(sys.getTotalBays("station1") == 1, "1 bay remaining");
    assertTrue(!sys.removeBay("station1", "bay_1"), "Double remove fails");
    assertTrue(!sys.removeBay("station1", "nonexistent"), "Remove nonexistent fails");
}

static void testDockingBayAllocationRemoveOccupied() {
    std::cout << "\n=== DockingBayAllocation: RemoveOccupied ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    sys.addBay("station1", "bay_1", "small");
    sys.requestDocking("station1", "ship1", "small", 0);
    sys.assignBay("station1", "ship1");

    assertTrue(!sys.removeBay("station1", "bay_1"), "Cannot remove occupied bay");
}

static void testDockingBayAllocationDockingFlow() {
    std::cout << "\n=== DockingBayAllocation: DockingFlow ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    sys.addBay("station1", "bay_1", "small");
    sys.addBay("station1", "bay_2", "medium");

    // Ship requests docking
    assertTrue(sys.requestDocking("station1", "ship1", "small", 0), "Ship1 requests docking");
    assertTrue(sys.getQueueLength("station1") == 1, "1 in queue");

    // Assign bay
    assertTrue(sys.assignBay("station1", "ship1"), "Assign bay to ship1");
    assertTrue(sys.getOccupiedBays("station1") == 1, "1 occupied");
    assertTrue(sys.getQueueLength("station1") == 0, "Queue empty after assign");
    assertTrue(sys.isShipDocked("station1", "ship1"), "Ship1 is docked");
    assertTrue(sys.getTotalDockings("station1") == 1, "1 total docking");

    // Release bay
    assertTrue(sys.releaseBay("station1", "ship1"), "Release bay for ship1");
    assertTrue(sys.getOccupiedBays("station1") == 0, "Zero occupied after release");
    assertTrue(!sys.isShipDocked("station1", "ship1"), "Ship1 no longer docked");
    assertTrue(sys.getTotalUndockings("station1") == 1, "1 total undocking");
}

static void testDockingBayAllocationInvalidRequest() {
    std::cout << "\n=== DockingBayAllocation: InvalidRequest ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    sys.addBay("station1", "bay_1", "small");

    assertTrue(!sys.requestDocking("station1", "", "small", 0), "Empty ship ID rejected");
    assertTrue(!sys.requestDocking("station1", "ship1", "huge", 0), "Invalid size rejected");
    assertTrue(!sys.requestDocking("station1", "ship1", "small", -1), "Negative priority rejected");
    assertTrue(!sys.requestDocking("nonexistent", "ship1", "small", 0), "Missing entity rejected");

    // Request then try to request again
    sys.requestDocking("station1", "ship1", "small", 0);
    assertTrue(!sys.requestDocking("station1", "ship1", "small", 0), "Duplicate request rejected");
}

static void testDockingBayAllocationNoFreeBay() {
    std::cout << "\n=== DockingBayAllocation: NoFreeBay ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    sys.addBay("station1", "bay_1", "small");

    // Dock ship1
    sys.requestDocking("station1", "ship1", "small", 0);
    sys.assignBay("station1", "ship1");

    // Ship2 requests same size — no bay available
    sys.requestDocking("station1", "ship2", "small", 0);
    assertTrue(!sys.assignBay("station1", "ship2"), "No free small bay");
    assertTrue(sys.getQueueLength("station1") == 1, "Ship2 still in queue");
}

static void testDockingBayAllocationFreeBaysBySize() {
    std::cout << "\n=== DockingBayAllocation: FreeBaysBySize ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    sys.addBay("station1", "bay_s1", "small");
    sys.addBay("station1", "bay_s2", "small");
    sys.addBay("station1", "bay_m1", "medium");
    sys.addBay("station1", "bay_l1", "large");

    assertTrue(sys.getFreeBays("station1", "small") == 2, "2 free small bays");
    assertTrue(sys.getFreeBays("station1", "medium") == 1, "1 free medium bay");
    assertTrue(sys.getFreeBays("station1", "large") == 1, "1 free large bay");

    // Dock in a small bay
    sys.requestDocking("station1", "ship1", "small", 0);
    sys.assignBay("station1", "ship1");
    assertTrue(sys.getFreeBays("station1", "small") == 1, "1 free small after dock");
}

static void testDockingBayAllocationAvgWaitTime() {
    std::cout << "\n=== DockingBayAllocation: AvgWaitTime ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    sys.addBay("station1", "bay_1", "small");

    sys.requestDocking("station1", "ship1", "small", 0);
    sys.update(5.0f); // Ship waits 5 seconds
    sys.assignBay("station1", "ship1");

    float avg = sys.getAvgWaitTime("station1");
    assertTrue(avg > 4.9f && avg < 5.1f, "Avg wait ~5s after one dock");
}

static void testDockingBayAllocationUpdate() {
    std::cout << "\n=== DockingBayAllocation: Update ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    world.createEntity("station1");
    sys.initialize("station1", "hub_alpha");

    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

static void testDockingBayAllocationMissing() {
    std::cout << "\n=== DockingBayAllocation: Missing ===" << std::endl;
    ecs::World world;
    systems::DockingBayAllocationSystem sys(&world);
    assertTrue(sys.getTotalBays("x") == 0, "Default bays on missing");
    assertTrue(sys.getOccupiedBays("x") == 0, "Default occupied on missing");
    assertTrue(sys.getFreeBays("x", "small") == 0, "Default free on missing");
    assertTrue(sys.getQueueLength("x") == 0, "Default queue on missing");
    assertTrue(!sys.isShipDocked("x", "s"), "Default docked on missing");
    assertTrue(sys.getTotalDockings("x") == 0, "Default dockings on missing");
    assertTrue(sys.getTotalUndockings("x") == 0, "Default undockings on missing");
    assertTrue(approxEqual(sys.getAvgWaitTime("x"), 0.0f), "Default avg wait on missing");
    assertTrue(sys.getStationId("x").empty(), "Default station ID on missing");
}

void run_docking_bay_allocation_system_tests() {
    testDockingBayAllocationCreate();
    testDockingBayAllocationInvalidInit();
    testDockingBayAllocationAddBay();
    testDockingBayAllocationRemoveBay();
    testDockingBayAllocationRemoveOccupied();
    testDockingBayAllocationDockingFlow();
    testDockingBayAllocationInvalidRequest();
    testDockingBayAllocationNoFreeBay();
    testDockingBayAllocationFreeBaysBySize();
    testDockingBayAllocationAvgWaitTime();
    testDockingBayAllocationUpdate();
    testDockingBayAllocationMissing();
}
