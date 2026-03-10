// Tests for: Order Matching System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/order_matching_system.h"

using namespace atlas;

// ==================== Order Matching System Tests ====================

static void testOrderMatchingCreate() {
    std::cout << "\n=== OrderMatching: Create ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    world.createEntity("market1");
    assertTrue(sys.initialize("market1", "the_forge"), "Init succeeds");
    assertTrue(sys.getBuyOrderCount("market1") == 0, "No buy orders");
    assertTrue(sys.getSellOrderCount("market1") == 0, "No sell orders");
    assertTrue(sys.getTotalMatches("market1") == 0, "0 matches");
    assertTrue(sys.getTotalVolumeTraded("market1") == 0, "0 volume");
    assertTrue(approxEqual(sys.getTotalValueTraded("market1"), 0.0f), "0 value");
    assertTrue(approxEqual(sys.getFeesCollected("market1"), 0.0f), "0 fees");
    assertTrue(approxEqual(sys.getBrokerFeeRate("market1"), 0.02f), "Default 2% fee");
}

static void testOrderMatchingPlaceOrders() {
    std::cout << "\n=== OrderMatching: PlaceOrders ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1", "the_forge");

    assertTrue(sys.placeBuyOrder("market1", "buy_01", "player1", "tritanium", 100, 5.0f),
               "Place buy order");
    assertTrue(sys.placeSellOrder("market1", "sell_01", "player2", "tritanium", 50, 4.0f),
               "Place sell order");
    assertTrue(sys.getBuyOrderCount("market1") == 1, "1 buy order");
    assertTrue(sys.getSellOrderCount("market1") == 1, "1 sell order");
}

static void testOrderMatchingMatch() {
    std::cout << "\n=== OrderMatching: Match ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1", "the_forge");

    // Buy at 5.0, sell at 4.0 → should match
    sys.placeBuyOrder("market1", "buy_01", "player1", "tritanium", 100, 5.0f);
    sys.placeSellOrder("market1", "sell_01", "player2", "tritanium", 50, 4.0f);

    sys.update(1.0f);
    assertTrue(sys.getTotalMatches("market1") == 1, "1 match");
    assertTrue(sys.getTotalVolumeTraded("market1") == 50, "50 units traded");
    // Trade at ask price: 4.0 * 50 = 200
    assertTrue(approxEqual(sys.getTotalValueTraded("market1"), 200.0f), "200 value traded");
    // Fees: 200 * 0.02 = 4.0
    assertTrue(approxEqual(sys.getFeesCollected("market1"), 4.0f), "4.0 fees collected");
    // Buy order partially filled (50 of 100), sell fully filled
    assertTrue(sys.getBuyOrderCount("market1") == 1, "Buy order remains (partial)");
    assertTrue(sys.getSellOrderCount("market1") == 0, "Sell order removed (filled)");
}

static void testOrderMatchingNoMatch() {
    std::cout << "\n=== OrderMatching: NoMatch ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    // Buy at 3.0, sell at 5.0 → no match (buy < sell)
    sys.placeBuyOrder("market1", "buy_01", "player1", "tritanium", 100, 3.0f);
    sys.placeSellOrder("market1", "sell_01", "player2", "tritanium", 50, 5.0f);

    sys.update(1.0f);
    assertTrue(sys.getTotalMatches("market1") == 0, "No matches");
    assertTrue(sys.getBuyOrderCount("market1") == 1, "Buy order stays");
    assertTrue(sys.getSellOrderCount("market1") == 1, "Sell order stays");
}

static void testOrderMatchingDifferentItems() {
    std::cout << "\n=== OrderMatching: DifferentItems ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    // Different item types → no match
    sys.placeBuyOrder("market1", "buy_01", "player1", "tritanium", 100, 10.0f);
    sys.placeSellOrder("market1", "sell_01", "player2", "pyerite", 50, 1.0f);

    sys.update(1.0f);
    assertTrue(sys.getTotalMatches("market1") == 0, "No cross-item matches");
}

