// Tests for: SupplyDemandSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/supply_demand_system.h"

using namespace atlas;

// ==================== SupplyDemandSystem Tests ====================

static void testSupplyDemandCreate() {
    std::cout << "\n=== SupplyDemand: Create ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    comp->addCommodity("tritanium", 100.0f, 100.0f, 100.0f);
    entity->addComponent(std::move(comp));

    assertTrue(approxEqual(sys.getPrice("system_a", "tritanium"), 100.0f), "Initial price 100");
    assertTrue(approxEqual(sys.getSupply("system_a", "tritanium"), 100.0f), "Initial supply 100");
    assertTrue(approxEqual(sys.getDemand("system_a", "tritanium"), 100.0f), "Initial demand 100");
}

static void testSupplyDemandMissingEntity() {
    std::cout << "\n=== SupplyDemand: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    assertTrue(approxEqual(sys.getPrice("x", "y"), 0.0f), "Price 0 for missing entity");
    assertTrue(approxEqual(sys.getSupply("x", "y"), 0.0f), "Supply 0 for missing entity");
    assertTrue(approxEqual(sys.getDemand("x", "y"), 0.0f), "Demand 0 for missing entity");
}

static void testSupplyDemandMissingCommodity() {
    std::cout << "\n=== SupplyDemand: MissingCommodity ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    comp->addCommodity("tritanium", 100.0f, 100.0f, 100.0f);
    entity->addComponent(std::move(comp));

    assertTrue(approxEqual(sys.getPrice("system_a", "nonexistent"), 0.0f),
               "Price 0 for missing commodity");
    assertTrue(approxEqual(sys.getSupply("system_a", "nonexistent"), 0.0f),
               "Supply 0 for missing commodity");
    assertTrue(approxEqual(sys.getDemand("system_a", "nonexistent"), 0.0f),
               "Demand 0 for missing commodity");
}

static void testSupplyDemandAddSupply() {
    std::cout << "\n=== SupplyDemand: AddSupply ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    comp->addCommodity("tritanium", 100.0f, 50.0f, 100.0f);
    entity->addComponent(std::move(comp));

    sys.addSupply("system_a", "tritanium", 25.0f);
    assertTrue(approxEqual(sys.getSupply("system_a", "tritanium"), 75.0f), "Supply is now 75");

    // Adding to nonexistent commodity is a no-op
    sys.addSupply("system_a", "nonexistent", 10.0f);
    assertTrue(true, "Adding to nonexistent commodity doesn't crash");

    // Adding to nonexistent entity is a no-op
    sys.addSupply("missing", "tritanium", 10.0f);
    assertTrue(true, "Adding to missing entity doesn't crash");
}

static void testSupplyDemandAddDemand() {
    std::cout << "\n=== SupplyDemand: AddDemand ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    comp->addCommodity("pyerite", 50.0f, 100.0f, 80.0f);
    entity->addComponent(std::move(comp));

    sys.addDemand("system_a", "pyerite", 20.0f);
    assertTrue(approxEqual(sys.getDemand("system_a", "pyerite"), 100.0f), "Demand is now 100");

    // Adding to nonexistent is a no-op
    sys.addDemand("system_a", "nonexistent", 10.0f);
    assertTrue(true, "Adding demand to nonexistent commodity doesn't crash");
}

static void testSupplyDemandNPCActivityModifier() {
    std::cout << "\n=== SupplyDemand: NPCActivityModifier ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    comp->addCommodity("mexallon", 200.0f, 100.0f, 100.0f);
    entity->addComponent(std::move(comp));

    sys.setNPCActivityModifier("system_a", 2.0f);

    // After update, NPC supply production should be doubled
    sys.update(1.0f);
    float supply = sys.getSupply("system_a", "mexallon");
    // supply_rate(1.0) * npc_modifier(2.0) * dt(1.0) = 2.0 production
    // supply_decay_rate(0.01) * dt(1.0) = 0.01 decay
    // 100 + 2.0 - 0.01 = ~101.99
    assertTrue(supply > 101.0f, "Higher supply with 2x NPC activity");

    // Missing entity is a no-op
    sys.setNPCActivityModifier("missing", 1.0f);
    assertTrue(true, "Setting modifier on missing entity doesn't crash");
}

