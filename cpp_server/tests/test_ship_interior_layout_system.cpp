// Tests for: ShipInteriorLayoutSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/exploration_components.h"
#include "components/fps_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "pcg/deck_graph.h"
#include "systems/movement_system.h"
#include "systems/ship_interior_layout_system.h"

using namespace atlas;

// ==================== ShipInteriorLayoutSystem Tests ====================

static void testInteriorLayoutGenerateFrigate() {
    std::cout << "\n=== Interior Layout Generate Frigate ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    bool ok = sys.generateLayout("ship_1", "frigate");
    assertTrue(ok, "Generate frigate layout succeeds");
    assertTrue(sys.getRoomCount("ship_1") == 5, "Frigate has 5 rooms");
}

static void testInteriorLayoutGenerateBattleship() {
    std::cout << "\n=== Interior Layout Generate Battleship ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    bool ok = sys.generateLayout("ship_bs", "battleship");
    assertTrue(ok, "Generate battleship layout succeeds");
    assertTrue(sys.getRoomCount("ship_bs") == 10, "Battleship has 10 rooms");
}

static void testInteriorLayoutGenerateCapital() {
    std::cout << "\n=== Interior Layout Generate Capital ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    bool ok = sys.generateLayout("ship_cap", "capital");
    assertTrue(ok, "Generate capital layout succeeds");
    assertTrue(sys.getRoomCount("ship_cap") == 12, "Capital has 12 rooms");
}

static void testInteriorLayoutDuplicate() {
    std::cout << "\n=== Interior Layout Duplicate ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "frigate");
    bool dup = sys.generateLayout("ship_1", "frigate");
    assertTrue(!dup, "Duplicate generation fails");
}

static void testInteriorLayoutFindBridge() {
    std::cout << "\n=== Interior Layout Find Bridge ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "cruiser");
    std::string bridge = sys.findRoomByType("ship_1",
        components::ShipInteriorLayout::RoomType::Bridge);
    assertTrue(!bridge.empty(), "Bridge room found");
}

static void testInteriorLayoutFindEngineering() {
    std::cout << "\n=== Interior Layout Find Engineering ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "cruiser");
    std::string eng = sys.findRoomByType("ship_1",
        components::ShipInteriorLayout::RoomType::Engineering);
    assertTrue(!eng.empty(), "Engineering room found");
}

static void testInteriorLayoutConnections() {
    std::cout << "\n=== Interior Layout Connections ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "frigate");
    auto* layout = sys.getLayout("ship_1");
    assertTrue(layout != nullptr, "Layout exists");
    assertTrue(layout->connectionCount() > 0, "Has connections");

    // First two rooms should be connected
    if (layout->rooms.size() >= 2) {
        assertTrue(layout->areConnected(layout->rooms[0].room_id,
                                         layout->rooms[1].room_id),
                   "Adjacent rooms are connected");
    }
}

static void testInteriorLayoutAdjacentRooms() {
    std::cout << "\n=== Interior Layout Adjacent Rooms ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "cruiser");
    auto* layout = sys.getLayout("ship_1");
    assertTrue(layout != nullptr, "Layout exists");

    if (layout->rooms.size() >= 3) {
        // Middle room should have at least 2 neighbors
        auto adj = sys.getAdjacentRooms("ship_1", layout->rooms[1].room_id);
        assertTrue(adj.size() >= 2, "Middle room has at least 2 adjacent rooms");
    }
}

static void testInteriorLayoutUnknownShip() {
    std::cout << "\n=== Interior Layout Unknown Ship ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    assertTrue(sys.getRoomCount("nonexistent") == 0, "No rooms for unknown ship");
    assertTrue(sys.getLayout("nonexistent") == nullptr, "No layout for unknown ship");
}

static void testInteriorLayoutRoomTypeName() {
    std::cout << "\n=== Interior Layout Room Type Names ===" << std::endl;
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(0) == "Bridge", "Bridge name");
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(1) == "Engineering", "Engineering name");
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(6) == "Corridor", "Corridor name");
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(7) == "Airlock", "Airlock name");
    assertTrue(systems::ShipInteriorLayoutSystem::roomTypeName(99) == "Unknown", "Unknown name");
}

static void testInteriorLayoutAirlockExteriorConnection() {
    std::cout << "\n=== Interior Layout Airlock Exterior Connection ===" << std::endl;
    ecs::World world;
    systems::ShipInteriorLayoutSystem sys(&world);

    sys.generateLayout("ship_1", "frigate");
    auto* layout = sys.getLayout("ship_1");
    assertTrue(layout != nullptr, "Layout exists");

    // Should have at least one connection to "exterior"
    bool hasExterior = false;
    for (const auto& c : layout->connections) {
        if (c.to_room_id == "exterior" || c.from_room_id == "exterior") {
            hasExterior = true;
            break;
        }
    }
    assertTrue(hasExterior, "Airlock has exterior connection");
}

static void testInteriorLayoutComponentDefaults() {
    std::cout << "\n=== Interior Layout Component Defaults ===" << std::endl;
    components::ShipInteriorLayout layout;
    assertTrue(layout.rooms.empty(), "Default empty rooms");
    assertTrue(layout.connections.empty(), "Default empty connections");
    assertTrue(layout.roomCount() == 0, "Default room count 0");
    assertTrue(layout.connectionCount() == 0, "Default connection count 0");
    assertTrue(!layout.hasRoom("any"), "Default has no rooms");
    assertTrue(layout.getRoom("any") == nullptr, "Default getRoom null");
}


void run_ship_interior_layout_system_tests() {
    testInteriorLayoutGenerateFrigate();
    testInteriorLayoutGenerateBattleship();
    testInteriorLayoutGenerateCapital();
    testInteriorLayoutDuplicate();
    testInteriorLayoutFindBridge();
    testInteriorLayoutFindEngineering();
    testInteriorLayoutConnections();
    testInteriorLayoutAdjacentRooms();
    testInteriorLayoutUnknownShip();
    testInteriorLayoutRoomTypeName();
    testInteriorLayoutAirlockExteriorConnection();
    testInteriorLayoutComponentDefaults();
}