static void testOrderMatchingCancel() {
    std::cout << "\n=== OrderMatching: Cancel ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    sys.placeBuyOrder("market1", "buy_01", "player1", "tritanium", 100, 5.0f);
    assertTrue(sys.cancelOrder("market1", "buy_01"), "Cancel buy succeeds");

    sys.update(1.0f);
    assertTrue(sys.getBuyOrderCount("market1") == 0, "Cancelled order removed");
    assertTrue(!sys.cancelOrder("market1", "buy_01"), "Double cancel fails");
}

static void testOrderMatchingBrokerFee() {
    std::cout << "\n=== OrderMatching: BrokerFee ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    sys.setBrokerFeeRate("market1", 0.05f); // 5%
    assertTrue(approxEqual(sys.getBrokerFeeRate("market1"), 0.05f), "5% fee set");

    sys.placeBuyOrder("market1", "buy_01", "player1", "tritanium", 100, 10.0f);
    sys.placeSellOrder("market1", "sell_01", "player2", "tritanium", 100, 8.0f);
    sys.update(1.0f);

    // Trade at 8.0 * 100 = 800, fee = 800 * 0.05 = 40
    assertTrue(approxEqual(sys.getFeesCollected("market1"), 40.0f), "40 fees at 5%");
}

static void testOrderMatchingMultipleMatches() {
    std::cout << "\n=== OrderMatching: MultipleMatches ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    sys.placeBuyOrder("market1", "buy_01", "p1", "tritanium", 50, 10.0f);
    sys.placeBuyOrder("market1", "buy_02", "p2", "tritanium", 30, 9.0f);
    sys.placeSellOrder("market1", "sell_01", "p3", "tritanium", 70, 8.0f);

    sys.update(1.0f);
    // Best buy (10.0) matches sell (8.0) first: fill 50 units
    // Then buy_02 (9.0) matches remaining 20 at sell price
    assertTrue(sys.getTotalMatches("market1") == 2, "2 matches");
    assertTrue(sys.getTotalVolumeTraded("market1") == 70, "70 total volume");
}

static void testOrderMatchingMissing() {
    std::cout << "\n=== OrderMatching: Missing ===" << std::endl;
    ecs::World world;
    systems::OrderMatchingSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.placeBuyOrder("nonexistent", "o1", "p1", "t", 1, 1.0f), "Buy fails on missing");
    assertTrue(!sys.placeSellOrder("nonexistent", "o1", "p1", "t", 1, 1.0f), "Sell fails on missing");
    assertTrue(!sys.cancelOrder("nonexistent", "o1"), "Cancel fails on missing");
    assertTrue(!sys.setBrokerFeeRate("nonexistent", 0.01f), "SetFee fails on missing");
    assertTrue(sys.getBuyOrderCount("nonexistent") == 0, "0 buy on missing");
    assertTrue(sys.getSellOrderCount("nonexistent") == 0, "0 sell on missing");
    assertTrue(sys.getTotalMatches("nonexistent") == 0, "0 matches on missing");
    assertTrue(sys.getTotalVolumeTraded("nonexistent") == 0, "0 volume on missing");
    assertTrue(approxEqual(sys.getTotalValueTraded("nonexistent"), 0.0f), "0 value on missing");
    assertTrue(approxEqual(sys.getFeesCollected("nonexistent"), 0.0f), "0 fees on missing");
    assertTrue(approxEqual(sys.getBrokerFeeRate("nonexistent"), 0.0f), "0 rate on missing");
}

void run_order_matching_system_tests() {
    testOrderMatchingCreate();
    testOrderMatchingPlaceOrders();
    testOrderMatchingMatch();
    testOrderMatchingNoMatch();
    testOrderMatchingDifferentItems();
    testOrderMatchingCancel();
    testOrderMatchingBrokerFee();
    testOrderMatchingMultipleMatches();
    testOrderMatchingMissing();
}