static void testSupplyDemandPriceUpdateHighDemand() {
    std::cout << "\n=== SupplyDemand: PriceUpdateHighDemand ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    comp->addCommodity("isogen", 100.0f, 50.0f, 200.0f);
    entity->addComponent(std::move(comp));

    // With demand(200) >> supply(50), price should increase
    sys.update(0.1f);
    float price = sys.getPrice("system_a", "isogen");
    assertTrue(price > 100.0f, "Price increases when demand > supply");
}

static void testSupplyDemandPriceFloorCeiling() {
    std::cout << "\n=== SupplyDemand: PriceFloorCeiling ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    // Extreme supply/demand ratio to test clamping
    comp->addCommodity("nocxium", 100.0f, 10000.0f, 1.0f); // huge supply, tiny demand
    entity->addComponent(std::move(comp));

    sys.update(0.1f);
    float price = sys.getPrice("system_a", "nocxium");
    // Price floor = base(100) * floor_mult(0.2) = 20
    assertTrue(price >= 20.0f, "Price respects floor");

    // Test ceiling with extreme demand
    auto* entity2 = world.createEntity("system_b");
    auto comp2 = std::make_unique<components::SupplyDemand>();
    comp2->addCommodity("zydrine", 100.0f, 1.0f, 10000.0f); // tiny supply, huge demand
    entity2->addComponent(std::move(comp2));

    sys.update(0.1f);
    float price2 = sys.getPrice("system_b", "zydrine");
    // Price ceiling = base(100) * ceiling_mult(5.0) = 500
    assertTrue(price2 <= 500.0f, "Price respects ceiling");
}

static void testSupplyDemandMultipleCommodities() {
    std::cout << "\n=== SupplyDemand: MultipleCommodities ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    comp->addCommodity("tritanium", 100.0f, 100.0f, 100.0f);
    comp->addCommodity("pyerite", 50.0f, 200.0f, 50.0f);
    comp->addCommodity("mexallon", 200.0f, 80.0f, 120.0f);
    entity->addComponent(std::move(comp));

    assertTrue(approxEqual(sys.getPrice("system_a", "tritanium"), 100.0f), "Tritanium initial price");
    assertTrue(approxEqual(sys.getPrice("system_a", "pyerite"), 50.0f), "Pyerite initial price");
    assertTrue(approxEqual(sys.getPrice("system_a", "mexallon"), 200.0f), "Mexallon initial price");

    // Duplicate add is a no-op
    sys.addSupply("system_a", "tritanium", 50.0f);
    assertTrue(approxEqual(sys.getSupply("system_a", "tritanium"), 150.0f), "Tritanium supply 150");
    assertTrue(approxEqual(sys.getSupply("system_a", "pyerite"), 200.0f),
               "Pyerite supply unchanged at 200");
}

static void testSupplyDemandSupplyFloor() {
    std::cout << "\n=== SupplyDemand: SupplyFloor ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    comp->addCommodity("rare_ore", 100.0f, 0.5f, 100.0f);
    comp->supply_decay_rate = 10.0f; // aggressive decay
    entity->addComponent(std::move(comp));

    // With high decay, supply should not go negative
    sys.update(1.0f);
    float supply = sys.getSupply("system_a", "rare_ore");
    assertTrue(supply >= 0.0f, "Supply never goes negative");
}

static void testSupplyDemandUpdate() {
    std::cout << "\n=== SupplyDemand: Update ===" << std::endl;
    ecs::World world;
    systems::SupplyDemandSystem sys(&world);
    auto* entity = world.createEntity("system_a");
    auto comp = std::make_unique<components::SupplyDemand>();
    entity->addComponent(std::move(comp));
    sys.update(1.0f);
    assertTrue(true, "Update tick OK");
}

void run_supply_demand_system_tests() {
    testSupplyDemandCreate();
    testSupplyDemandMissingEntity();
    testSupplyDemandMissingCommodity();
    testSupplyDemandAddSupply();
    testSupplyDemandAddDemand();
    testSupplyDemandNPCActivityModifier();
    testSupplyDemandPriceUpdateHighDemand();
    testSupplyDemandPriceFloorCeiling();
    testSupplyDemandMultipleCommodities();
    testSupplyDemandSupplyFloor();
    testSupplyDemandUpdate();
}
