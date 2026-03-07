// Tests for: Fuel Consumption System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/fuel_consumption_system.h"

using namespace atlas;

// ==================== Fuel Consumption System Tests ====================

static void testFuelConsumptionCreate() {
    std::cout << "\n=== FuelConsumption: Create ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1", "frigate_01", 200.0), "Init succeeds");
    assertTrue(sys.getCurrentFuel("ship1") == 200.0, "Full fuel");
    assertTrue(sys.getMaxFuel("ship1") == 200.0, "Max fuel is 200");
    assertTrue(sys.getFuelPercentage("ship1") == 100.0, "100% fuel");
    assertTrue(sys.getFuelType("ship1") == "Standard", "Default Standard fuel");
    assertTrue(sys.canWarp("ship1"), "Can warp with full tank");
    assertTrue(sys.getTotalFuelConsumed("ship1") == 0.0, "No fuel consumed");
    assertTrue(sys.getTotalFuelPurchased("ship1") == 0.0, "No fuel purchased");
    assertTrue(sys.getRefuelCount("ship1") == 0, "No refuels");
}

static void testFuelConsumptionIdleDrain() {
    std::cout << "\n=== FuelConsumption: IdleDrain ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", 100.0);
    sys.setConsumptionRates("ship1", 5.0, 0.5, 0.1);

    // Idle: 0.1 * 10 = 1.0 consumed -> 99.0 remaining
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getCurrentFuel("ship1"), 99.0), "Idle drain after 10s");
    assertTrue(sys.getTotalFuelConsumed("ship1") > 0.0, "Some fuel consumed");
}

static void testFuelConsumptionWarpDrain() {
    std::cout << "\n=== FuelConsumption: WarpDrain ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", 100.0);
    sys.setConsumptionRates("ship1", 5.0, 0.5, 0.0);
    sys.setWarpState("ship1", true);

    // Warp: 5.0 * 10 = 50.0 consumed -> 50.0 remaining
    sys.update(10.0f);
    assertTrue(approxEqual(sys.getCurrentFuel("ship1"), 50.0), "Half fuel after 10s warp");
    assertTrue(approxEqual(sys.getFuelPercentage("ship1"), 50.0), "50% fuel");
}

static void testFuelConsumptionThrustDrain() {
    std::cout << "\n=== FuelConsumption: ThrustDrain ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", 100.0);
    sys.setConsumptionRates("ship1", 5.0, 1.0, 0.0);
    sys.setThrustState("ship1", true);

    // Thrust: 1.0 * 20 = 20.0 consumed -> 80.0 remaining
    sys.update(20.0f);
    assertTrue(approxEqual(sys.getCurrentFuel("ship1"), 80.0), "80 fuel after 20s thrust");
}

static void testFuelConsumptionEmptyTank() {
    std::cout << "\n=== FuelConsumption: EmptyTank ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", 10.0);
    sys.setConsumptionRates("ship1", 5.0, 0.5, 0.0);
    sys.setWarpState("ship1", true);

    // Warp: 5.0 * 10 = 50.0 needed, but only 10.0 available
    sys.update(10.0f);
    assertTrue(sys.getCurrentFuel("ship1") == 0.0, "Tank empty");
    assertTrue(!sys.canWarp("ship1"), "Cannot warp with empty tank");
}

static void testFuelConsumptionRefuel() {
    std::cout << "\n=== FuelConsumption: Refuel ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", 100.0);
    sys.setConsumptionRates("ship1", 5.0, 0.5, 0.0);
    sys.setWarpState("ship1", true);
    sys.update(10.0f); // Consume 50

    assertTrue(sys.refuel("ship1", 30.0), "Refuel 30 units");
    assertTrue(approxEqual(sys.getCurrentFuel("ship1"), 80.0), "80 fuel after refuel");
    assertTrue(sys.getTotalFuelPurchased("ship1") == 30.0, "30 purchased");
    assertTrue(sys.getRefuelCount("ship1") == 1, "1 refuel");

    // Refuel beyond max capacity
    assertTrue(sys.refuel("ship1", 50.0), "Refuel 50 but capped");
    assertTrue(approxEqual(sys.getCurrentFuel("ship1"), 100.0), "Capped at max");
    assertTrue(sys.getTotalFuelPurchased("ship1") == 50.0, "Only 20 actually added");
}

static void testFuelConsumptionRefuelInvalid() {
    std::cout << "\n=== FuelConsumption: RefuelInvalid ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", 100.0);
    assertTrue(!sys.refuel("ship1", 0.0), "Cannot refuel 0");
    assertTrue(!sys.refuel("ship1", -10.0), "Cannot refuel negative");
}

static void testFuelConsumptionFuelType() {
    std::cout << "\n=== FuelConsumption: FuelType ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1", "frigate_01", 100.0);

    assertTrue(sys.setFuelType("ship1", "HighGrade"), "Set HighGrade fuel");
    assertTrue(sys.getFuelType("ship1") == "HighGrade", "Fuel type is HighGrade");

    assertTrue(sys.setFuelType("ship1", "Experimental"), "Set Experimental fuel");
    assertTrue(sys.getFuelType("ship1") == "Experimental", "Fuel type is Experimental");
}

static void testFuelConsumptionMissing() {
    std::cout << "\n=== FuelConsumption: Missing ===" << std::endl;
    ecs::World world;
    systems::FuelConsumptionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "ship", 100.0), "Init fails on missing");
    assertTrue(!sys.setFuelType("nonexistent", "HighGrade"), "SetFuelType fails on missing");
    assertTrue(!sys.setWarpState("nonexistent", true), "SetWarpState fails on missing");
    assertTrue(!sys.setThrustState("nonexistent", true), "SetThrustState fails on missing");
    assertTrue(!sys.refuel("nonexistent", 50.0), "Refuel fails on missing");
    assertTrue(!sys.setConsumptionRates("nonexistent", 1.0, 1.0, 1.0), "SetRates fails on missing");
    assertTrue(sys.getCurrentFuel("nonexistent") == 0.0, "0 fuel on missing");
    assertTrue(sys.getMaxFuel("nonexistent") == 0.0, "0 max fuel on missing");
    assertTrue(sys.getFuelPercentage("nonexistent") == 0.0, "0% on missing");
    assertTrue(sys.getFuelType("nonexistent") == "Unknown", "Unknown type on missing");
    assertTrue(!sys.canWarp("nonexistent"), "Cannot warp on missing");
    assertTrue(sys.getTotalFuelConsumed("nonexistent") == 0.0, "0 consumed on missing");
    assertTrue(sys.getTotalFuelPurchased("nonexistent") == 0.0, "0 purchased on missing");
    assertTrue(sys.getRefuelCount("nonexistent") == 0, "0 refuels on missing");
}


void run_fuel_consumption_system_tests() {
    testFuelConsumptionCreate();
    testFuelConsumptionIdleDrain();
    testFuelConsumptionWarpDrain();
    testFuelConsumptionThrustDrain();
    testFuelConsumptionEmptyTank();
    testFuelConsumptionRefuel();
    testFuelConsumptionRefuelInvalid();
    testFuelConsumptionFuelType();
    testFuelConsumptionMissing();
}
