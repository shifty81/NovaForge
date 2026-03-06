// Tests for: Trade Route Analytics System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/trade_route_analytics_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Trade Route Analytics System Tests ====================

static void testTradeRouteAnalyticsCreate() {
    std::cout << "\n=== TradeRouteAnalytics: Create ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    assertTrue(sys.initialize("tra1"), "Init succeeds");
    assertTrue(sys.getRouteCount("tra1") == 0, "No routes initially");
    assertTrue(approxEqual(sys.getTotalVolume("tra1"), 0.0f), "Total volume is 0");
    assertTrue(approxEqual(sys.getTotalRevenue("tra1"), 0.0f), "Total revenue is 0");
    assertTrue(sys.getTotalSnapshots("tra1") == 0, "No snapshots initially");
}

static void testTradeRouteAnalyticsRegister() {
    std::cout << "\n=== TradeRouteAnalytics: Register ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");
    assertTrue(sys.registerRoute("tra1", "r1", "Jita", "Amarr", "Tritanium"), "Register route 1");
    assertTrue(sys.registerRoute("tra1", "r2", "Amarr", "Dodixie", "Pyerite"), "Register route 2");
    assertTrue(sys.getRouteCount("tra1") == 2, "2 routes registered");
}

static void testTradeRouteAnalyticsDuplicate() {
    std::cout << "\n=== TradeRouteAnalytics: Duplicate ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");
    sys.registerRoute("tra1", "r1", "Jita", "Amarr", "Tritanium");
    assertTrue(!sys.registerRoute("tra1", "r1", "Jita", "Rens", "Mexallon"), "Duplicate route rejected");
}

static void testTradeRouteAnalyticsRecordTrip() {
    std::cout << "\n=== TradeRouteAnalytics: RecordTrip ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");
    sys.registerRoute("tra1", "r1", "Jita", "Amarr", "Tritanium");
    assertTrue(sys.recordTrip("tra1", "r1", 1000.0f, 5000.0f, 3000.0f), "Record trip succeeds");
    assertTrue(approxEqual(sys.getRouteVolume("tra1", "r1"), 1000.0f), "Volume is 1000");
    assertTrue(approxEqual(sys.getRouteRevenue("tra1", "r1"), 5000.0f), "Revenue is 5000");
    assertTrue(approxEqual(sys.getTotalVolume("tra1"), 1000.0f), "Total volume 1000");
    assertTrue(approxEqual(sys.getTotalRevenue("tra1"), 5000.0f), "Total revenue 5000");
    assertTrue(!sys.recordTrip("tra1", "nonexistent", 100.0f, 200.0f, 100.0f), "Trip on missing route fails");
}

static void testTradeRouteAnalyticsCongestion() {
    std::cout << "\n=== TradeRouteAnalytics: Congestion ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");
    sys.registerRoute("tra1", "r1", "Jita", "Amarr", "Tritanium");
    assertTrue(sys.updateCongestion("tra1", "r1", 0.7f), "Set congestion succeeds");
    assertTrue(sys.updateCongestion("tra1", "r1", 1.5f), "Clamped congestion accepted");
    assertTrue(!sys.updateCongestion("tra1", "nonexistent", 0.5f), "Missing route fails");
}

static void testTradeRouteAnalyticsProfitMargin() {
    std::cout << "\n=== TradeRouteAnalytics: ProfitMargin ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");
    sys.registerRoute("tra1", "r1", "Jita", "Amarr", "Tritanium");
    sys.recordTrip("tra1", "r1", 1000.0f, 10000.0f, 6000.0f);
    float margin = sys.getRouteProfitMargin("tra1", "r1");
    assertTrue(approxEqual(margin, 0.4f), "Profit margin is 40%");
    assertTrue(approxEqual(sys.getRouteProfitMargin("tra1", "nonexistent"), 0.0f), "Missing route returns 0");
}

static void testTradeRouteAnalyticsMostProfitable() {
    std::cout << "\n=== TradeRouteAnalytics: MostProfitable ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");
    sys.registerRoute("tra1", "r1", "Jita", "Amarr", "Tritanium");
    sys.registerRoute("tra1", "r2", "Amarr", "Dodixie", "Pyerite");
    sys.recordTrip("tra1", "r1", 500.0f, 3000.0f, 2000.0f);
    sys.recordTrip("tra1", "r2", 800.0f, 8000.0f, 2000.0f);
    assertTrue(sys.getMostProfitableRoute("tra1") == "r2", "Route 2 is most profitable");
}

