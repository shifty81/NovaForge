// Tests for: FuelBlockSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/fuel_block_system.h"

using namespace atlas;

// ==================== FuelBlockSystem Tests ====================

static void testFuelBlockInit() {
    std::cout << "\n=== FuelBlock: Init ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    assertTrue(sys.initialize("e1"), "Init succeeds");
    assertTrue(sys.getReserveCount("e1") == 0, "Zero reserves initially");
    assertTrue(sys.isOnline("e1"), "Online initially");
    assertTrue(!sys.isLowFuel("e1"), "No low-fuel warning initially");
    assertTrue(sys.getTotalRefuels("e1") == 0, "Zero refuels");
    assertTrue(approxEqual(sys.getTotalFuelConsumed("e1"), 0.0f), "Zero consumed");
    assertTrue(sys.getTotalOfflineEvents("e1") == 0, "Zero offline events");
    assertTrue(sys.getStructureId("e1") == "", "No structure ID");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testFuelBlockAddReserve() {
    std::cout << "\n=== FuelBlock: AddReserve ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    assertTrue(sys.addReserve("e1", "fuel_std", FT::Standard, 500.0f, 1000.0f, 1.0f),
               "Add standard reserve");
    assertTrue(sys.getReserveCount("e1") == 1, "1 reserve");
    assertTrue(sys.hasReserve("e1", "fuel_std"), "Has fuel_std");
    assertTrue(approxEqual(sys.getFuelQuantity("e1", "fuel_std"), 500.0f),
               "Quantity is 500");
    assertTrue(approxEqual(sys.getFuelCapacity("e1", "fuel_std"), 1000.0f),
               "Capacity is 1000");
    assertTrue(approxEqual(sys.getConsumptionRate("e1", "fuel_std"), 1.0f),
               "Rate is 1.0");

    assertTrue(sys.addReserve("e1", "fuel_n2", FT::Nitrogen, 200.0f, 500.0f, 0.5f),
               "Add nitrogen reserve");
    assertTrue(sys.getReserveCount("e1") == 2, "2 reserves");
}

static void testFuelBlockAddReserveValidation() {
    std::cout << "\n=== FuelBlock: AddReserveValidation ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    assertTrue(!sys.addReserve("e1", "", FT::Standard, 100, 200, 1),
               "Empty reserve_id rejected");
    assertTrue(!sys.addReserve("e1", "r1", FT::Standard, 100, 0, 1),
               "Zero max_quantity rejected");
    assertTrue(!sys.addReserve("e1", "r1", FT::Standard, 100, -1, 1),
               "Negative max_quantity rejected");
    assertTrue(!sys.addReserve("e1", "r1", FT::Standard, -1, 200, 1),
               "Negative quantity rejected");
    assertTrue(!sys.addReserve("e1", "r1", FT::Standard, 300, 200, 1),
               "Quantity > max rejected");
    assertTrue(!sys.addReserve("e1", "r1", FT::Standard, 100, 200, -1),
               "Negative rate rejected");

    assertTrue(sys.addReserve("e1", "r1", FT::Standard, 100, 200, 0.0f),
               "Zero consumption rate allowed");
    assertTrue(!sys.addReserve("e1", "r1", FT::Helium, 50, 100, 0.5f),
               "Duplicate reserve_id rejected");
    assertTrue(!sys.addReserve("missing", "r9", FT::Standard, 50, 100, 1),
               "Missing entity rejected");
}

static void testFuelBlockCapacity() {
    std::cout << "\n=== FuelBlock: Capacity ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setMaxReserves("e1", 2);

    using FT = components::FuelBlockState::FuelType;
    assertTrue(sys.addReserve("e1", "r1", FT::Standard, 100, 200, 1), "Add 1");
    assertTrue(sys.addReserve("e1", "r2", FT::Nitrogen, 100, 200, 1), "Add 2");
    assertTrue(!sys.addReserve("e1", "r3", FT::Helium, 100, 200, 1),
               "Add 3 rejected at capacity");
    assertTrue(sys.getReserveCount("e1") == 2, "Still 2");
}

static void testFuelBlockConsumption() {
    std::cout << "\n=== FuelBlock: Consumption ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    // 100 units, consuming 10/sec → empties in 10 sec
    sys.addReserve("e1", "r1", FT::Standard, 100.0f, 1000.0f, 10.0f);

    sys.update(5.0f);
    assertTrue(approxEqual(sys.getFuelQuantity("e1", "r1"), 50.0f), "50 after 5s");
    assertTrue(sys.isOnline("e1"), "Still online");

    sys.update(5.0f);
    assertTrue(approxEqual(sys.getFuelQuantity("e1", "r1"), 0.0f), "0 after 10s");
    assertTrue(!sys.isOnline("e1"), "Offline when empty");
    assertTrue(sys.getTotalOfflineEvents("e1") == 1, "1 offline event");
    assertTrue(approxEqual(sys.getTotalFuelConsumed("e1"), 100.0f), "Consumed 100");
}

