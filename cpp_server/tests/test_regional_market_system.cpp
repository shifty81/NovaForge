// Tests for: Regional Market System
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/regional_market_system.h"

using namespace atlas;

// ==================== Regional Market System Tests ====================

static void testRegionalMarketCreate() {
    std::cout << "\n=== RegionalMarket: Create ===" << std::endl;
    ecs::World world;
    systems::RegionalMarketSystem sys(&world);
    world.createEntity("market1");
    assertTrue(sys.initialize("market1"), "Init succeeds");
    assertTrue(sys.getTrackedItemCount("market1") == 0, "No items initially");
    assertTrue(sys.getTotalUpdates("market1") == 0, "0 updates");
    assertTrue(approxEqual(sys.getVolatilityFactor("market1"), 0.05f),
               "Default volatility 0.05");
}

static void testRegionalMarketAddPrice() {
    std::cout << "\n=== RegionalMarket: AddPrice ===" << std::endl;
    ecs::World world;
    systems::RegionalMarketSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    assertTrue(sys.addRegionPrice("market1", "jita", "tritanium",
                                  100.0f, 50000.0f, 50000.0f, 1.0f),
               "Add Jita tritanium");
    assertTrue(sys.getTrackedItemCount("market1") == 1, "1 tracked item");
    assertTrue(approxEqual(sys.getAveragePrice("market1", "jita", "tritanium"), 100.0f),
               "Price 100.0");
    assertTrue(approxEqual(sys.getHubMultiplier("market1", "jita", "tritanium"), 1.0f),
               "Hub multiplier 1.0");

    // Add remote region with higher multiplier
    assertTrue(sys.addRegionPrice("market1", "outer_ring", "tritanium",
                                  100.0f, 10000.0f, 15000.0f, 1.5f),
               "Add remote tritanium");
    assertTrue(sys.getTrackedItemCount("market1") == 2, "2 tracked items");
    assertTrue(approxEqual(sys.getHubMultiplier("market1", "outer_ring", "tritanium"), 1.5f),
               "Remote hub multiplier 1.5");
}

static void testRegionalMarketRecordTrade() {
    std::cout << "\n=== RegionalMarket: RecordTrade ===" << std::endl;
    ecs::World world;
    systems::RegionalMarketSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");
    sys.addRegionPrice("market1", "jita", "tritanium",
                       100.0f, 50000.0f, 50000.0f);

    assertTrue(sys.recordTrade("market1", "jita", "tritanium", 100, 110.0f),
               "Record trade");
    assertTrue(sys.getTradeVolume("market1", "jita", "tritanium") == 100, "Volume 100");

    // Record another trade
    assertTrue(sys.recordTrade("market1", "jita", "tritanium", 50, 105.0f),
               "Record trade 2");
    assertTrue(sys.getTradeVolume("market1", "jita", "tritanium") == 150, "Volume 150");
}

static void testRegionalMarketPriceUpdate() {
    std::cout << "\n=== RegionalMarket: PriceUpdate ===" << std::endl;
    ecs::World world;
    systems::RegionalMarketSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    // High demand / low supply should increase price
    sys.addRegionPrice("market1", "jita", "mexallon",
                       100.0f, 1000.0f, 2000.0f, 1.0f);
    float initial = sys.getAveragePrice("market1", "jita", "mexallon");

    // Tick past update interval (60s)
    sys.update(65.0f);
    assertTrue(sys.getTotalUpdates("market1") == 1, "1 update");

    float updated = sys.getAveragePrice("market1", "jita", "mexallon");
    assertTrue(updated > initial, "Price increased with high demand");
}

