// Tests for: Market Browser System
#include "test_log.h"
#include "components/core_components.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/market_browser_system.h"

using namespace atlas;

// ==================== Market Browser System Tests ====================

static void testMarketBrowserCreate() {
    std::cout << "\n=== MarketBrowser: Create ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_001"), "Init succeeds");
    assertTrue(sys.getOrderCount("p1") == 0, "No orders");
    assertTrue(sys.getBuyOrderCount("p1") == 0, "No buy orders");
    assertTrue(sys.getSellOrderCount("p1") == 0, "No sell orders");
    assertTrue(sys.getFavoriteCount("p1") == 0, "No favorites");
    assertTrue(sys.getTransactionCount("p1") == 0, "No transactions");
    assertTrue(approxEqual(sys.getTotalSpent("p1"), 0.0), "0 spent");
    assertTrue(approxEqual(sys.getTotalEarned("p1"), 0.0), "0 earned");
    assertTrue(sys.getFilter("p1").empty(), "No filter");
}

static void testMarketBrowserAddOrders() {
    std::cout << "\n=== MarketBrowser: AddOrders ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.addOrder("p1", "o1", "Tritanium", false, 5.50, 1000), "Add sell order");
    assertTrue(sys.addOrder("p1", "o2", "Tritanium", true, 5.00, 500), "Add buy order");
    assertTrue(sys.addOrder("p1", "o3", "Pyerite", false, 12.00, 200), "Add Pyerite sell");
    assertTrue(sys.getOrderCount("p1") == 3, "3 orders");
    assertTrue(sys.getBuyOrderCount("p1") == 1, "1 buy order");
    assertTrue(sys.getSellOrderCount("p1") == 2, "2 sell orders");
    assertTrue(sys.hasOrder("p1", "o1"), "Has o1");
    assertTrue(!sys.hasOrder("p1", "o99"), "No o99");
    // Duplicate rejected
    assertTrue(!sys.addOrder("p1", "o1", "Tritanium", false, 6.00, 100), "Dup rejected");
    assertTrue(sys.getOrderCount("p1") == 3, "Still 3 orders");
}

static void testMarketBrowserOrderMax() {
    std::cout << "\n=== MarketBrowser: OrderMax ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::MarketBrowserState>();
    state->max_orders = 2;
    sys.addOrder("p1", "o1", "Tritanium", false, 5.0, 100);
    sys.addOrder("p1", "o2", "Pyerite", false, 10.0, 200);
    assertTrue(!sys.addOrder("p1", "o3", "Mexallon", false, 20.0, 50), "Max orders enforced");
    assertTrue(sys.getOrderCount("p1") == 2, "Still 2");
}

static void testMarketBrowserRemoveOrder() {
    std::cout << "\n=== MarketBrowser: RemoveOrder ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addOrder("p1", "o1", "Tritanium", false, 5.0, 100);
    sys.addOrder("p1", "o2", "Pyerite", true, 10.0, 200);
    assertTrue(sys.removeOrder("p1", "o1"), "Remove o1");
    assertTrue(sys.getOrderCount("p1") == 1, "1 left");
    assertTrue(!sys.hasOrder("p1", "o1"), "o1 gone");
    assertTrue(!sys.removeOrder("p1", "o1"), "Double remove fails");
    assertTrue(!sys.removeOrder("p1", "nonexistent"), "Nonexistent fails");
}

