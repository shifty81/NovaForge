// Tests for: ControlPortSystem
#include "test_log.h"
#include "components/fps_components.h"
#include "ecs/system.h"
#include "systems/control_port_system.h"

using namespace atlas;

// ==================== ControlPortSystem Tests ====================

static void testControlPortInit() {
    std::cout << "\n=== ControlPort: Init ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.get_port_count("e1") == 0, "No ports initially");
    assertTrue(sys.get_total_uses("e1") == 0, "Zero uses initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testControlPortAddRemove() {
    std::cout << "\n=== ControlPort: AddRemove ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(sys.add_port("e1", "turret_1", "turret", 2), "Add turret port");
    assertTrue(sys.add_port("e1", "pilot", "cockpit", 1), "Add cockpit port");
    assertTrue(sys.get_port_count("e1") == 2, "Two ports");
    assertTrue(!sys.add_port("e1", "turret_1", "turret", 2), "Duplicate rejected");
    assertTrue(sys.get_port_count("e1") == 2, "Still two ports");

    assertTrue(sys.remove_port("e1", "turret_1"), "Remove turret");
    assertTrue(sys.get_port_count("e1") == 1, "One port left");
    assertTrue(!sys.remove_port("e1", "turret_1"), "Remove nonexistent fails");
}

static void testControlPortAddValidation() {
    std::cout << "\n=== ControlPort: AddValidation ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.add_port("e1", "", "type", 0), "Empty port_id rejected");
    assertTrue(sys.get_port_count("e1") == 0, "No ports added");
}

static void testControlPortOccupy() {
    std::cout << "\n=== ControlPort: Occupy ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_port("e1", "p1", "turret", 2);

    assertTrue(!sys.is_occupied("e1", "p1"), "Not occupied initially");
    assertTrue(sys.get_occupant("e1", "p1") == "", "No occupant");
    assertTrue(sys.occupy_port("e1", "p1", "player_a"), "Occupy succeeds");
    assertTrue(sys.is_occupied("e1", "p1"), "Now occupied");
    assertTrue(sys.get_occupant("e1", "p1") == "player_a", "Occupant is player_a");
    assertTrue(sys.get_total_uses("e1") == 1, "One use");

    // Can't double-occupy
    assertTrue(!sys.occupy_port("e1", "p1", "player_b"), "Double-occupy rejected");
    assertTrue(sys.get_total_uses("e1") == 1, "Still one use");
}

static void testControlPortVacate() {
    std::cout << "\n=== ControlPort: Vacate ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_port("e1", "p1", "turret", 2);

    assertTrue(!sys.vacate_port("e1", "p1"), "Can't vacate unoccupied");
    sys.occupy_port("e1", "p1", "player_a");
    assertTrue(sys.vacate_port("e1", "p1"), "Vacate succeeds");
    assertTrue(!sys.is_occupied("e1", "p1"), "No longer occupied");
    assertTrue(sys.get_occupant("e1", "p1") == "", "Occupant cleared");

    // Re-occupy
    assertTrue(sys.occupy_port("e1", "p1", "player_b"), "Re-occupy succeeds");
    assertTrue(sys.get_occupant("e1", "p1") == "player_b", "New occupant");
    assertTrue(sys.get_total_uses("e1") == 2, "Two total uses");
}

static void testControlPortType() {
    std::cout << "\n=== ControlPort: Type ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_port("e1", "p1", "turret", 2);
    sys.add_port("e1", "p2", "cockpit", 1);

    assertTrue(sys.get_port_type("e1", "p1") == "turret", "Type is turret");
    assertTrue(sys.get_port_type("e1", "p2") == "cockpit", "Type is cockpit");
    assertTrue(sys.get_port_type("e1", "p3") == "", "Unknown port returns empty");
}

static void testControlPortClear() {
    std::cout << "\n=== ControlPort: Clear ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_port("e1", "p1", "turret", 2);
    sys.add_port("e1", "p2", "cockpit", 1);
    assertTrue(sys.get_port_count("e1") == 2, "Two ports before clear");
    assertTrue(sys.clear_ports("e1"), "Clear succeeds");
    assertTrue(sys.get_port_count("e1") == 0, "Zero ports after clear");
}

static void testControlPortMaxCap() {
    std::cout << "\n=== ControlPort: MaxCap ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    for (int i = 0; i < 20; i++) {
        std::string id = "port_" + std::to_string(i);
        assertTrue(sys.add_port("e1", id, "generic", 0), "Add port within limit");
    }
    assertTrue(!sys.add_port("e1", "port_20", "generic", 0), "Blocked at max");
    assertTrue(sys.get_port_count("e1") == 20, "At max capacity");
}

static void testControlPortUpdate() {
    std::cout << "\n=== ControlPort: Update ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.add_port("e1", "p1", "turret", 2);
    sys.occupy_port("e1", "p1", "player_a");

    sys.update(1.0f);
    // Use time should have accumulated — no public getter, but system should not crash
    assertTrue(sys.is_occupied("e1", "p1"), "Still occupied after update");
}

static void testControlPortMissing() {
    std::cout << "\n=== ControlPort: Missing ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);

    assertTrue(!sys.add_port("no", "p", "t", 0), "add_port fails on missing");
    assertTrue(!sys.remove_port("no", "p"), "remove_port fails");
    assertTrue(!sys.occupy_port("no", "p", "x"), "occupy_port fails");
    assertTrue(!sys.vacate_port("no", "p"), "vacate_port fails");
    assertTrue(!sys.clear_ports("no"), "clear_ports fails");
    assertTrue(!sys.is_occupied("no", "p"), "is_occupied default");
    assertTrue(sys.get_occupant("no", "p") == "", "get_occupant default");
    assertTrue(sys.get_port_count("no") == 0, "get_port_count default");
    assertTrue(sys.get_port_type("no", "p") == "", "get_port_type default");
    assertTrue(sys.get_total_uses("no") == 0, "get_total_uses default");
}

static void testControlPortOccupyNonexistentPort() {
    std::cout << "\n=== ControlPort: OccupyNonexistentPort ===" << std::endl;
    ecs::World world;
    systems::ControlPortSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    assertTrue(!sys.occupy_port("e1", "ghost", "player"), "Occupy nonexistent port fails");
    assertTrue(!sys.vacate_port("e1", "ghost"), "Vacate nonexistent port fails");
}

void run_control_port_system_tests() {
    testControlPortInit();
    testControlPortAddRemove();
    testControlPortAddValidation();
    testControlPortOccupy();
    testControlPortVacate();
    testControlPortType();
    testControlPortClear();
    testControlPortMaxCap();
    testControlPortUpdate();
    testControlPortMissing();
    testControlPortOccupyNonexistentPort();
}
