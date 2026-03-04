// Tests for: FleetSupplyLine System Tests
#include "test_log.h"
#include "components/fleet_components.h"
#include "ecs/system.h"
#include "systems/fleet_supply_line_system.h"

using namespace atlas;

// ==================== FleetSupplyLine System Tests ====================

static void testSupplyLineCreate() {
    std::cout << "\n=== SupplyLine: Create ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    assertTrue(sys.initializeSupplyLine("fleet1"), "Init supply line succeeds");
    assertTrue(sys.getDepotCount("fleet1") == 0, "No depots initially");
    assertTrue(sys.getTotalResupplies("fleet1") == 0, "No resupplies initially");
}

static void testSupplyLineAddDepot() {
    std::cout << "\n=== SupplyLine: AddDepot ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    assertTrue(sys.addDepot("fleet1", "depot1", "system_alpha", 100.0f), "Add depot succeeds");
    assertTrue(sys.getDepotCount("fleet1") == 1, "1 depot");
    assertTrue(!sys.addDepot("fleet1", "depot1", "system_beta", 50.0f), "Duplicate depot rejected");
}

static void testSupplyLineResupply() {
    std::cout << "\n=== SupplyLine: Resupply ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    assertTrue(sys.resupplyDepot("fleet1", "depot1", 50.0f, 60.0f), "Resupply succeeds");
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 50.0f), "Fuel is 50");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 60.0f), "Ammo is 60");
    assertTrue(sys.getTotalResupplies("fleet1") == 1, "1 resupply counted");
}

static void testSupplyLineResupplyCap() {
    std::cout << "\n=== SupplyLine: ResupplyCap ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 80.0f, 80.0f);
    sys.resupplyDepot("fleet1", "depot1", 50.0f, 50.0f); // should cap at 100
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 100.0f), "Fuel capped at 100");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 100.0f), "Ammo capped at 100");
}

static void testSupplyLineConsume() {
    std::cout << "\n=== SupplyLine: Consume ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 80.0f, 70.0f);
    assertTrue(sys.consumeSupplies("fleet1", "depot1", 30.0f, 20.0f), "Consume succeeds");
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 50.0f), "Fuel is 50 after consume");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 50.0f), "Ammo is 50 after consume");
}

static void testSupplyLineConsumeFloor() {
    std::cout << "\n=== SupplyLine: ConsumeFloor ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 10.0f, 5.0f);
    sys.consumeSupplies("fleet1", "depot1", 50.0f, 50.0f); // consume more than available
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 0.0f), "Fuel floored at 0");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 0.0f), "Ammo floored at 0");
}

static void testSupplyLineCritical() {
    std::cout << "\n=== SupplyLine: Critical ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 15.0f, 50.0f); // fuel < 20
    assertTrue(sys.isDepotCritical("fleet1", "depot1"), "Depot critical (low fuel)");
    sys.resupplyDepot("fleet1", "depot1", 30.0f, 0.0f); // fuel now 45
    assertTrue(!sys.isDepotCritical("fleet1", "depot1"), "Depot not critical (both above 20)");
}

static void testSupplyLineUpdate() {
    std::cout << "\n=== SupplyLine: Update ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.resupplyDepot("fleet1", "depot1", 50.0f, 50.0f);
    sys.update(10.0f); // consumption_rate=1.0, delta=10 → drain 10 each
    assertTrue(approxEqual(sys.getFuelLevel("fleet1", "depot1"), 40.0f), "Fuel after update is 40");
    assertTrue(approxEqual(sys.getAmmoLevel("fleet1", "depot1"), 40.0f), "Ammo after update is 40");
}

static void testSupplyLineRemoveDepot() {
    std::cout << "\n=== SupplyLine: RemoveDepot ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    world.createEntity("fleet1");
    sys.initializeSupplyLine("fleet1");
    sys.addDepot("fleet1", "depot1", "sys1", 100.0f);
    sys.addDepot("fleet1", "depot2", "sys2", 100.0f);
    assertTrue(sys.removeDepot("fleet1", "depot1"), "Remove depot succeeds");
    assertTrue(sys.getDepotCount("fleet1") == 1, "1 depot remains");
    assertTrue(!sys.removeDepot("fleet1", "depot1"), "Remove nonexistent fails");
}

static void testSupplyLineMissing() {
    std::cout << "\n=== SupplyLine: Missing ===" << std::endl;
    ecs::World world;
    systems::FleetSupplyLineSystem sys(&world);
    assertTrue(!sys.initializeSupplyLine("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.addDepot("nonexistent", "d1", "s1", 100), "Add depot fails on missing");
    assertTrue(!sys.removeDepot("nonexistent", "d1"), "Remove fails on missing");
    assertTrue(!sys.resupplyDepot("nonexistent", "d1", 10, 10), "Resupply fails on missing");
    assertTrue(!sys.consumeSupplies("nonexistent", "d1", 10, 10), "Consume fails on missing");
    assertTrue(sys.getDepotCount("nonexistent") == 0, "0 depots on missing");
    assertTrue(approxEqual(sys.getFuelLevel("nonexistent", "d1"), 0.0f), "0 fuel on missing");
    assertTrue(approxEqual(sys.getAmmoLevel("nonexistent", "d1"), 0.0f), "0 ammo on missing");
    assertTrue(sys.getTotalResupplies("nonexistent") == 0, "0 resupplies on missing");
    assertTrue(!sys.isDepotCritical("nonexistent", "d1"), "Not critical on missing");
}


void run_fleet_supply_line_system_tests() {
    testSupplyLineCreate();
    testSupplyLineAddDepot();
    testSupplyLineResupply();
    testSupplyLineResupplyCap();
    testSupplyLineConsume();
    testSupplyLineConsumeFloor();
    testSupplyLineCritical();
    testSupplyLineUpdate();
    testSupplyLineRemoveDepot();
    testSupplyLineMissing();
}