static void testFuelBlockLowFuelWarning() {
    std::cout << "\n=== FuelBlock: LowFuelWarning ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");
    sys.setLowFuelThreshold("e1", 60.0f); // warn if < 60s remaining

    using FT = components::FuelBlockState::FuelType;
    // 100 units at 1/sec → 100s remaining
    sys.addReserve("e1", "r1", FT::Standard, 100.0f, 1000.0f, 1.0f);

    sys.update(1.0f);
    assertTrue(!sys.isLowFuel("e1"), "No warning at 99s remaining");

    // Consume to 50s remaining
    sys.update(49.0f);
    assertTrue(sys.isLowFuel("e1"), "Warning at 50s remaining (< 60s threshold)");
    assertTrue(sys.isOnline("e1"), "Still online");
}

static void testFuelBlockRefuel() {
    std::cout << "\n=== FuelBlock: Refuel ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    sys.addReserve("e1", "r1", FT::Standard, 100.0f, 500.0f, 1.0f);

    assertTrue(sys.refuel("e1", "r1", 200.0f), "Refuel 200");
    assertTrue(approxEqual(sys.getFuelQuantity("e1", "r1"), 300.0f), "Now 300");
    assertTrue(sys.getTotalRefuels("e1") == 1, "1 refuel");

    // Refuel beyond capacity — capped at max
    assertTrue(sys.refuel("e1", "r1", 999.0f), "Refuel capped at max");
    assertTrue(approxEqual(sys.getFuelQuantity("e1", "r1"), 500.0f), "Capped at 500");
    assertTrue(sys.getTotalRefuels("e1") == 2, "2 refuels");

    // Invalid refuel
    assertTrue(!sys.refuel("e1", "r1", 0.0f), "Zero amount rejected");
    assertTrue(!sys.refuel("e1", "r1", -10.0f), "Negative amount rejected");
    assertTrue(!sys.refuel("e1", "unknown", 50.0f), "Unknown reserve rejected");
}

static void testFuelBlockBringOnline() {
    std::cout << "\n=== FuelBlock: BringOnline ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    sys.addReserve("e1", "r1", FT::Standard, 10.0f, 100.0f, 10.0f);

    // Already online — fails
    assertTrue(!sys.bringOnline("e1"), "Cannot bring online when already online");

    // Drain it to trigger offline
    sys.update(1.0f); // 10 - 10 = 0
    assertTrue(!sys.isOnline("e1"), "Offline");

    // Cannot bring online with empty reserve
    assertTrue(!sys.bringOnline("e1"), "Cannot go online with empty fuel");

    // Refuel and bring online
    sys.refuel("e1", "r1", 50.0f);
    assertTrue(sys.bringOnline("e1"), "Bring online after refuel");
    assertTrue(sys.isOnline("e1"), "Now online");
}

static void testFuelBlockRemoveReserve() {
    std::cout << "\n=== FuelBlock: RemoveReserve ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    sys.addReserve("e1", "r1", FT::Standard, 100, 200, 1);
    sys.addReserve("e1", "r2", FT::Nitrogen, 100, 200, 1);

    assertTrue(sys.removeReserve("e1", "r1"), "Remove r1");
    assertTrue(sys.getReserveCount("e1") == 1, "1 left");
    assertTrue(!sys.hasReserve("e1", "r1"), "r1 gone");
    assertTrue(sys.hasReserve("e1", "r2"), "r2 present");
    assertTrue(!sys.removeReserve("e1", "r1"), "Remove already removed fails");
    assertTrue(!sys.removeReserve("e1", "unknown"), "Remove unknown fails");
}

static void testFuelBlockClearReserves() {
    std::cout << "\n=== FuelBlock: ClearReserves ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    sys.addReserve("e1", "r1", FT::Standard, 100, 200, 1);
    sys.addReserve("e1", "r2", FT::Nitrogen, 100, 200, 1);

    assertTrue(sys.clearReserves("e1"), "ClearReserves succeeds");
    assertTrue(sys.getReserveCount("e1") == 0, "0 after clear");
}