static void testTradeRouteAnalyticsRemove() {
    std::cout << "\n=== TradeRouteAnalytics: Remove ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");
    sys.registerRoute("tra1", "r1", "Jita", "Amarr", "Tritanium");
    assertTrue(sys.removeRoute("tra1", "r1"), "Remove succeeds");
    assertTrue(sys.getRouteCount("tra1") == 0, "0 routes after remove");
    assertTrue(!sys.removeRoute("tra1", "r1"), "Double remove fails");
}

static void testTradeRouteAnalyticsSnapshot() {
    std::cout << "\n=== TradeRouteAnalytics: Snapshot ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");

    auto* entity = world.getEntity("tra1");
    auto* tra = entity->getComponent<components::TradeRouteAnalytics>();
    tra->snapshot_interval = 10.0f;

    sys.update(5.0f);
    assertTrue(sys.getTotalSnapshots("tra1") == 0, "No snapshot at 5s");
    sys.update(6.0f);
    assertTrue(sys.getTotalSnapshots("tra1") == 1, "1 snapshot at 11s");
    sys.update(10.0f);
    assertTrue(sys.getTotalSnapshots("tra1") == 2, "2 snapshots at 21s");
}

static void testTradeRouteAnalyticsMaxLimit() {
    std::cout << "\n=== TradeRouteAnalytics: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    world.createEntity("tra1");
    sys.initialize("tra1");

    auto* entity = world.getEntity("tra1");
    auto* tra = entity->getComponent<components::TradeRouteAnalytics>();
    tra->max_routes = 2;

    sys.registerRoute("tra1", "r1", "Jita", "Amarr", "Tritanium");
    sys.registerRoute("tra1", "r2", "Amarr", "Dodixie", "Pyerite");
    assertTrue(!sys.registerRoute("tra1", "r3", "Dodixie", "Rens", "Isogen"), "Max routes enforced");
}

static void testTradeRouteAnalyticsMissing() {
    std::cout << "\n=== TradeRouteAnalytics: Missing ===" << std::endl;
    ecs::World world;
    systems::TradeRouteAnalyticsSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing");
    assertTrue(!sys.registerRoute("nonexistent", "r1", "A", "B", "C"), "Register fails on missing");
    assertTrue(!sys.recordTrip("nonexistent", "r1", 1.0f, 2.0f, 1.0f), "RecordTrip fails on missing");
    assertTrue(!sys.updateCongestion("nonexistent", "r1", 0.5f), "Congestion fails on missing");
    assertTrue(!sys.removeRoute("nonexistent", "r1"), "Remove fails on missing");
    assertTrue(sys.getRouteCount("nonexistent") == 0, "0 routes on missing");
    assertTrue(approxEqual(sys.getRouteVolume("nonexistent", "r1"), 0.0f), "0 volume on missing");
    assertTrue(approxEqual(sys.getRouteRevenue("nonexistent", "r1"), 0.0f), "0 revenue on missing");
    assertTrue(approxEqual(sys.getRouteProfitMargin("nonexistent", "r1"), 0.0f), "0 margin on missing");
    assertTrue(sys.getMostProfitableRoute("nonexistent") == "", "Empty most profitable on missing");
    assertTrue(approxEqual(sys.getTotalVolume("nonexistent"), 0.0f), "0 total vol on missing");
    assertTrue(approxEqual(sys.getTotalRevenue("nonexistent"), 0.0f), "0 total rev on missing");
    assertTrue(sys.getTotalSnapshots("nonexistent") == 0, "0 snapshots on missing");
}


void run_trade_route_analytics_system_tests() {
    testTradeRouteAnalyticsCreate();
    testTradeRouteAnalyticsRegister();
    testTradeRouteAnalyticsDuplicate();
    testTradeRouteAnalyticsRecordTrip();
    testTradeRouteAnalyticsCongestion();
    testTradeRouteAnalyticsProfitMargin();
    testTradeRouteAnalyticsMostProfitable();
    testTradeRouteAnalyticsRemove();
    testTradeRouteAnalyticsSnapshot();
    testTradeRouteAnalyticsMaxLimit();
    testTradeRouteAnalyticsMissing();
}
