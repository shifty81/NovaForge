// Tests for: MarketWatchlistSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/market_watchlist_system.h"

using namespace atlas;

// ==================== MarketWatchlistSystem Tests ====================

static void testMarketWatchlistInit() {
    std::cout << "\n=== MarketWatchlist: Init ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1"), "Init succeeds");
    assertTrue(sys.getEntryCount("p1") == 0, "Zero entries initially");
    assertTrue(sys.getPendingAlertCount("p1") == 0, "Zero pending alerts initially");
    assertTrue(sys.getTotalAlertsFired("p1") == 0, "Zero total alerts initially");
    assertTrue(!sys.initialize("nonexistent"), "Init fails on missing entity");
}

static void testMarketWatchlistAddEntry() {
    std::cout << "\n=== MarketWatchlist: AddEntry ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    assertTrue(sys.addEntry("p1", "tritanium", "Tritanium", 5.50f), "Add first entry");
    assertTrue(sys.addEntry("p1", "plex", "PLEX", 1500000.0f), "Add second entry");
    assertTrue(sys.getEntryCount("p1") == 2, "Two entries stored");
    assertTrue(sys.hasEntry("p1", "tritanium"), "Has tritanium entry");
    assertTrue(sys.hasEntry("p1", "plex"), "Has plex entry");
    assertTrue(!sys.hasEntry("p1", "unknown"), "Unknown entry not found");

    assertTrue(!sys.addEntry("p1", "tritanium", "Duplicate", 5.0f), "Duplicate id rejected");
    assertTrue(!sys.addEntry("p1", "", "Empty ID", 1.0f), "Empty id rejected");
    assertTrue(sys.getEntryCount("p1") == 2, "Count unchanged after rejections");
}

static void testMarketWatchlistRemoveEntry() {
    std::cout << "\n=== MarketWatchlist: RemoveEntry ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "tritanium", "Tritanium", 5.50f);
    sys.addEntry("p1", "plex", "PLEX", 1500000.0f);

    assertTrue(sys.removeEntry("p1", "tritanium"), "Remove existing entry");
    assertTrue(sys.getEntryCount("p1") == 1, "One entry remaining");
    assertTrue(!sys.hasEntry("p1", "tritanium"), "Tritanium gone");
    assertTrue(!sys.removeEntry("p1", "tritanium"), "Remove nonexistent fails");
    assertTrue(!sys.removeEntry("p1", "unknown"), "Remove unknown fails");
}

static void testMarketWatchlistClear() {
    std::cout << "\n=== MarketWatchlist: Clear ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "item1", "Item 1", 100.0f);
    sys.addEntry("p1", "item2", "Item 2", 200.0f);

    assertTrue(sys.clearWatchlist("p1"), "Clear succeeds");
    assertTrue(sys.getEntryCount("p1") == 0, "Zero entries after clear");
}

static void testMarketWatchlistMaxEntries() {
    std::cout << "\n=== MarketWatchlist: MaxEntries ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");

    // Default max is 30
    for (int i = 0; i < 30; i++) {
        std::string id = "item" + std::to_string(i);
        sys.addEntry("p1", id, "Item", static_cast<float>(i));
    }
    assertTrue(sys.getEntryCount("p1") == 30, "30 entries added");
    assertTrue(!sys.addEntry("p1", "item30", "Over limit", 1.0f), "Blocked at max");
    assertTrue(sys.getEntryCount("p1") == 30, "Count still 30");
}

