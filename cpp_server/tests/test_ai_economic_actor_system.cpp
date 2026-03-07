// Tests for: AI Economic Actor System tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "components/npc_components.h"
#include "ecs/system.h"
#include "systems/ai_economic_actor_system.h"

using namespace atlas;

// ==================== AI Economic Actor System tests ====================

static void testAIEconomicActorDefaults() {
    std::cout << "\n=== AI Economic Actor Defaults ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    auto* entity = world.createEntity("npc_trader_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);

    assertTrue(actor->owned_ship_type.empty(), "Ship type empty by default");
    assertTrue(actor->ship_value == 0.0, "Ship value starts at 0");
    assertTrue(!actor->is_destroyed, "Not destroyed by default");
    assertTrue(!actor->permanently_dead, "Not permanently dead by default");
    assertTrue(actor->destruction_count == 0, "No destructions");
    assertTrue(actor->replacement_count == 0, "No replacements");
    assertTrue(approxEqual(actor->time_alive, 0.0f), "Time alive starts at 0");
    assertTrue(intent->wallet == 10000.0, "Wallet default Credits");
}

static void testAIEconomicActorEarnSpend() {
    std::cout << "\n=== AI Economic Actor Earn/Spend ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_miner_1");
    addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);
    intent->wallet = 5000.0;

    sys.earnISC("npc_miner_1", 3000.0);
    assertTrue(intent->wallet == 8000.0, "Earned 3000 Credits");

    bool spent = sys.spendISC("npc_miner_1", 2000.0);
    assertTrue(spent, "Spend succeeds with sufficient funds");
    assertTrue(intent->wallet == 6000.0, "Wallet after spend");

    bool fail = sys.spendISC("npc_miner_1", 99999.0);
    assertTrue(!fail, "Spend fails with insufficient funds");
    assertTrue(intent->wallet == 6000.0, "Wallet unchanged after failed spend");
}

static void testAIEconomicActorShipDestruction() {
    std::cout << "\n=== AI Economic Actor Ship Destruction ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_hauler_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);
    actor->owned_ship_type = "Badger";
    actor->ship_value = 5000.0;
    intent->wallet = 12000.0;

    // Destroy the ship
    bool destroyed = sys.handleShipDestruction("npc_hauler_1");
    assertTrue(destroyed, "Destruction handled");
    assertTrue(actor->is_destroyed, "Ship marked destroyed");
    assertTrue(actor->destruction_count == 1, "Destruction count incremented");

    // Update should trigger replacement since wallet (12000) >= ship_value (5000)
    sys.update(1.0f);
    assertTrue(!actor->is_destroyed, "Ship replaced after update");
    assertTrue(actor->replacement_count == 1, "Replacement count incremented");
    assertTrue(intent->wallet == 7000.0, "Wallet reduced by ship value");
}

static void testAIEconomicActorPermanentDeath() {
    std::cout << "\n=== AI Economic Actor Permanent Death ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_pirate_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);
    actor->owned_ship_type = "Rifter";
    actor->ship_value = 10000.0;
    intent->wallet = 3000.0;  // Cannot afford replacement

    sys.handleShipDestruction("npc_pirate_1");
    sys.update(1.0f);

    assertTrue(actor->permanently_dead, "NPC permanently dead when broke");
    assertTrue(actor->is_destroyed, "Ship still destroyed");

    auto dead = sys.getDeadActors();
    assertTrue(dead.size() == 1, "One dead actor");
    assertTrue(dead[0] == "npc_pirate_1", "Dead actor is pirate");
}

static void testAIEconomicActorTimeTracking() {
    std::cout << "\n=== AI Economic Actor Time Tracking ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_patrol_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    addComp<components::SimNPCIntent>(entity);

    sys.update(5.0f);
    assertTrue(approxEqual(actor->time_alive, 5.0f), "Time alive updated");

    sys.update(3.0f);
    assertTrue(approxEqual(actor->time_alive, 8.0f), "Time alive accumulated");
}

static void testAIEconomicActorTotalEconomy() {
    std::cout << "\n=== AI Economic Actor Total Economy ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* e1 = world.createEntity("trader_1");
    addComp<components::AIEconomicActor>(e1);
    auto* i1 = addComp<components::SimNPCIntent>(e1);
    i1->wallet = 50000.0;

    auto* e2 = world.createEntity("trader_2");
    addComp<components::AIEconomicActor>(e2);
    auto* i2 = addComp<components::SimNPCIntent>(e2);
    i2->wallet = 30000.0;

    double total = sys.getTotalEconomyISC();
    assertTrue(total == 80000.0, "Total economy Credits correct");

    auto low = sys.getLowFundsActors(40000.0);
    assertTrue(low.size() == 1, "One low-funds actor");
}

static void testAIEconomicActorCanAffordReplacement() {
    std::cout << "\n=== AI Economic Actor Can Afford Replacement ===" << std::endl;
    using namespace atlas;
    ecs::World world;
    systems::AIEconomicActorSystem sys(&world);

    auto* entity = world.createEntity("npc_1");
    auto* actor = addComp<components::AIEconomicActor>(entity);
    auto* intent = addComp<components::SimNPCIntent>(entity);
    actor->ship_value = 5000.0;
    intent->wallet = 10000.0;

    assertTrue(sys.canAffordReplacement("npc_1"), "Can afford with 10k wallet and 5k ship");

    intent->wallet = 3000.0;
    assertTrue(!sys.canAffordReplacement("npc_1"), "Cannot afford with 3k wallet and 5k ship");
}


void run_ai_economic_actor_system_tests() {
    testAIEconomicActorDefaults();
    testAIEconomicActorEarnSpend();
    testAIEconomicActorShipDestruction();
    testAIEconomicActorPermanentDeath();
    testAIEconomicActorTimeTracking();
    testAIEconomicActorTotalEconomy();
    testAIEconomicActorCanAffordReplacement();
}
