// Tests for: Phase 11: Fleet Cargo Ship Loss Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/navigation_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/fleet_cargo_system.h"

using namespace atlas;

// ==================== Phase 11: Fleet Cargo Ship Loss Tests ====================

static void testFleetCargoShipLoss() {
    std::cout << "\n=== Fleet Cargo Ship Loss ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship1 = world.createEntity("ship1");
    auto* inv1 = addComp<components::Inventory>(ship1);
    inv1->max_capacity = 400.0f;
    auto* ship2 = world.createEntity("ship2");
    auto* inv2 = addComp<components::Inventory>(ship2);
    inv2->max_capacity = 300.0f;
    sys.addContributor("pool1", "ship1");
    sys.addContributor("pool1", "ship2");
    assertTrue(sys.getTotalCapacity("pool1") == 700, "Total capacity before loss is 700");
    uint64_t lost = sys.handleShipLoss("pool1", "ship1");
    assertTrue(lost == 400, "Lost capacity is 400");
    assertTrue(sys.getTotalCapacity("pool1") == 300, "Total capacity after loss is 300");
}

static void testFleetCargoScaledCapacity() {
    std::cout << "\n=== Fleet Cargo Scaled Capacity ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 1000.0f;
    sys.addContributor("pool1", "ship1");
    // logistics=1.2, skill=1.1, morale=1.0 → 1000 * 1.32 = 1320
    uint64_t scaled = sys.getScaledCapacity("pool1", 1.2f, 1.1f, 1.0f);
    assertTrue(scaled == 1320, "Scaled capacity is 1320 (1000*1.2*1.1*1.0)");
}

static void testFleetCargoScaledWithMoralePenalty() {
    std::cout << "\n=== Fleet Cargo Scaled With Morale Penalty ===" << std::endl;
    ecs::World world;
    systems::FleetCargoSystem sys(&world);
    world.createEntity("pool1");
    auto* ship = world.createEntity("ship1");
    auto* inv = addComp<components::Inventory>(ship);
    inv->max_capacity = 1000.0f;
    sys.addContributor("pool1", "ship1");
    // logistics=1.0, skill=1.0, morale=0.75 → 1000 * 0.75 = 750
    uint64_t scaled = sys.getScaledCapacity("pool1", 1.0f, 1.0f, 0.75f);
    assertTrue(scaled == 750, "Scaled capacity penalized by low morale (750)");
}


void run_phase_11_fleet_cargo_ship_loss_tests() {
    testFleetCargoShipLoss();
    testFleetCargoScaledCapacity();
    testFleetCargoScaledWithMoralePenalty();
}
