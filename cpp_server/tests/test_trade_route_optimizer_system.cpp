// Tests for: TradeRouteOptimizerSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/trade_route_optimizer_system.h"

using namespace atlas;

// ==================== TradeRouteOptimizerSystem Tests ====================

static void testTradeRouteOptimizerCreate() {
    std::cout << "\n=== TradeRouteOptimizer: Create ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    assertTrue(sys.initialize("trader1", 100), "Init with 100 cargo succeeds");
    assertTrue(sys.getCargoCapacity("trader1") == 100, "Cargo capacity 100");
    assertTrue(sys.getMarketEntryCount("trader1") == 0, "Zero market entries");
    assertTrue(sys.getRouteHopCount("trader1") == 0, "Zero route hops");
    assertTrue(sys.getRoutesCalculated("trader1") == 0, "Zero routes calculated");
    assertTrue(approxEqual(sys.getTotalEstimatedProfit("trader1"), 0.0f), "Zero profit");
}

static void testTradeRouteOptimizerInvalidInit() {
    std::cout << "\n=== TradeRouteOptimizer: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    assertTrue(!sys.initialize("missing", 100), "Missing entity fails");
    world.createEntity("trader1");
    assertTrue(!sys.initialize("trader1", 0), "Zero cargo fails");
    assertTrue(!sys.initialize("trader1", -10), "Negative cargo fails");
}

static void testTradeRouteOptimizerAddMarket() {
    std::cout << "\n=== TradeRouteOptimizer: AddMarket ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    assertTrue(sys.addMarketEntry("trader1", "station_a", "tritanium", 10.0f, 8.0f, 500, 200),
               "Add tritanium at station A");
    assertTrue(sys.addMarketEntry("trader1", "station_b", "tritanium", 12.0f, 15.0f, 100, 800),
               "Add tritanium at station B");
    assertTrue(sys.addMarketEntry("trader1", "station_a", "pyerite", 20.0f, 18.0f, 300, 100),
               "Add pyerite at station A");
    assertTrue(sys.getMarketEntryCount("trader1") == 3, "3 market entries");

    // Duplicate rejected
    assertTrue(!sys.addMarketEntry("trader1", "station_a", "tritanium", 5.0f, 4.0f, 100, 100),
               "Duplicate station+commodity rejected");
}

static void testTradeRouteOptimizerInvalidMarket() {
    std::cout << "\n=== TradeRouteOptimizer: InvalidMarket ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    assertTrue(!sys.addMarketEntry("trader1", "", "tritanium", 10.0f, 8.0f, 100, 100),
               "Empty station rejected");
    assertTrue(!sys.addMarketEntry("trader1", "station_a", "", 10.0f, 8.0f, 100, 100),
               "Empty commodity rejected");
    assertTrue(!sys.addMarketEntry("trader1", "station_a", "ore", -1.0f, 8.0f, 100, 100),
               "Negative buy price rejected");
    assertTrue(!sys.addMarketEntry("trader1", "station_a", "ore", 10.0f, -1.0f, 100, 100),
               "Negative sell price rejected");
    assertTrue(!sys.addMarketEntry("trader1", "station_a", "ore", 10.0f, 8.0f, -1, 100),
               "Negative supply rejected");
    assertTrue(!sys.addMarketEntry("nonexistent", "station_a", "ore", 10.0f, 8.0f, 100, 100),
               "Missing entity rejected");
}

static void testTradeRouteOptimizerRemoveMarket() {
    std::cout << "\n=== TradeRouteOptimizer: RemoveMarket ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    sys.addMarketEntry("trader1", "station_a", "tritanium", 10.0f, 8.0f, 500, 200);
    sys.addMarketEntry("trader1", "station_b", "tritanium", 12.0f, 15.0f, 100, 800);

    assertTrue(sys.removeMarketEntry("trader1", "station_a", "tritanium"), "Remove succeeds");
    assertTrue(sys.getMarketEntryCount("trader1") == 1, "1 entry remaining");
    assertTrue(!sys.removeMarketEntry("trader1", "station_a", "tritanium"), "Double remove fails");
    assertTrue(!sys.removeMarketEntry("trader1", "station_x", "gold"), "Remove nonexistent fails");
}

static void testTradeRouteOptimizerClearMarket() {
    std::cout << "\n=== TradeRouteOptimizer: ClearMarket ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    sys.addMarketEntry("trader1", "station_a", "tritanium", 10.0f, 8.0f, 500, 200);
    sys.addMarketEntry("trader1", "station_b", "pyerite", 20.0f, 18.0f, 300, 100);

    assertTrue(sys.clearMarketData("trader1"), "Clear market succeeds");
    assertTrue(sys.getMarketEntryCount("trader1") == 0, "Zero entries after clear");
}

