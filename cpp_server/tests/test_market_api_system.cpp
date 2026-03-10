// Tests for: MarketApiSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/market_api_system.h"

using namespace atlas;

// ==================== MarketApiSystem Tests ====================

static void testMarketApiInit() {
    std::cout << "\n=== MarketApi: Init ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    assertTrue(sys.initialize("api1", "jita_region"), "Init succeeds");
    assertTrue(sys.getSubscriptionCount("api1") == 0, "0 subscriptions");
    assertTrue(sys.getPendingRequestCount("api1") == 0, "0 pending requests");
    assertTrue(sys.getTotalRequests("api1") == 0, "0 total requests");
    assertTrue(sys.getHistoryCount("api1") == 0, "0 history entries");
    assertTrue(sys.getTotalPushes("api1") == 0, "0 total pushes");
    assertTrue(approxEqual(sys.getPushInterval("api1"), 5.0f), "Default push interval 5s");
}

static void testMarketApiSubscribe() {
    std::cout << "\n=== MarketApi: Subscribe ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");

    assertTrue(sys.subscribe("api1", "client_a", "tritanium"), "Subscribe client_a to tritanium");
    assertTrue(sys.subscribe("api1", "client_a", "pyerite"), "Subscribe client_a to pyerite");
    assertTrue(sys.subscribe("api1", "client_b", "tritanium"), "Subscribe client_b to tritanium");
    assertTrue(sys.getSubscriptionCount("api1") == 3, "3 subscriptions");
    assertTrue(sys.isSubscribed("api1", "client_a", "tritanium"), "client_a subscribed to tritanium");
    assertTrue(!sys.isSubscribed("api1", "client_a", "mexallon"), "client_a not subscribed to mexallon");

    // Duplicate subscription rejected
    assertTrue(!sys.subscribe("api1", "client_a", "tritanium"), "Duplicate subscription rejected");
    assertTrue(sys.getSubscriptionCount("api1") == 3, "Still 3 after duplicate");
}

static void testMarketApiUnsubscribe() {
    std::cout << "\n=== MarketApi: Unsubscribe ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");
    sys.subscribe("api1", "client_a", "tritanium");
    sys.subscribe("api1", "client_b", "pyerite");

    assertTrue(sys.unsubscribe("api1", "client_a", "tritanium"), "Unsubscribe succeeds");
    assertTrue(sys.getSubscriptionCount("api1") == 1, "1 subscription left");
    assertTrue(!sys.isSubscribed("api1", "client_a", "tritanium"), "client_a unsubscribed");
    assertTrue(!sys.unsubscribe("api1", "client_a", "tritanium"), "Double unsubscribe fails");
    assertTrue(!sys.unsubscribe("api1", "client_z", "nonexistent"), "Missing unsubscribe fails");
}

static void testMarketApiSubscribeMaxLimit() {
    std::cout << "\n=== MarketApi: SubscribeMaxLimit ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");
    auto* entity = world.getEntity("api1");
    auto* comp = entity->getComponent<components::MarketApiState>();
    comp->max_subscriptions = 2;

    sys.subscribe("api1", "c1", "item_a");
    sys.subscribe("api1", "c2", "item_b");
    assertTrue(!sys.subscribe("api1", "c3", "item_c"), "Max subscriptions enforced");
    assertTrue(sys.getSubscriptionCount("api1") == 2, "Still 2");
}

static void testMarketApiSubmitRequest() {
    std::cout << "\n=== MarketApi: SubmitRequest ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");

    assertTrue(sys.submitRequest("api1", "client_a", "sell_orders", "tritanium"), "Submit sell_orders request");
    assertTrue(sys.submitRequest("api1", "client_a", "buy_orders", "tritanium"), "Submit buy_orders request");
    assertTrue(sys.submitRequest("api1", "client_b", "history", "pyerite"), "Submit history request");
    assertTrue(sys.getTotalRequests("api1") == 3, "3 total requests");
    assertTrue(sys.getPendingRequestCount("api1") == 3, "3 pending");

    // After a tick, requests are fulfilled
    sys.update(0.1f);
    assertTrue(sys.getPendingRequestCount("api1") == 0, "0 pending after tick");
}

static void testMarketApiEmptyRequest() {
    std::cout << "\n=== MarketApi: EmptyRequest ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");

    assertTrue(!sys.submitRequest("api1", "", "sell_orders", "item"), "Empty client_id rejected");
    assertTrue(!sys.submitRequest("api1", "c1", "", "item"), "Empty request_type rejected");
    assertTrue(sys.getTotalRequests("api1") == 0, "0 requests");
}