static void testMarketWatchlistBuyAlert() {
    std::cout << "\n=== MarketWatchlist: BuyAlert ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "tritanium", "Tritanium", 5.50f);

    // Buy alert fires when price drops AT OR BELOW threshold
    assertTrue(sys.setBuyThreshold("p1", "tritanium", 5.0f), "Set buy threshold");
    assertTrue(!sys.hasBuyAlert("p1", "tritanium"), "No alert yet");
    assertTrue(sys.getPendingAlertCount("p1") == 0, "No pending alerts");

    // Price drop triggers alert
    assertTrue(sys.updatePrice("p1", "tritanium", 4.80f), "Update price below threshold");
    assertTrue(sys.hasBuyAlert("p1", "tritanium"), "Buy alert fired");
    assertTrue(sys.getPendingAlertCount("p1") == 1, "1 pending alert");
    assertTrue(sys.getTotalAlertsFired("p1") == 1, "1 total alert fired");

    // Alert is one-shot: further price drops don't fire again
    assertTrue(sys.updatePrice("p1", "tritanium", 4.50f), "Update price further");
    assertTrue(sys.getTotalAlertsFired("p1") == 1, "Total still 1 (one-shot)");
}

static void testMarketWatchlistSellAlert() {
    std::cout << "\n=== MarketWatchlist: SellAlert ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "plex", "PLEX", 1500000.0f);

    // Sell alert fires when price rises AT OR ABOVE threshold
    assertTrue(sys.setSellThreshold("p1", "plex", 2000000.0f), "Set sell threshold");
    assertTrue(!sys.hasSellAlert("p1", "plex"), "No alert yet");

    // Price below threshold — no alert
    assertTrue(sys.updatePrice("p1", "plex", 1800000.0f), "Update price still below");
    assertTrue(!sys.hasSellAlert("p1", "plex"), "No alert below threshold");

    // Price at threshold fires alert
    assertTrue(sys.updatePrice("p1", "plex", 2000000.0f), "Update price at threshold");
    assertTrue(sys.hasSellAlert("p1", "plex"), "Sell alert fired at threshold");
    assertTrue(sys.getTotalAlertsFired("p1") == 1, "1 total alert");
}

static void testMarketWatchlistBothAlerts() {
    std::cout << "\n=== MarketWatchlist: BothAlerts ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "item1", "Item 1", 100.0f);
    sys.addEntry("p1", "item2", "Item 2", 200.0f);

    sys.setBuyThreshold("p1", "item1", 90.0f);
    sys.setSellThreshold("p1", "item2", 250.0f);

    sys.updatePrice("p1", "item1", 85.0f);   // fires buy alert
    sys.updatePrice("p1", "item2", 260.0f);  // fires sell alert

    assertTrue(sys.getPendingAlertCount("p1") == 2, "2 pending alerts");
    assertTrue(sys.getTotalAlertsFired("p1") == 2, "2 total alerts fired");
}

static void testMarketWatchlistAcknowledge() {
    std::cout << "\n=== MarketWatchlist: Acknowledge ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "item1", "Item 1", 100.0f);
    sys.addEntry("p1", "item2", "Item 2", 200.0f);
    sys.setBuyThreshold("p1", "item1", 90.0f);
    sys.setBuyThreshold("p1", "item2", 180.0f);
    sys.updatePrice("p1", "item1", 85.0f);
    sys.updatePrice("p1", "item2", 175.0f);

    assertTrue(sys.getPendingAlertCount("p1") == 2, "2 pending alerts before ack");
    assertTrue(sys.acknowledgeAlert("p1", "item1"), "Acknowledge item1 alert");
    assertTrue(sys.getPendingAlertCount("p1") == 1, "1 pending after ack item1");
    assertTrue(!sys.hasBuyAlert("p1", "item1"), "Item1 alert cleared");
    assertTrue(sys.hasBuyAlert("p1", "item2"), "Item2 still pending");

    assertTrue(sys.acknowledgeAllAlerts("p1"), "Acknowledge all alerts");
    assertTrue(sys.getPendingAlertCount("p1") == 0, "0 pending after ack all");
    // total_alerts_fired is cumulative — not reset by acknowledge
    assertTrue(sys.getTotalAlertsFired("p1") == 2, "Total alerts still 2");
}

