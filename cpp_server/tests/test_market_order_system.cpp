// Tests for: MarketOrderSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/market_order_system.h"

using namespace atlas;

// ==================== MarketOrderSystem Tests ====================

static void testMarketOrderCreate() {
    std::cout << "\n=== MarketOrder: Create ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());

    sys.placeOrder("order1", components::MarketOrder::OrderType::Buy,
                   "tritanium", 100, 5.0f, "region_a", "station_1", "player1");

    auto* order = entity->getComponent<components::MarketOrder>();
    assertTrue(order != nullptr, "Order component exists");
    assertTrue(order->type == components::MarketOrder::OrderType::Buy, "Type is Buy");
    assertTrue(order->item_type == "tritanium", "Item is tritanium");
    assertTrue(order->quantity == 100, "Quantity is 100");
    assertTrue(order->quantity_remaining == 100, "Remaining is 100");
    assertTrue(approxEqual(order->price_per_unit, 5.0f), "Price is 5.0");
    assertTrue(order->region_id == "region_a", "Region is region_a");
    assertTrue(order->station_id == "station_1", "Station is station_1");
    assertTrue(order->owner_id == "player1", "Owner is player1");
    assertTrue(!order->is_filled, "Not filled");
}

static void testMarketOrderSellOrder() {
    std::cout << "\n=== MarketOrder: SellOrder ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());

    sys.placeOrder("order1", components::MarketOrder::OrderType::Sell,
                   "pyerite", 50, 10.0f, "region_b", "station_2", "npc_miner");

    auto* order = entity->getComponent<components::MarketOrder>();
    assertTrue(order->type == components::MarketOrder::OrderType::Sell, "Type is Sell");
    assertTrue(order->item_type == "pyerite", "Item is pyerite");
    assertTrue(order->quantity == 50, "Quantity is 50");
}

static void testMarketOrderMissingEntity() {
    std::cout << "\n=== MarketOrder: MissingEntity ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);

    // Placing order on missing entity is a no-op (no crash)
    sys.placeOrder("missing", components::MarketOrder::OrderType::Buy,
                   "item", 10, 1.0f, "r", "s", "o");
    assertTrue(true, "Place on missing entity doesn't crash");

    assertTrue(!sys.cancelOrder("missing"), "Cancel on missing entity fails");
    assertTrue(sys.fillOrder("missing", 10) == 0, "Fill on missing entity returns 0");
    assertTrue(!sys.isOrderExpired("missing"), "Expired on missing entity returns false");
}

static void testMarketOrderMissingComponent() {
    std::cout << "\n=== MarketOrder: MissingComponent ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    world.createEntity("entity1"); // no MarketOrder component

    sys.placeOrder("entity1", components::MarketOrder::OrderType::Buy,
                   "item", 10, 1.0f, "r", "s", "o");
    assertTrue(true, "Place on entity without component doesn't crash");

    assertTrue(!sys.cancelOrder("entity1"), "Cancel without component fails");
    assertTrue(sys.fillOrder("entity1", 10) == 0, "Fill without component returns 0");
}

static void testMarketOrderCancel() {
    std::cout << "\n=== MarketOrder: Cancel ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());

    sys.placeOrder("order1", components::MarketOrder::OrderType::Buy,
                   "tritanium", 100, 5.0f, "region_a", "station_1", "player1");

    assertTrue(sys.cancelOrder("order1"), "Cancel succeeds");

    auto* order = entity->getComponent<components::MarketOrder>();
    assertTrue(order->is_filled, "Order marked as filled after cancel");
    assertTrue(order->quantity_remaining == 0, "Remaining is 0 after cancel");
}

static void testMarketOrderFillPartial() {
    std::cout << "\n=== MarketOrder: FillPartial ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());

    sys.placeOrder("order1", components::MarketOrder::OrderType::Sell,
                   "mexallon", 100, 20.0f, "region_a", "station_1", "player1");

    int filled = sys.fillOrder("order1", 30);
    assertTrue(filled == 30, "Filled 30 units");

    auto* order = entity->getComponent<components::MarketOrder>();
    assertTrue(order->quantity_remaining == 70, "70 remaining");
    assertTrue(!order->is_filled, "Not fully filled yet");
}

static void testMarketOrderFillFull() {
    std::cout << "\n=== MarketOrder: FillFull ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());

    sys.placeOrder("order1", components::MarketOrder::OrderType::Buy,
                   "isogen", 50, 15.0f, "region_a", "station_1", "player1");

    int filled = sys.fillOrder("order1", 50);
    assertTrue(filled == 50, "Filled all 50 units");

    auto* order = entity->getComponent<components::MarketOrder>();
    assertTrue(order->quantity_remaining == 0, "0 remaining");
    assertTrue(order->is_filled, "Order is now filled");

    // Can't fill a filled order
    int extra = sys.fillOrder("order1", 10);
    assertTrue(extra == 0, "Cannot fill already-filled order");
}

