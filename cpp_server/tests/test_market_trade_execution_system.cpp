// Tests for: Market Trade Execution System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/market_trade_execution_system.h"

using namespace atlas;

// ==================== Market Trade Execution System Tests ====================

static void testMarketTradeCreate() {
    std::cout << "\n=== MarketTrade: Create ===" << std::endl;
    ecs::World world;
    systems::MarketTradeExecutionSystem sys(&world);
    world.createEntity("market1");
    assertTrue(sys.initialize("market1", 0.02), "Init succeeds with 2% fee");
    assertTrue(sys.getQueuedOrderCount("market1") == 0, "0 queued orders");
    assertTrue(sys.getCompletedTradeCount("market1") == 0, "0 completed");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalBrokerFees("market1")), 0.0f), "0 fees");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalVolumeTraded("market1")), 0.0f), "0 volume");
    assertTrue(sys.getPartialFillCount("market1") == 0, "0 partial fills");
}

static void testMarketTradeBuyFullFill() {
    std::cout << "\n=== MarketTrade: BuyFullFill ===" << std::endl;
    ecs::World world;
    systems::MarketTradeExecutionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1", 0.02); // 2% broker fee

    // Buy 10 units at 100 ISC each, balance = 2000 ISC
    assertTrue(sys.queueBuyOrder("market1", "tritanium", 10, 100.0, 2000.0), "Queue buy");
    assertTrue(sys.getQueuedOrderCount("market1") == 1, "1 queued");

    sys.update(1.0f);
    assertTrue(sys.getQueuedOrderCount("market1") == 0, "0 queued after tick");
    assertTrue(sys.getCompletedTradeCount("market1") == 1, "1 completed");

    // Total cost = 10 * 100 = 1000, broker fee = 1000 * 0.02 = 20
    assertTrue(approxEqual(static_cast<float>(sys.getTotalBrokerFees("market1")), 20.0f), "20 ISC fee");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalVolumeTraded("market1")), 1000.0f), "1000 volume");
}

static void testMarketTradeSellFullFill() {
    std::cout << "\n=== MarketTrade: SellFullFill ===" << std::endl;
    ecs::World world;
    systems::MarketTradeExecutionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1", 0.05); // 5% broker fee

    // Sell 20 units at 50 ISC each, stock = 20
    assertTrue(sys.queueSellOrder("market1", "pyerite", 20, 50.0, 20), "Queue sell");
    sys.update(1.0f);

    assertTrue(sys.getCompletedTradeCount("market1") == 1, "1 completed");
    // Revenue = 20 * 50 = 1000, fee = 1000 * 0.05 = 50
    assertTrue(approxEqual(static_cast<float>(sys.getTotalBrokerFees("market1")), 50.0f), "50 ISC fee");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalVolumeTraded("market1")), 1000.0f), "1000 volume");
}

static void testMarketTradePartialBuy() {
    std::cout << "\n=== MarketTrade: PartialBuy ===" << std::endl;
    ecs::World world;
    systems::MarketTradeExecutionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1", 0.0); // No broker fee for simpler math

    // Try to buy 10 at 100 ISC, but only have 500 ISC balance
    assertTrue(sys.queueBuyOrder("market1", "tritanium", 10, 100.0, 500.0), "Queue buy");
    sys.update(1.0f);

    assertTrue(sys.getCompletedTradeCount("market1") == 1, "1 completed (partial)");
    assertTrue(sys.getPartialFillCount("market1") == 1, "1 partial fill");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalVolumeTraded("market1")), 500.0f), "500 volume (5 units)");
}

static void testMarketTradePartialSell() {
    std::cout << "\n=== MarketTrade: PartialSell ===" << std::endl;
    ecs::World world;
    systems::MarketTradeExecutionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1", 0.0);

    // Sell 20 units but only have 5 in stock
    assertTrue(sys.queueSellOrder("market1", "pyerite", 20, 100.0, 5), "Queue sell");
    sys.update(1.0f);

    assertTrue(sys.getCompletedTradeCount("market1") == 1, "1 completed (partial)");
    assertTrue(sys.getPartialFillCount("market1") == 1, "1 partial fill");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalVolumeTraded("market1")), 500.0f), "500 volume (5 units)");
}

static void testMarketTradeSetBrokerFee() {
    std::cout << "\n=== MarketTrade: SetBrokerFee ===" << std::endl;
    ecs::World world;
    systems::MarketTradeExecutionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1", 0.02);

    assertTrue(sys.setBrokerFeeRate("market1", 0.10), "Set fee to 10%");

    sys.queueBuyOrder("market1", "tritanium", 10, 100.0, 2000.0);
    sys.update(1.0f);

    // Fee = 1000 * 0.10 = 100
    assertTrue(approxEqual(static_cast<float>(sys.getTotalBrokerFees("market1")), 100.0f), "100 ISC fee at 10%");
}

static void testMarketTradeInvalidOrders() {
    std::cout << "\n=== MarketTrade: InvalidOrders ===" << std::endl;
    ecs::World world;
    systems::MarketTradeExecutionSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1", 0.02);

    assertTrue(!sys.queueBuyOrder("market1", "t", 0, 100.0, 1000.0), "0 qty rejected");
    assertTrue(!sys.queueBuyOrder("market1", "t", -5, 100.0, 1000.0), "Negative qty rejected");
    assertTrue(!sys.queueBuyOrder("market1", "t", 10, 0.0, 1000.0), "0 price rejected");
    assertTrue(!sys.queueBuyOrder("market1", "t", 10, -10.0, 1000.0), "Negative price rejected");
    assertTrue(!sys.queueSellOrder("market1", "t", 10, 100.0, -1), "Negative stock rejected");
    assertTrue(sys.getQueuedOrderCount("market1") == 0, "All invalid rejected");
}

static void testMarketTradeMissing() {
    std::cout << "\n=== MarketTrade: Missing ===" << std::endl;
    ecs::World world;
    systems::MarketTradeExecutionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 0.02), "Init fails on missing");
    assertTrue(!sys.queueBuyOrder("nonexistent", "t", 1, 100.0, 1000.0), "Buy fails on missing");
    assertTrue(!sys.queueSellOrder("nonexistent", "t", 1, 100.0, 10), "Sell fails on missing");
    assertTrue(sys.getQueuedOrderCount("nonexistent") == 0, "0 queued on missing");
    assertTrue(sys.getCompletedTradeCount("nonexistent") == 0, "0 completed on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalBrokerFees("nonexistent")), 0.0f), "0 fees on missing");
    assertTrue(sys.getPartialFillCount("nonexistent") == 0, "0 partial on missing");
}

void run_market_trade_execution_system_tests() {
    testMarketTradeCreate();
    testMarketTradeBuyFullFill();
    testMarketTradeSellFullFill();
    testMarketTradePartialBuy();
    testMarketTradePartialSell();
    testMarketTradeSetBrokerFee();
    testMarketTradeInvalidOrders();
    testMarketTradeMissing();
}