static void testRegionalMarketVolatility() {
    std::cout << "\n=== RegionalMarket: Volatility ===" << std::endl;
    ecs::World world;
    systems::RegionalMarketSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    assertTrue(sys.setVolatilityFactor("market1", 0.1f), "Set volatility");
    assertTrue(approxEqual(sys.getVolatilityFactor("market1"), 0.1f), "Volatility 0.1");

    // Higher volatility means larger price swings
    sys.addRegionPrice("market1", "jita", "pyerite",
                       100.0f, 1000.0f, 2000.0f, 1.0f);
    sys.update(65.0f);
    float high_vol = sys.getAveragePrice("market1", "jita", "pyerite");

    // Compare: low volatility
    world.createEntity("market2");
    sys.initialize("market2");
    sys.setVolatilityFactor("market2", 0.01f);
    sys.addRegionPrice("market2", "jita", "pyerite",
                       100.0f, 1000.0f, 2000.0f, 1.0f);
    sys.update(65.0f);
    float low_vol = sys.getAveragePrice("market2", "jita", "pyerite");

    assertTrue(high_vol > low_vol, "Higher volatility = bigger price change");
}

static void testRegionalMarketHubMultiplier() {
    std::cout << "\n=== RegionalMarket: HubMultiplier ===" << std::endl;
    ecs::World world;
    systems::RegionalMarketSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    // Trade hub (multiplier 1.0) vs remote (multiplier 1.5)
    sys.addRegionPrice("market1", "jita", "isogen",
                       100.0f, 5000.0f, 5000.0f, 1.0f);
    sys.addRegionPrice("market1", "outer_ring", "isogen",
                       100.0f, 5000.0f, 5000.0f, 1.5f);

    assertTrue(approxEqual(sys.getHubMultiplier("market1", "jita", "isogen"), 1.0f),
               "Jita hub x1.0");
    assertTrue(approxEqual(sys.getHubMultiplier("market1", "outer_ring", "isogen"), 1.5f),
               "Outer Ring hub x1.5");
}

static void testRegionalMarketTradeNotFound() {
    std::cout << "\n=== RegionalMarket: TradeNotFound ===" << std::endl;
    ecs::World world;
    systems::RegionalMarketSystem sys(&world);
    world.createEntity("market1");
    sys.initialize("market1");

    assertTrue(!sys.recordTrade("market1", "unknown_region", "tritanium", 100, 110.0f),
               "Trade fails on unknown region/item");
    assertTrue(approxEqual(sys.getAveragePrice("market1", "unknown", "item"), 0.0f),
               "0 price on unknown");
    assertTrue(sys.getTradeVolume("market1", "unknown", "item") == 0,
               "0 volume on unknown");
}

static void testRegionalMarketMissing() {
    std::cout << "\n=== RegionalMarket: Missing ===" << std::endl;
    ecs::World world;
    systems::RegionalMarketSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.addRegionPrice("nonexistent", "r1", "item", 100.0f, 1000.0f, 1000.0f),
               "AddPrice fails on missing");
    assertTrue(!sys.recordTrade("nonexistent", "r1", "item", 100, 110.0f),
               "RecordTrade fails on missing");
    assertTrue(!sys.setVolatilityFactor("nonexistent", 0.1f), "SetVol fails on missing");
    assertTrue(sys.getTrackedItemCount("nonexistent") == 0, "0 items on missing");
    assertTrue(sys.getTotalUpdates("nonexistent") == 0, "0 updates on missing");
    assertTrue(approxEqual(sys.getAveragePrice("nonexistent", "r1", "item"), 0.0f),
               "0 price on missing");
    assertTrue(approxEqual(sys.getHubMultiplier("nonexistent", "r1", "item"), 0.0f),
               "0 hub on missing");
    assertTrue(sys.getTradeVolume("nonexistent", "r1", "item") == 0,
               "0 volume on missing");
    assertTrue(approxEqual(sys.getVolatilityFactor("nonexistent"), 0.0f),
               "0 volatility on missing");
}

void run_regional_market_system_tests() {
    testRegionalMarketCreate();
    testRegionalMarketAddPrice();
    testRegionalMarketRecordTrade();
    testRegionalMarketPriceUpdate();
    testRegionalMarketVolatility();
    testRegionalMarketHubMultiplier();
    testRegionalMarketTradeNotFound();
    testRegionalMarketMissing();
}