static void testFuelBlockConfiguration() {
    std::cout << "\n=== FuelBlock: Configuration ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    sys.addReserve("e1", "r1", FT::Standard, 100, 200, 1);

    assertTrue(sys.setStructureId("e1", "citadel_01"), "Set structure ID");
    assertTrue(sys.getStructureId("e1") == "citadel_01", "Structure ID matches");

    assertTrue(sys.setLowFuelThreshold("e1", 7200.0f), "Set threshold");
    assertTrue(!sys.setLowFuelThreshold("e1", -1.0f), "Negative threshold rejected");
    assertTrue(sys.setLowFuelThreshold("e1", 0.0f), "Zero threshold allowed");

    assertTrue(sys.setMaxReserves("e1", 10), "Set max reserves");
    assertTrue(!sys.setMaxReserves("e1", 0), "Zero max rejected");
    assertTrue(!sys.setMaxReserves("e1", -1), "Negative max rejected");

    assertTrue(sys.setConsumptionRate("e1", "r1", 2.0f), "Set consumption rate");
    assertTrue(approxEqual(sys.getConsumptionRate("e1", "r1"), 2.0f), "Rate is 2.0");
    assertTrue(!sys.setConsumptionRate("e1", "r1", -1.0f), "Negative rate rejected");
    assertTrue(sys.setConsumptionRate("e1", "r1", 0.0f), "Zero rate allowed");
    assertTrue(!sys.setConsumptionRate("e1", "unknown", 1.0f), "Unknown reserve rejected");
}

static void testFuelBlockTimeUntilEmpty() {
    std::cout << "\n=== FuelBlock: TimeUntilEmpty ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);
    world.createEntity("e1");
    sys.initialize("e1");

    using FT = components::FuelBlockState::FuelType;
    sys.addReserve("e1", "r1", FT::Standard, 100.0f, 200.0f, 2.0f);
    sys.addReserve("e1", "r2", FT::Nitrogen, 50.0f, 100.0f, 0.0f); // no consumption

    assertTrue(approxEqual(sys.getTimeUntilEmpty("e1", "r1"), 50.0f), "50s until empty");
    assertTrue(approxEqual(sys.getTimeUntilEmpty("e1", "r2"), -1.0f), "Infinite for 0 rate");
    assertTrue(approxEqual(sys.getTimeUntilEmpty("e1", "unknown"), 0.0f), "0 for unknown");
}

static void testFuelBlockMissing() {
    std::cout << "\n=== FuelBlock: Missing ===" << std::endl;
    ecs::World world;
    systems::FuelBlockSystem sys(&world);

    using FT = components::FuelBlockState::FuelType;
    assertTrue(!sys.addReserve("none", "r1", FT::Standard, 100, 200, 1),
               "Add fails on missing");
    assertTrue(!sys.removeReserve("none", "r1"), "Remove fails on missing");
    assertTrue(!sys.clearReserves("none"), "Clear fails on missing");
    assertTrue(!sys.refuel("none", "r1", 50), "Refuel fails on missing");
    assertTrue(!sys.setConsumptionRate("none", "r1", 1), "SetRate fails on missing");
    assertTrue(!sys.bringOnline("none"), "BringOnline fails on missing");
    assertTrue(!sys.setStructureId("none", "s1"), "SetStructureId fails on missing");
    assertTrue(!sys.setLowFuelThreshold("none", 60), "SetThreshold fails on missing");
    assertTrue(!sys.setMaxReserves("none", 5), "SetMax fails on missing");
    assertTrue(sys.getReserveCount("none") == 0, "0 count on missing");
    assertTrue(!sys.hasReserve("none", "r1"), "No reserve on missing");
    assertTrue(approxEqual(sys.getFuelQuantity("none", "r1"), 0.0f), "0 qty on missing");
    assertTrue(approxEqual(sys.getFuelCapacity("none", "r1"), 0.0f), "0 cap on missing");
    assertTrue(approxEqual(sys.getConsumptionRate("none", "r1"), 0.0f), "0 rate on missing");
    assertTrue(!sys.isOnline("none"), "Not online on missing");
    assertTrue(!sys.isLowFuel("none"), "Not low-fuel on missing");
    assertTrue(sys.getStructureId("none") == "", "Empty structureId on missing");
    assertTrue(sys.getTotalRefuels("none") == 0, "0 refuels on missing");
    assertTrue(approxEqual(sys.getTotalFuelConsumed("none"), 0.0f), "0 consumed on missing");
    assertTrue(sys.getTotalOfflineEvents("none") == 0, "0 offline on missing");
    assertTrue(approxEqual(sys.getTimeUntilEmpty("none", "r1"), 0.0f), "0 time on missing");
}

void run_fuel_block_system_tests() {
    testFuelBlockInit();
    testFuelBlockAddReserve();
    testFuelBlockAddReserveValidation();
    testFuelBlockCapacity();
    testFuelBlockConsumption();
    testFuelBlockLowFuelWarning();
    testFuelBlockRefuel();
    testFuelBlockBringOnline();
    testFuelBlockRemoveReserve();
    testFuelBlockClearReserves();
    testFuelBlockConfiguration();
    testFuelBlockTimeUntilEmpty();
    testFuelBlockMissing();
}
