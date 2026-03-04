// Tests for: EconomicFlowSystem Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/economic_flow_system.h"

using namespace atlas;

// ==================== EconomicFlowSystem Tests ====================

static void testEconomicFlowDefaults() {
    std::cout << "\n=== Economic Flow: Defaults ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("system1");
    addComp<components::EconomicFlowState>(sys_entity);

    systems::EconomicFlowSystem sys(&world);
    assertTrue(approxEqual(sys.getEconomicHealth("system1"), 1.0f), "Default health is 1.0");
    assertTrue(approxEqual(sys.getTotalProduction("system1"), 0.0f), "Default production is 0");
    assertTrue(approxEqual(sys.getTotalConsumption("system1"), 0.0f), "Default consumption is 0");
}

static void testEconomicFlowRecordProduction() {
    std::cout << "\n=== Economic Flow: Record Production ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("system1");
    addComp<components::EconomicFlowState>(sys_entity);

    systems::EconomicFlowSystem sys(&world);
    sys.recordProduction("system1", "ore", 100.0f);
    assertTrue(approxEqual(sys.getProductionRate("system1", "ore"), 100.0f), "Production recorded");
}

static void testEconomicFlowRecordConsumption() {
    std::cout << "\n=== Economic Flow: Record Consumption ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("system1");
    addComp<components::EconomicFlowState>(sys_entity);

    systems::EconomicFlowSystem sys(&world);
    sys.recordConsumption("system1", "fuel", 50.0f);
    assertTrue(approxEqual(sys.getConsumptionRate("system1", "fuel"), 50.0f), "Consumption recorded");
}

static void testEconomicFlowTransport() {
    std::cout << "\n=== Economic Flow: Transport ===" << std::endl;
    ecs::World world;
    auto* s1 = world.createEntity("system1");
    addComp<components::EconomicFlowState>(s1);
    auto* s2 = world.createEntity("system2");
    addComp<components::EconomicFlowState>(s2);

    systems::EconomicFlowSystem sys(&world);
    sys.recordTransport("system1", "system2", "ore", 30.0f);

    auto* flow1 = s1->getComponent<components::EconomicFlowState>();
    auto* flow2 = s2->getComponent<components::EconomicFlowState>();
    assertTrue(approxEqual(flow1->transport_out_rate["ore"], 30.0f), "Transport out recorded");
    assertTrue(approxEqual(flow2->transport_in_rate["ore"], 30.0f), "Transport in recorded");
}

static void testEconomicFlowDestruction() {
    std::cout << "\n=== Economic Flow: Destruction ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("system1");
    addComp<components::EconomicFlowState>(sys_entity);

    systems::EconomicFlowSystem sys(&world);
    sys.recordDestruction("system1", "cargo", 20.0f);

    auto* flow = sys_entity->getComponent<components::EconomicFlowState>();
    assertTrue(approxEqual(flow->destruction_rate["cargo"], 20.0f), "Destruction recorded");
}

static void testEconomicFlowNetFlow() {
    std::cout << "\n=== Economic Flow: Net Flow ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("system1");
    auto* flow = addComp<components::EconomicFlowState>(sys_entity);
    flow->production_rate["ore"] = 100.0f;
    flow->consumption_rate["ore"] = 40.0f;
    flow->transport_in_rate["ore"] = 10.0f;
    flow->transport_out_rate["ore"] = 20.0f;
    flow->destruction_rate["ore"] = 5.0f;

    systems::EconomicFlowSystem sys(&world);
    // net = 100 + 10 - 40 - 20 - 5 = 45
    float net = sys.getNetFlow("system1", "ore");
    assertTrue(approxEqual(net, 45.0f), "Net flow is 45");
}

static void testEconomicFlowHealthUpdate() {
    std::cout << "\n=== Economic Flow: Health Update ===" << std::endl;
    ecs::World world;
    auto* sys_entity = world.createEntity("system1");
    auto* flow = addComp<components::EconomicFlowState>(sys_entity);
    flow->production_rate["ore"] = 100.0f;
    flow->consumption_rate["ore"] = 50.0f;

    systems::EconomicFlowSystem sys(&world);
    sys.update(1.0f);

    // health should move towards 100/50 = 2.0 (clamped)
    float health = sys.getEconomicHealth("system1");
    assertTrue(health > 1.0f, "Health above 1.0 with surplus production");
}

static void testEconomicFlowMissingEntity() {
    std::cout << "\n=== Economic Flow: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::EconomicFlowSystem sys(&world);
    assertTrue(approxEqual(sys.getEconomicHealth("nonexistent"), 1.0f), "Default health for missing");
    assertTrue(approxEqual(sys.getNetFlow("nonexistent", "ore"), 0.0f), "Default net flow for missing");
    assertTrue(approxEqual(sys.getTotalProduction("nonexistent"), 0.0f), "Default production for missing");
}

static void testEconomicFlowAutoCreateComponent() {
    std::cout << "\n=== Economic Flow: Auto-create Component ===" << std::endl;
    ecs::World world;
    world.createEntity("system1");

    systems::EconomicFlowSystem sys(&world);
    sys.recordProduction("system1", "ore", 50.0f);
    assertTrue(approxEqual(sys.getProductionRate("system1", "ore"), 50.0f), "Component auto-created and production recorded");
}


void run_economic_flow_system_tests() {
    testEconomicFlowDefaults();
    testEconomicFlowRecordProduction();
    testEconomicFlowRecordConsumption();
    testEconomicFlowTransport();
    testEconomicFlowDestruction();
    testEconomicFlowNetFlow();
    testEconomicFlowHealthUpdate();
    testEconomicFlowMissingEntity();
    testEconomicFlowAutoCreateComponent();
}