static void testMarketBrowserPriceQueries() {
    std::cout << "\n=== MarketBrowser: PriceQueries ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addOrder("p1", "o1", "Tritanium", false, 5.50, 1000);
    sys.addOrder("p1", "o2", "Tritanium", false, 5.20, 500);
    sys.addOrder("p1", "o3", "Tritanium", true, 5.00, 800);
    sys.addOrder("p1", "o4", "Tritanium", true, 5.10, 300);
    assertTrue(approxEqual(sys.getLowestSellPrice("p1", "Tritanium"), 5.20), "Lowest sell 5.20");
    assertTrue(approxEqual(sys.getHighestBuyPrice("p1", "Tritanium"), 5.10), "Highest buy 5.10");
    assertTrue(sys.getOrderCountForItem("p1", "Tritanium") == 4, "4 Tritanium orders");
    assertTrue(sys.getOrderCountForItem("p1", "Pyerite") == 0, "0 Pyerite orders");
    assertTrue(approxEqual(sys.getLowestSellPrice("p1", "Pyerite"), 0.0), "0 for missing item");
    assertTrue(approxEqual(sys.getHighestBuyPrice("p1", "Pyerite"), 0.0), "0 for missing item");
}

static void testMarketBrowserFilter() {
    std::cout << "\n=== MarketBrowser: Filter ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.addOrder("p1", "o1", "Tritanium", false, 5.0, 100);
    sys.addOrder("p1", "o2", "Pyerite", false, 10.0, 200);
    sys.addOrder("p1", "o3", "Tritanium Ore", false, 3.0, 500);
    // No filter — all match
    assertTrue(sys.getFilteredCount("p1") == 3, "3 unfiltered");
    // Filter by "Tri"
    assertTrue(sys.setFilter("p1", "Tri"), "Set filter");
    assertTrue(sys.getFilter("p1") == "Tri", "Filter is Tri");
    assertTrue(sys.getFilteredCount("p1") == 2, "2 match Tri");
    // Filter by "Pyerite"
    sys.setFilter("p1", "Pyerite");
    assertTrue(sys.getFilteredCount("p1") == 1, "1 match Pyerite");
    // Clear filter
    sys.setFilter("p1", "");
    assertTrue(sys.getFilteredCount("p1") == 3, "3 with empty filter");
}

static void testMarketBrowserFavorites() {
    std::cout << "\n=== MarketBrowser: Favorites ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.addFavorite("p1", "Tritanium"), "Add fav");
    assertTrue(sys.addFavorite("p1", "Pyerite"), "Add fav 2");
    assertTrue(sys.getFavoriteCount("p1") == 2, "2 favorites");
    assertTrue(sys.isFavorite("p1", "Tritanium"), "Is fav");
    assertTrue(!sys.isFavorite("p1", "Mexallon"), "Not fav");
    assertTrue(!sys.addFavorite("p1", "Tritanium"), "Dup fav rejected");
    assertTrue(sys.removeFavorite("p1", "Tritanium"), "Remove fav");
    assertTrue(sys.getFavoriteCount("p1") == 1, "1 favorite");
    assertTrue(!sys.isFavorite("p1", "Tritanium"), "No longer fav");
    assertTrue(!sys.removeFavorite("p1", "Tritanium"), "Double remove fails");
}

static void testMarketBrowserFavoriteMax() {
    std::cout << "\n=== MarketBrowser: FavoriteMax ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::MarketBrowserState>();
    state->max_favorites = 2;
    sys.addFavorite("p1", "Tritanium");
    sys.addFavorite("p1", "Pyerite");
    assertTrue(!sys.addFavorite("p1", "Mexallon"), "Max favorites enforced");
    assertTrue(sys.getFavoriteCount("p1") == 2, "Still 2");
}

static void testMarketBrowserTransactions() {
    std::cout << "\n=== MarketBrowser: Transactions ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    assertTrue(sys.recordTransaction("p1", "Tritanium", true, 5.0, 100), "Buy 100 Tri");
    assertTrue(sys.recordTransaction("p1", "Pyerite", false, 10.0, 50), "Sell 50 Pye");
    assertTrue(sys.getTransactionCount("p1") == 2, "2 transactions");
    assertTrue(approxEqual(sys.getTotalSpent("p1"), 500.0), "500 spent");
    assertTrue(approxEqual(sys.getTotalEarned("p1"), 500.0), "500 earned");
    // Additional buy
    sys.recordTransaction("p1", "Mexallon", true, 20.0, 10);
    assertTrue(approxEqual(sys.getTotalSpent("p1"), 700.0), "700 spent total");
    assertTrue(sys.getTransactionCount("p1") == 3, "3 transactions");
}

