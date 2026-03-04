// Tests for: Ship Fitting System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/ship_fitting_system.h"

using namespace atlas;

// ==================== Ship Fitting System Tests ====================

static void testShipFittingFitModule() {
    std::cout << "\n=== Ship Fitting: Fit Module ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    bool ok = fittingSys.fitModule("ship1", "mod1", "Light Autocannon", "high", 10.0f, 5.0f);
    assertTrue(ok, "Fitting a high-slot module succeeds");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 1, "One high-slot module fitted");
}

static void testShipFittingSlotLimit() {
    std::cout << "\n=== Ship Fitting: Slot Limit ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 500.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    // Frigate has 3 high slots
    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 5.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 5.0f, 5.0f);
    fittingSys.fitModule("ship1", "h3", "Gun 3", "high", 5.0f, 5.0f);
    bool ok = fittingSys.fitModule("ship1", "h4", "Gun 4", "high", 5.0f, 5.0f);
    assertTrue(!ok, "Cannot fit more than 3 high-slot modules on Frigate");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 3, "Still 3 high-slot modules");
}

static void testShipFittingCPUOverflow() {
    std::cout << "\n=== Ship Fitting: CPU Overflow ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Cruiser";
    ship->cpu_max = 30.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 15.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 15.0f, 5.0f);
    bool ok = fittingSys.fitModule("ship1", "h3", "Gun 3", "high", 15.0f, 5.0f);
    assertTrue(!ok, "Cannot exceed CPU budget");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 2, "Still 2 modules");
}

static void testShipFittingRemoveModule() {
    std::cout << "\n=== Ship Fitting: Remove Module ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 10.0f, 5.0f);
    fittingSys.fitModule("ship1", "h2", "Gun 2", "high", 10.0f, 5.0f);
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 2, "2 modules before remove");

    bool ok = fittingSys.removeModule("ship1", "high", 0);
    assertTrue(ok, "Remove succeeds");
    assertTrue(fittingSys.getFittedCount("ship1", "high") == 1, "1 module after remove");
}

static void testShipFittingValidate() {
    std::cout << "\n=== Ship Fitting: Validate ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Frigate";
    ship->cpu_max = 100.0f;
    ship->powergrid_max = 50.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "h1", "Gun 1", "high", 10.0f, 5.0f);
    assertTrue(fittingSys.validateFitting("ship1"), "Valid fitting within budget");
}

static void testShipFittingSlotCapacity() {
    std::cout << "\n=== Ship Fitting: Slot Capacity Lookup ===" << std::endl;
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "high") == 3,
               "Frigate has 3 high slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "mid") == 3,
               "Frigate has 3 mid slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Frigate", "low") == 2,
               "Frigate has 2 low slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Battleship", "high") == 7,
               "Battleship has 7 high slots");
    assertTrue(systems::ShipFittingSystem::getSlotCapacity("Cruiser", "mid") == 4,
               "Cruiser has 4 mid slots");
}

static void testShipFittingMidLowSlots() {
    std::cout << "\n=== Ship Fitting: Mid and Low Slots ===" << std::endl;
    ecs::World world;
    systems::ShipFittingSystem fittingSys(&world);

    auto* ship_entity = world.createEntity("ship1");
    auto* ship = addComp<components::Ship>(ship_entity);
    ship->ship_class = "Cruiser";
    ship->cpu_max = 500.0f;
    ship->powergrid_max = 500.0f;
    addComp<components::ModuleRack>(ship_entity);

    fittingSys.fitModule("ship1", "m1", "Shield Booster", "mid", 10.0f, 10.0f);
    fittingSys.fitModule("ship1", "l1", "Armor Plate", "low", 10.0f, 10.0f);
    assertTrue(fittingSys.getFittedCount("ship1", "mid") == 1, "1 mid-slot module fitted");
    assertTrue(fittingSys.getFittedCount("ship1", "low") == 1, "1 low-slot module fitted");
}


void run_ship_fitting_system_tests() {
    testShipFittingFitModule();
    testShipFittingSlotLimit();
    testShipFittingCPUOverflow();
    testShipFittingRemoveModule();
    testShipFittingValidate();
    testShipFittingSlotCapacity();
    testShipFittingMidLowSlots();
}