static void testMarketWatchlistClearThresholds() {
    std::cout << "\n=== MarketWatchlist: ClearThresholds ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "item1", "Item 1", 100.0f);
    sys.setBuyThreshold("p1", "item1", 90.0f);
    sys.setSellThreshold("p1", "item1", 120.0f);
    sys.updatePrice("p1", "item1", 80.0f);
    assertTrue(sys.hasBuyAlert("p1", "item1"), "Buy alert fired");

    assertTrue(sys.clearBuyThreshold("p1", "item1"), "Clear buy threshold");
    assertTrue(!sys.hasBuyAlert("p1", "item1"), "Alert cleared with threshold");
    // Sell threshold still active
    sys.updatePrice("p1", "item1", 125.0f);
    assertTrue(sys.hasSellAlert("p1", "item1"), "Sell alert still fires");

    assertTrue(sys.clearSellThreshold("p1", "item1"), "Clear sell threshold");
    assertTrue(!sys.hasSellAlert("p1", "item1"), "Sell alert cleared");
}

static void testMarketWatchlistInvalidThreshold() {
    std::cout << "\n=== MarketWatchlist: InvalidThreshold ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "item1", "Item 1", 100.0f);

    assertTrue(!sys.setBuyThreshold("p1", "item1", 0.0f), "Zero threshold rejected");
    assertTrue(!sys.setBuyThreshold("p1", "item1", -1.0f), "Negative threshold rejected");
    assertTrue(!sys.setBuyThreshold("p1", "unknown", 50.0f), "Unknown entry rejected");
}

static void testMarketWatchlistGetCurrentPrice() {
    std::cout << "\n=== MarketWatchlist: CurrentPrice ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1");
    sys.addEntry("p1", "item1", "Item 1", 100.0f);

    assertTrue(approxEqual(sys.getCurrentPrice("p1", "item1"), 100.0f), "Initial price stored");
    sys.updatePrice("p1", "item1", 150.0f);
    assertTrue(approxEqual(sys.getCurrentPrice("p1", "item1"), 150.0f), "Price updated");
    assertTrue(approxEqual(sys.getCurrentPrice("p1", "unknown"), 0.0f), "Unknown entry returns 0");
}

static void testMarketWatchlistMissing() {
    std::cout << "\n=== MarketWatchlist: Missing ===" << std::endl;
    ecs::World world;
    systems::MarketWatchlistSystem sys(&world);

    assertTrue(!sys.addEntry("none", "item", "Item", 1.0f), "Add fails on missing");
    assertTrue(!sys.removeEntry("none", "item"), "Remove fails on missing");
    assertTrue(!sys.clearWatchlist("none"), "Clear fails on missing");
    assertTrue(!sys.setBuyThreshold("none", "item", 1.0f), "SetBuy fails on missing");
    assertTrue(!sys.setSellThreshold("none", "item", 1.0f), "SetSell fails on missing");
    assertTrue(!sys.updatePrice("none", "item", 1.0f), "UpdatePrice fails on missing");
    assertTrue(!sys.acknowledgeAlert("none", "item"), "Acknowledge fails on missing");
    assertTrue(!sys.acknowledgeAllAlerts("none"), "AcknowledgeAll fails on missing");
    assertTrue(sys.getEntryCount("none") == 0, "0 entries on missing");
    assertTrue(!sys.hasBuyAlert("none", "item"), "No buy alert on missing");
    assertTrue(!sys.hasSellAlert("none", "item"), "No sell alert on missing");
    assertTrue(sys.getPendingAlertCount("none") == 0, "0 pending on missing");
    assertTrue(sys.getTotalAlertsFired("none") == 0, "0 total on missing");
    assertTrue(!sys.hasEntry("none", "item"), "Not found on missing");
}

void run_market_watchlist_system_tests() {
    testMarketWatchlistInit();
    testMarketWatchlistAddEntry();
    testMarketWatchlistRemoveEntry();
    testMarketWatchlistClear();
    testMarketWatchlistMaxEntries();
    testMarketWatchlistBuyAlert();
    testMarketWatchlistSellAlert();
    testMarketWatchlistBothAlerts();
    testMarketWatchlistAcknowledge();
    testMarketWatchlistClearThresholds();
    testMarketWatchlistInvalidThreshold();
    testMarketWatchlistGetCurrentPrice();
    testMarketWatchlistMissing();
}