static void testMarketBrowserTransactionMax() {
    std::cout << "\n=== MarketBrowser: TransactionMax ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::MarketBrowserState>();
    state->max_transactions = 2;
    sys.recordTransaction("p1", "Tritanium", true, 5.0, 100);
    sys.recordTransaction("p1", "Pyerite", true, 10.0, 50);
    // Oldest evicted when max reached
    assertTrue(sys.recordTransaction("p1", "Mexallon", false, 20.0, 10), "Evicts oldest");
    assertTrue(sys.getTransactionCount("p1") == 2, "Still 2 transactions");
}

static void testMarketBrowserUpdate() {
    std::cout << "\n=== MarketBrowser: Update ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "player_001");
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("p1");
    auto* state = entity->getComponent<components::MarketBrowserState>();
    assertTrue(approxEqual(state->elapsed_time, 3.5f), "Elapsed time 3.5s");
}

static void testMarketBrowserMissing() {
    std::cout << "\n=== MarketBrowser: Missing ===" << std::endl;
    ecs::World world;
    systems::MarketBrowserSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "x"), "Init fails");
    assertTrue(!sys.addOrder("nonexistent", "o", "i", false, 1.0, 1), "addOrder fails");
    assertTrue(sys.getOrderCount("nonexistent") == 0, "0 orders");
    assertTrue(!sys.removeOrder("nonexistent", "o"), "removeOrder fails");
    assertTrue(!sys.hasOrder("nonexistent", "o"), "hasOrder false");
    assertTrue(sys.getBuyOrderCount("nonexistent") == 0, "0 buy");
    assertTrue(sys.getSellOrderCount("nonexistent") == 0, "0 sell");
    assertTrue(approxEqual(sys.getLowestSellPrice("nonexistent", "i"), 0.0), "0 sell price");
    assertTrue(approxEqual(sys.getHighestBuyPrice("nonexistent", "i"), 0.0), "0 buy price");
    assertTrue(sys.getOrderCountForItem("nonexistent", "i") == 0, "0 item orders");
    assertTrue(!sys.setFilter("nonexistent", "f"), "setFilter fails");
    assertTrue(sys.getFilter("nonexistent").empty(), "Empty filter");
    assertTrue(sys.getFilteredCount("nonexistent") == 0, "0 filtered");
    assertTrue(!sys.addFavorite("nonexistent", "i"), "addFav fails");
    assertTrue(!sys.removeFavorite("nonexistent", "i"), "removeFav fails");
    assertTrue(!sys.isFavorite("nonexistent", "i"), "isFav false");
    assertTrue(sys.getFavoriteCount("nonexistent") == 0, "0 favs");
    assertTrue(!sys.recordTransaction("nonexistent", "i", true, 1.0, 1), "recordTx fails");
    assertTrue(sys.getTransactionCount("nonexistent") == 0, "0 tx");
    assertTrue(approxEqual(sys.getTotalSpent("nonexistent"), 0.0), "0 spent");
    assertTrue(approxEqual(sys.getTotalEarned("nonexistent"), 0.0), "0 earned");
}

void run_market_browser_system_tests() {
    testMarketBrowserCreate();
    testMarketBrowserAddOrders();
    testMarketBrowserOrderMax();
    testMarketBrowserRemoveOrder();
    testMarketBrowserPriceQueries();
    testMarketBrowserFilter();
    testMarketBrowserFavorites();
    testMarketBrowserFavoriteMax();
    testMarketBrowserTransactions();
    testMarketBrowserTransactionMax();
    testMarketBrowserUpdate();
    testMarketBrowserMissing();
}
