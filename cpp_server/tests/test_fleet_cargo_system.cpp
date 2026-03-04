// Tests for: FleetCargoSystem Tests
#include "test_log.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/fleet_cargo_system.h"

using namespace atlas;

// ==================== FleetCargoSystem Tests ====================

static void testFleetCargoAddContributor() {
    std::cout << "\n=== Fleet Cargo Add Contributor ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    sys.addContributor("pool1", "ship1");
    sys.recalculate("pool1");
    assertTrue(sys.getTotalCapacity("pool1") == 400, "Total capacity is 400 after adding ship");
}

static void testFleetCargoRemoveContributor() {
    std::cout << "\n=== Fleet Cargo Remove Contributor ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    sys.addContributor("pool1", "ship1");
    sys.removeContributor("pool1", "ship1");
    assertTrue(sys.getTotalCapacity("pool1") == 0, "Total capacity 0 after removing ship");
}

static void testFleetCargoMultipleShips() {
    std::cout << "\n=== Fleet Cargo Multiple Ships ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    for (int i = 0; i < 3; i++) {
        std::string sid = "ship" + std::to_string(i);
        auto* ship = world.createEntity(sid);
        auto* inv = addComp<components::Inventory>(ship);
        inv->max_capacity = 200.0f;
        sys.addContributor("pool1", sid);
    }
    sys.recalculate("pool1");
    assertTrue(sys.getTotalCapacity("pool1") == 600, "Aggregate capacity of 3 ships is 600");
}

static void testFleetCargoUsedCapacity() {
    std::cout << "\n=== Fleet Cargo Used Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 400.0f;
    components::Inventory::Item item;
    item.item_id = "ore1";
    item.name = "Ferrite";
    item.type = "ore";
    item.quantity = 10;
    item.volume = 5.0f;
    inv->items.push_back(item);
    sys.addContributor("pool1", "ship1");
    sys.recalculate("pool1");
    assertTrue(sys.getUsedCapacity("pool1") == 50, "Used capacity reflects items (10*5=50)");
}

static void testFleetCargoGetCapacity() {
    std::cout << "\n=== Fleet Cargo Get Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 300.0f;
    sys.addContributor("pool1", "ship1");
    assertTrue(sys.getTotalCapacity("pool1") == 300, "getTotalCapacity query returns 300");
}


void run_fleet_cargo_system_tests() {
    testFleetCargoAddContributor();
    testFleetCargoRemoveContributor();
    testFleetCargoMultipleShips();
    testFleetCargoUsedCapacity();
    testFleetCargoGetCapacity();
}
