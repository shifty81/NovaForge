// Tests for: Mineral Economy Integration Test
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/market_system.h"
#include "systems/mining_system.h"
#include "systems/refining_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Mineral Economy Integration Test ====================

static void testMineralEconomyEndToEnd() {
    std::cout << "\n=== Mineral Economy End-to-End ===" << std::endl;
    ecs::World world;
    systems::MiningSystem mining(&world);
    systems::RefiningSystem refining(&world);
    systems::MarketSystem market(&world);

    // Create station with refining and market
    auto* station = world.createEntity("econ_station");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "econ_station";
    addComp<components::RefiningFacility>(station);
    refining.installDefaultRecipes("econ_station");

    // Seed NPC market
    int seeded = market.seedNPCOrders("econ_station");
    assertTrue(seeded == 4, "NPC orders seeded");

    // Create miner with laser and inventory
    auto* miner = world.createEntity("econ_miner");
    auto* minerPos = addComp<components::Position>(miner);
    minerPos->x = 0; minerPos->y = 0; minerPos->z = 0;
    auto* laser = addComp<components::MiningLaser>(miner);
    laser->yield_per_cycle = 100.0f;
    laser->cycle_time = 1.0f;
    auto* inv = addComp<components::Inventory>(miner);
    inv->max_capacity = 10000.0f;

    // Create deposit
    std::string depositId = mining.createDeposit("Ferrite", 5000.0f, 10.0f, 0.0f, 0.0f);
    assertTrue(!depositId.empty(), "Deposit created");

    // Mine ore
    bool started = mining.startMining("econ_miner", depositId);
    assertTrue(started, "Mining started");

    // Run several cycles
    for (int i = 0; i < 5; ++i) {
        mining.update(1.0f);
    }

    // Check miner has ore
    bool hasOre = false;
    for (const auto& item : inv->items) {
        if (item.name == "Ferrite" && item.quantity > 0) hasOre = true;
    }
    assertTrue(hasOre, "Miner has Ferrite ore after mining");

    // Refine the ore at the station
    int refined = refining.refineOre("econ_miner", "econ_station", "Ferrite", 1);
    assertTrue(refined > 0, "Ore refined successfully");

    // Check that Stellium was produced (Ferrite → Stellium)
    bool hasStellium = false;
    for (const auto& item : inv->items) {
        if (item.name == "Stellium" && item.quantity > 0) hasStellium = true;
    }
    assertTrue(hasStellium, "Miner has Stellium after refining Ferrite");

    // Verify market still has mineral prices
    double tritPrice = market.getLowestSellPrice("econ_station", "mineral_tritanium");
    assertTrue(tritPrice > 0.0, "Stellium still available on market");
}


void run_economy_tests() {
    testMineralEconomyEndToEnd();
}
