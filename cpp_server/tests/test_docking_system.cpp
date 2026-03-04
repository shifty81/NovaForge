// Tests for: Docking System Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/docking_system.h"

using namespace atlas;

// ==================== Docking System Tests ====================

static void testDockingPortDefaults() {
    std::cout << "\n=== Docking Port Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("port1");
    auto* port = addComp<components::DockingPort>(e);
    assertTrue(!port->isOccupied(), "Not occupied by default");
}

static void testDockingDock() {
    std::cout << "\n=== Docking Dock ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("port2");
    addComp<components::DockingPort>(e);

    systems::DockingSystem sys(&world);
    assertTrue(sys.dock("port2", "ship_1"), "Dock succeeds");
    assertTrue(sys.isOccupied("port2"), "Port is occupied");
    assertTrue(sys.getDockedEntity("port2") == "ship_1", "Correct docked entity");
}

static void testDockingUndock() {
    std::cout << "\n=== Docking Undock ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("port3");
    addComp<components::DockingPort>(e);

    systems::DockingSystem sys(&world);
    sys.dock("port3", "ship_2");
    std::string undocked = sys.undock("port3");
    assertTrue(undocked == "ship_2", "Undocked correct entity");
    assertTrue(!sys.isOccupied("port3"), "Port is empty after undock");
}


void run_docking_system_tests() {
    testDockingPortDefaults();
    testDockingDock();
    testDockingUndock();
}