static void testTradeRouteOptimizerCalculateRoute() {
    std::cout << "\n=== TradeRouteOptimizer: CalculateRoute ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    // Station A sells tritanium cheap (buy=10), Station B buys it high (sell=15)
    sys.addMarketEntry("trader1", "station_a", "tritanium", 10.0f, 8.0f, 500, 200);
    sys.addMarketEntry("trader1", "station_b", "tritanium", 12.0f, 15.0f, 100, 800);

    assertTrue(sys.calculateOptimalRoute("trader1", 60.0f), "Calculate route succeeds");
    assertTrue(sys.getRouteHopCount("trader1") >= 1, "At least 1 route hop");
    assertTrue(sys.getRoutesCalculated("trader1") == 1, "1 route calculated");

    float profit = sys.getTotalEstimatedProfit("trader1");
    assertTrue(profit > 0.0f, "Positive profit estimated");

    float bestPPU = sys.getBestProfitPerUnit("trader1");
    assertTrue(bestPPU > 0.0f, "Positive best profit per unit");
}

static void testTradeRouteOptimizerNoProfit() {
    std::cout << "\n=== TradeRouteOptimizer: NoProfit ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    // All prices make trade unprofitable
    sys.addMarketEntry("trader1", "station_a", "tritanium", 15.0f, 8.0f, 500, 0);
    sys.addMarketEntry("trader1", "station_b", "tritanium", 20.0f, 10.0f, 100, 0);

    assertTrue(!sys.calculateOptimalRoute("trader1", 60.0f), "No profitable route");
}

static void testTradeRouteOptimizerInvalidCalc() {
    std::cout << "\n=== TradeRouteOptimizer: InvalidCalc ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    // Empty market data
    assertTrue(!sys.calculateOptimalRoute("trader1", 60.0f), "Empty market fails");

    sys.addMarketEntry("trader1", "station_a", "tritanium", 10.0f, 8.0f, 500, 200);
    assertTrue(!sys.calculateOptimalRoute("trader1", 0.0f), "Zero travel time rejected");
    assertTrue(!sys.calculateOptimalRoute("trader1", -5.0f), "Negative travel time rejected");
    assertTrue(!sys.calculateOptimalRoute("nonexistent", 60.0f), "Missing entity rejected");
}

static void testTradeRouteOptimizerClearRoute() {
    std::cout << "\n=== TradeRouteOptimizer: ClearRoute ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    sys.addMarketEntry("trader1", "station_a", "tritanium", 10.0f, 8.0f, 500, 200);
    sys.addMarketEntry("trader1", "station_b", "tritanium", 12.0f, 15.0f, 100, 800);
    sys.calculateOptimalRoute("trader1", 60.0f);

    assertTrue(sys.clearRoute("trader1"), "Clear route succeeds");
    assertTrue(sys.getRouteHopCount("trader1") == 0, "Zero hops after clear");
    assertTrue(approxEqual(sys.getTotalEstimatedProfit("trader1"), 0.0f), "Zero profit after clear");
}

static void testTradeRouteOptimizerUpdate() {
    std::cout << "\n=== TradeRouteOptimizer: Update ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    world.createEntity("trader1");
    sys.initialize("trader1", 100);

    sys.update(1.0f);
    // Just verifies update doesn't crash
    assertTrue(true, "Update tick OK");
}

static void testTradeRouteOptimizerMissing() {
    std::cout << "\n=== TradeRouteOptimizer: Missing ===" << std::endl;
    ecs::World world;
    systems::TradeRouteOptimizerSystem sys(&world);
    assertTrue(sys.getMarketEntryCount("x") == 0, "Default market entries on missing");
    assertTrue(sys.getRouteHopCount("x") == 0, "Default route hops on missing");
    assertTrue(approxEqual(sys.getTotalEstimatedProfit("x"), 0.0f), "Default profit on missing");
    assertTrue(approxEqual(sys.getTotalTravelTime("x"), 0.0f), "Default travel time on missing");
    assertTrue(sys.getCargoCapacity("x") == 0, "Default cargo on missing");
    assertTrue(sys.getRoutesCalculated("x") == 0, "Default routes calculated on missing");
    assertTrue(approxEqual(sys.getBestProfitPerUnit("x"), 0.0f), "Default best PPU on missing");
}

void run_trade_route_optimizer_system_tests() {
    testTradeRouteOptimizerCreate();
    testTradeRouteOptimizerInvalidInit();
    testTradeRouteOptimizerAddMarket();
    testTradeRouteOptimizerInvalidMarket();
    testTradeRouteOptimizerRemoveMarket();
    testTradeRouteOptimizerClearMarket();
    testTradeRouteOptimizerCalculateRoute();
    testTradeRouteOptimizerNoProfit();
    testTradeRouteOptimizerInvalidCalc();
    testTradeRouteOptimizerClearRoute();
    testTradeRouteOptimizerUpdate();
    testTradeRouteOptimizerMissing();
}