static void testMarketOrderFillOverflow() {
    std::cout << "\n=== MarketOrder: FillOverflow ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());

    sys.placeOrder("order1", components::MarketOrder::OrderType::Sell,
                   "nocxium", 20, 50.0f, "region_a", "station_1", "player1");

    int filled = sys.fillOrder("order1", 100); // more than available
    assertTrue(filled == 20, "Capped at remaining quantity 20");

    auto* order = entity->getComponent<components::MarketOrder>();
    assertTrue(order->is_filled, "Order filled after overflow fill");
}

static void testMarketOrderGetOrdersForRegion() {
    std::cout << "\n=== MarketOrder: GetOrdersForRegion ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);

    // Create orders in different regions
    auto* e1 = world.createEntity("order1");
    e1->addComponent(std::make_unique<components::MarketOrder>());
    sys.placeOrder("order1", components::MarketOrder::OrderType::Buy,
                   "trit", 100, 5.0f, "region_a", "s1", "p1");

    auto* e2 = world.createEntity("order2");
    e2->addComponent(std::make_unique<components::MarketOrder>());
    sys.placeOrder("order2", components::MarketOrder::OrderType::Sell,
                   "pye", 50, 10.0f, "region_a", "s2", "p2");

    auto* e3 = world.createEntity("order3");
    e3->addComponent(std::make_unique<components::MarketOrder>());
    sys.placeOrder("order3", components::MarketOrder::OrderType::Buy,
                   "mex", 200, 20.0f, "region_b", "s3", "p3");

    auto region_a_orders = sys.getOrdersForRegion("region_a");
    assertTrue(static_cast<int>(region_a_orders.size()) == 2, "2 orders in region_a");

    auto region_b_orders = sys.getOrdersForRegion("region_b");
    assertTrue(static_cast<int>(region_b_orders.size()) == 1, "1 order in region_b");

    auto empty_orders = sys.getOrdersForRegion("region_c");
    assertTrue(empty_orders.empty(), "No orders in region_c");
}

static void testMarketOrderDispatchAIFleet() {
    std::cout << "\n=== MarketOrder: DispatchAIFleet ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());

    sys.placeOrder("order1", components::MarketOrder::OrderType::Buy,
                   "trit", 100, 5.0f, "region_a", "s1", "p1");

    std::string dispatch_id = sys.dispatchAIFleet("order1",
        components::AIFleetDispatch::DispatchType::Hauling, "system_b", 3);
    assertTrue(!dispatch_id.empty(), "Dispatch ID returned");

    auto dispatches = sys.getActiveDispatches();
    assertTrue(static_cast<int>(dispatches.size()) == 1, "1 active dispatch");

    // Advance time past completion (estimated = 60 * fleet_size = 180s)
    sys.update(200.0f);
    dispatches = sys.getActiveDispatches();
    assertTrue(dispatches.empty(), "Dispatch completed after time elapsed");
}

static void testMarketOrderExpiry() {
    std::cout << "\n=== MarketOrder: Expiry ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());

    sys.placeOrder("order1", components::MarketOrder::OrderType::Buy,
                   "trit", 100, 5.0f, "region_a", "s1", "p1");

    assertTrue(!sys.isOrderExpired("order1"), "Not expired initially");

    // Default expiry is 86400s (24h). Advance past it.
    sys.update(86401.0f);
    assertTrue(sys.isOrderExpired("order1"), "Expired after 24h");
}

static void testMarketOrderUpdate() {
    std::cout << "\n=== MarketOrder: Update ===" << std::endl;
    ecs::World world;
    systems::MarketOrderSystem sys(&world);
    auto* entity = world.createEntity("order1");
    entity->addComponent(std::make_unique<components::MarketOrder>());
    sys.placeOrder("order1", components::MarketOrder::OrderType::Buy,
                   "trit", 100, 5.0f, "region_a", "s1", "p1");

    sys.update(10.0f);
    auto* order = entity->getComponent<components::MarketOrder>();
    assertTrue(approxEqual(order->elapsed_time, 10.0f), "Elapsed time updated to 10");
}

void run_market_order_system_tests() {
    testMarketOrderCreate();
    testMarketOrderSellOrder();
    testMarketOrderMissingEntity();
    testMarketOrderMissingComponent();
    testMarketOrderCancel();
    testMarketOrderFillPartial();
    testMarketOrderFillFull();
    testMarketOrderFillOverflow();
    testMarketOrderGetOrdersForRegion();
    testMarketOrderDispatchAIFleet();
    testMarketOrderExpiry();
    testMarketOrderUpdate();
}