static void testMarketApiRecordSnapshot() {
    std::cout << "\n=== MarketApi: RecordSnapshot ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");

    assertTrue(sys.recordSnapshot("api1", 95.0f, 100.0f, 5000.0f), "Record snapshot 1");
    assertTrue(sys.recordSnapshot("api1", 96.0f, 101.0f, 4800.0f), "Record snapshot 2");
    assertTrue(sys.recordSnapshot("api1", 94.0f, 99.0f, 5200.0f), "Record snapshot 3");
    assertTrue(sys.getHistoryCount("api1") == 3, "3 history entries");
}

static void testMarketApiHistoryMaxLimit() {
    std::cout << "\n=== MarketApi: HistoryMaxLimit ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");
    auto* entity = world.getEntity("api1");
    auto* comp = entity->getComponent<components::MarketApiState>();
    comp->max_history = 5;

    for (int i = 0; i < 8; ++i) {
        sys.recordSnapshot("api1", 90.0f + i, 100.0f + i, 1000.0f);
    }
    assertTrue(sys.getHistoryCount("api1") == 5, "History capped at 5");
}

static void testMarketApiPushInterval() {
    std::cout << "\n=== MarketApi: PushInterval ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");
    sys.subscribe("api1", "client_a", "tritanium");

    assertTrue(sys.setPushInterval("api1", 10.0f), "Set push interval 10s");
    assertTrue(approxEqual(sys.getPushInterval("api1"), 10.0f), "Push interval is 10s");
    assertTrue(!sys.setPushInterval("api1", 0.0f), "Zero interval rejected");
    assertTrue(!sys.setPushInterval("api1", -1.0f), "Negative interval rejected");

    // No push before interval
    sys.update(5.0f);
    assertTrue(sys.getTotalPushes("api1") == 0, "0 pushes before interval");

    // Push fires after interval
    sys.update(6.0f);
    assertTrue(sys.getTotalPushes("api1") == 1, "1 push after interval");
}

static void testMarketApiPushUpdatesSubscriptions() {
    std::cout << "\n=== MarketApi: PushUpdatesSubscriptions ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);
    world.createEntity("api1");
    sys.initialize("api1");
    sys.subscribe("api1", "c1", "tritanium");
    sys.subscribe("api1", "c2", "pyerite");

    // Tick past two push intervals (5s each)
    sys.update(5.1f);
    sys.update(5.1f);
    assertTrue(sys.getTotalPushes("api1") == 2, "2 total pushes");

    // Verify subscription snapshot counts incremented
    auto* entity = world.getEntity("api1");
    auto* comp = entity->getComponent<components::MarketApiState>();
    assertTrue(comp->subscriptions[0].snapshot_count == 2, "c1 got 2 snapshots");
    assertTrue(comp->subscriptions[1].snapshot_count == 2, "c2 got 2 snapshots");
}

static void testMarketApiMissing() {
    std::cout << "\n=== MarketApi: Missing ===" << std::endl;
    ecs::World world;
    systems::MarketApiSystem sys(&world);

    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
    assertTrue(!sys.subscribe("nonexistent", "c1", "item"), "Subscribe fails on missing");
    assertTrue(!sys.unsubscribe("nonexistent", "c1", "item"), "Unsubscribe fails on missing");
    assertTrue(!sys.isSubscribed("nonexistent", "c1", "item"), "IsSubscribed false on missing");
    assertTrue(sys.getSubscriptionCount("nonexistent") == 0, "0 subs on missing");
    assertTrue(!sys.submitRequest("nonexistent", "c1", "sell_orders", "item"), "Request fails on missing");
    assertTrue(sys.getTotalRequests("nonexistent") == 0, "0 requests on missing");
    assertTrue(!sys.recordSnapshot("nonexistent", 100.0f, 110.0f, 1000.0f), "Snapshot fails on missing");
    assertTrue(sys.getHistoryCount("nonexistent") == 0, "0 history on missing");
    assertTrue(approxEqual(sys.getPushInterval("nonexistent"), 0.0f), "0 interval on missing");
}

void run_market_api_system_tests() {
    testMarketApiInit();
    testMarketApiSubscribe();
    testMarketApiUnsubscribe();
    testMarketApiSubscribeMaxLimit();
    testMarketApiSubmitRequest();
    testMarketApiEmptyRequest();
    testMarketApiRecordSnapshot();
    testMarketApiHistoryMaxLimit();
    testMarketApiPushInterval();
    testMarketApiPushUpdatesSubscriptions();
    testMarketApiMissing();
}
