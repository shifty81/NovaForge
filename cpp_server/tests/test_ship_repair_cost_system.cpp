// Tests for: Ship Repair Cost System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/ship_repair_cost_system.h"

using namespace atlas;

// ==================== Ship Repair Cost System Tests ====================

static void testShipRepairCostCreate() {
    std::cout << "\n=== ShipRepairCost: Create ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    assertTrue(sys.initialize("ship1"), "Init succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getRepairCost("ship1")), 0.0f), "No repair cost initially");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscSpent("ship1")), 0.0f), "No ISC spent initially");
    assertTrue(sys.getDamageRecordCount("ship1") == 0, "No damage records");
    assertTrue(sys.getTotalRepairsCompleted("ship1") == 0, "No repairs completed");
    assertTrue(sys.getTotalDamageEvents("ship1") == 0, "No damage events");
}

static void testShipRepairCostRecordDamage() {
    std::cout << "\n=== ShipRepairCost: RecordDamage ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    assertTrue(sys.recordDamage("ship1", "npc1", 100.0f, 50.0f, 10.0f), "Record damage");
    assertTrue(sys.getDamageRecordCount("ship1") == 1, "1 damage record");
    assertTrue(sys.getTotalDamageEvents("ship1") == 1, "1 damage event");
    // Cost: 100*1 + 50*3 + 10*10 = 100+150+100 = 350
    assertTrue(approxEqual(static_cast<float>(sys.getRepairCost("ship1")), 350.0f), "Repair cost 350");
}

static void testShipRepairCostMultipleDamage() {
    std::cout << "\n=== ShipRepairCost: MultipleDamage ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.recordDamage("ship1", "npc1", 100.0f, 0.0f, 0.0f);   // 100*1 = 100
    sys.recordDamage("ship1", "npc2", 0.0f, 200.0f, 0.0f);   // 200*3 = 600
    sys.recordDamage("ship1", "npc3", 0.0f, 0.0f, 50.0f);    // 50*10 = 500
    assertTrue(sys.getDamageRecordCount("ship1") == 3, "3 damage records");
    assertTrue(sys.getTotalDamageEvents("ship1") == 3, "3 damage events");
    // Total: 100+600+500 = 1200
    assertTrue(approxEqual(static_cast<float>(sys.getRepairCost("ship1")), 1200.0f), "Repair cost 1200");
}

static void testShipRepairCostApplyRepairDocked() {
    std::cout << "\n=== ShipRepairCost: ApplyRepairDocked ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.recordDamage("ship1", "npc1", 100.0f, 50.0f, 10.0f);
    sys.setDocked("ship1", true);
    assertTrue(sys.applyRepair("ship1"), "Repair succeeds when docked");
    assertTrue(approxEqual(static_cast<float>(sys.getRepairCost("ship1")), 0.0f), "Repair cost reset to 0");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscSpent("ship1")), 350.0f), "ISC spent is 350");
    assertTrue(sys.getDamageRecordCount("ship1") == 0, "Records cleared after repair");
    assertTrue(sys.getTotalRepairsCompleted("ship1") == 1, "1 repair completed");
}

static void testShipRepairCostNotDockedCannotRepair() {
    std::cout << "\n=== ShipRepairCost: NotDockedCannotRepair ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.recordDamage("ship1", "npc1", 100.0f, 0.0f, 0.0f);
    assertTrue(!sys.applyRepair("ship1"), "Repair fails when not docked");
    assertTrue(approxEqual(static_cast<float>(sys.getRepairCost("ship1")), 100.0f), "Cost unchanged");
}

static void testShipRepairCostDiscount() {
    std::cout << "\n=== ShipRepairCost: Discount ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.recordDamage("ship1", "npc1", 0.0f, 0.0f, 100.0f); // 100*10 = 1000
    sys.setDiscount("ship1", 0.5f); // 50% discount
    sys.setDocked("ship1", true);
    sys.applyRepair("ship1");
    // Cost: 1000 - 1000*0.5 = 500
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscSpent("ship1")), 500.0f), "ISC spent 500 with discount");
}

static void testShipRepairCostCustomRates() {
    std::cout << "\n=== ShipRepairCost: CustomRates ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setCostRates("ship1", 2.0f, 5.0f, 20.0f);
    sys.recordDamage("ship1", "npc1", 100.0f, 100.0f, 100.0f);
    // Cost: 100*2 + 100*5 + 100*20 = 200+500+2000 = 2700
    assertTrue(approxEqual(static_cast<float>(sys.getRepairCost("ship1")), 2700.0f), "Custom rate cost 2700");
}

static void testShipRepairCostMaxRecords() {
    std::cout << "\n=== ShipRepairCost: MaxRecords ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");

    auto* entity = world.getEntity("ship1");
    auto* comp = entity->getComponent<components::ShipRepairCost>();
    comp->max_records = 3;

    sys.recordDamage("ship1", "npc1", 10.0f, 0.0f, 0.0f);
    sys.recordDamage("ship1", "npc2", 10.0f, 0.0f, 0.0f);
    sys.recordDamage("ship1", "npc3", 10.0f, 0.0f, 0.0f);
    sys.recordDamage("ship1", "npc4", 10.0f, 0.0f, 0.0f); // evicts oldest
    assertTrue(sys.getDamageRecordCount("ship1") == 3, "Max 3 records enforced");
    assertTrue(sys.getTotalDamageEvents("ship1") == 4, "4 total damage events");
}

static void testShipRepairCostNoRepairWhenZero() {
    std::cout << "\n=== ShipRepairCost: NoRepairWhenZero ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    world.createEntity("ship1");
    sys.initialize("ship1");
    sys.setDocked("ship1", true);
    assertTrue(!sys.applyRepair("ship1"), "No repair needed when cost is 0");
}

static void testShipRepairCostMissing() {
    std::cout << "\n=== ShipRepairCost: Missing ===" << std::endl;
    ecs::World world;
    systems::ShipRepairCostSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.recordDamage("nonexistent", "src", 10.0f, 10.0f, 10.0f), "Record fails on missing");
    assertTrue(!sys.applyRepair("nonexistent"), "Repair fails on missing");
    assertTrue(!sys.setDocked("nonexistent", true), "SetDocked fails on missing");
    assertTrue(!sys.setDiscount("nonexistent", 0.5f), "SetDiscount fails on missing");
    assertTrue(!sys.setCostRates("nonexistent", 1.0f, 2.0f, 3.0f), "SetCostRates fails on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getRepairCost("nonexistent")), 0.0f), "0 cost on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalIscSpent("nonexistent")), 0.0f), "0 ISC on missing");
    assertTrue(sys.getDamageRecordCount("nonexistent") == 0, "0 records on missing");
    assertTrue(sys.getTotalRepairsCompleted("nonexistent") == 0, "0 repairs on missing");
    assertTrue(sys.getTotalDamageEvents("nonexistent") == 0, "0 events on missing");
}


void run_ship_repair_cost_system_tests() {
    testShipRepairCostCreate();
    testShipRepairCostRecordDamage();
    testShipRepairCostMultipleDamage();
    testShipRepairCostApplyRepairDocked();
    testShipRepairCostNotDockedCannotRepair();
    testShipRepairCostDiscount();
    testShipRepairCostCustomRates();
    testShipRepairCostMaxRecords();
    testShipRepairCostNoRepairWhenZero();
    testShipRepairCostMissing();
}
