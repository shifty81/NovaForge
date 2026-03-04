// Tests for: Market Order Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "components/ship_components.h"
#include "ecs/component.h"
#include "ecs/system.h"
#include "systems/fleet_system.h"
#include "systems/market_order_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== Market Order Tests ====================

static void testMarketOrderComponentDefaults() {
    std::cout << "\n=== Market Order Component Defaults ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("order1");
    auto* order = addComp<components::MarketOrder>(e);
    assertTrue(!order->is_filled, "Not filled");
    assertTrue(approxEqual(order->elapsed_time, 0.0f), "No elapsed time");
}

static void testMarketPlaceOrderSystem() {
    std::cout << "\n=== Market Place Order System ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("order2");
    addComp<components::MarketOrder>(e);

    systems::MarketOrderSystem sys(&world);
    sys.placeOrder("order2", components::MarketOrder::OrderType::Sell,
                   "Stellium", 1000, 5.0f, "region_a", "station_1", "player_1");

    auto* order = e->getComponent<components::MarketOrder>();
    assertTrue(order->item_type == "Stellium", "Item is Stellium");
    assertTrue(order->quantity == 1000, "Quantity 1000");
    assertTrue(order->quantity_remaining == 1000, "Remaining 1000");
    assertTrue(approxEqual(order->price_per_unit, 5.0f), "Price 5.0");
}

static void testMarketFillOrderSystem() {
    std::cout << "\n=== Market Fill Order System ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("order3");
    addComp<components::MarketOrder>(e);

    systems::MarketOrderSystem sys(&world);
    sys.placeOrder("order3", components::MarketOrder::OrderType::Buy,
                   "Vanthium", 500, 10.0f, "region_b", "station_2", "player_2");

    int filled = sys.fillOrder("order3", 200);
    assertTrue(filled == 200, "Filled 200");
    auto* order = e->getComponent<components::MarketOrder>();
    assertTrue(order->quantity_remaining == 300, "300 remaining");
    assertTrue(!order->is_filled, "Not fully filled yet");

    int filled2 = sys.fillOrder("order3", 300);
    assertTrue(filled2 == 300, "Filled remaining 300");
    assertTrue(order->is_filled, "Now fully filled");
}

static void testMarketOrderExpirySystem() {
    std::cout << "\n=== Market Order Expiry System ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("order4");
    auto* order = addComp<components::MarketOrder>(e);
    order->expiry_time = 100.0f;

    systems::MarketOrderSystem sys(&world);
    sys.placeOrder("order4", components::MarketOrder::OrderType::Sell,
                   "Cydrium", 100, 20.0f, "region_c", "station_3", "npc_1");

    assertTrue(!sys.isOrderExpired("order4"), "Not expired initially");
    sys.update(101.0f);
    assertTrue(sys.isOrderExpired("order4"), "Expired after 101s");
}

static void testAIFleetDispatchSystem() {
    std::cout << "\n=== AI Fleet Dispatch ===" << std::endl;
    ecs::World world;
    auto* e = world.createEntity("order5");
    addComp<components::MarketOrder>(e);

    systems::MarketOrderSystem sys(&world);
    std::string dispatch_id = sys.dispatchAIFleet("order5",
        components::AIFleetDispatch::DispatchType::Hauling, "system_alpha", 2);
    assertTrue(!dispatch_id.empty(), "Dispatch created");

    auto dispatches = sys.getActiveDispatches();
    assertTrue(static_cast<int>(dispatches.size()) == 1, "One active dispatch");

    auto* de = world.getEntity(dispatch_id);
    auto* dispatch = de->getComponent<components::AIFleetDispatch>();
    assertTrue(!dispatch->isComplete(), "Not complete yet");

    // Simulate enough time for completion
    sys.update(dispatch->estimated_completion + 1.0f);
    assertTrue(dispatch->isComplete(), "Complete after time");
}


void run_market_order_tests() {
    testMarketOrderComponentDefaults();
    testMarketPlaceOrderSystem();
    testMarketFillOrderSystem();
    testMarketOrderExpirySystem();
    testAIFleetDispatchSystem();
}
