// Tests for: TradeWindowSystem
#include "test_log.h"
#include "components/game_components.h"
#include "ecs/system.h"
#include "systems/trade_window_system.h"

using namespace atlas;

// ==================== TradeWindowSystem Tests ====================

static void testTradeWindowInit() {
    std::cout << "\n=== TradeWindow: Init ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", "player_alice"), "Init succeeds");
    assertTrue(sys.getState("p1") == components::TradeWindow::TradeState::Idle,
               "Initial state is Idle");
    assertTrue(!sys.isTradeOpen("p1"), "Trade not open initially");
    assertTrue(sys.getOfferCount("p1") == 0, "Zero offers initially");
    assertTrue(approxEqual(sys.getTotalOfferValue("p1"), 0.0f), "Zero offer value initially");
    assertTrue(sys.getTotalTrades("p1") == 0, "Zero total trades initially");
    assertTrue(sys.getPartnerId("p1").empty(), "No partner initially");
}

static void testTradeWindowInitFails() {
    std::cout << "\n=== TradeWindow: InitFails ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "owner"), "Init fails on missing entity");
    world.createEntity("p1");
    assertTrue(!sys.initialize("p1", ""), "Init fails with empty owner_id");
}

static void testTradeWindowOpenTrade() {
    std::cout << "\n=== TradeWindow: OpenTrade ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");

    assertTrue(sys.openTrade("p1", "bob"), "Open trade with bob");
    assertTrue(sys.isTradeOpen("p1"), "Trade is open");
    assertTrue(sys.getPartnerId("p1") == "bob", "Partner ID stored");
    assertTrue(sys.getState("p1") == components::TradeWindow::TradeState::Open,
               "State is Open");

    assertTrue(!sys.openTrade("p1", "charlie"), "Cannot re-open while already open");
    assertTrue(!sys.openTrade("p1", ""), "Empty partner ID rejected");
}

static void testTradeWindowAddOffer() {
    std::cout << "\n=== TradeWindow: AddOffer ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.openTrade("p1", "bob");

    assertTrue(sys.addOffer("p1", "tritanium", "Tritanium", 1000, 5.0f),
               "Add first offer");
    assertTrue(sys.addOffer("p1", "plex", "PLEX", 1, 1500000.0f),
               "Add second offer");
    assertTrue(sys.getOfferCount("p1") == 2, "Two offers stored");
    assertTrue(!sys.addOffer("p1", "tritanium", "Duplicate", 100, 5.0f),
               "Duplicate item rejected");
    assertTrue(!sys.addOffer("p1", "", "Empty ID", 1, 0.0f),
               "Empty item ID rejected");
    assertTrue(!sys.addOffer("p1", "scrap", "Scrap", 0, 0.0f),
               "Zero quantity rejected");
    assertTrue(sys.getOfferCount("p1") == 2, "Count unchanged after rejections");
}

static void testTradeWindowRemoveOffer() {
    std::cout << "\n=== TradeWindow: RemoveOffer ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.openTrade("p1", "bob");
    sys.addOffer("p1", "tritanium", "Tritanium", 1000, 5.0f);
    sys.addOffer("p1", "plex", "PLEX", 1, 1500000.0f);

    assertTrue(sys.removeOffer("p1", "tritanium"), "Remove existing offer");
    assertTrue(sys.getOfferCount("p1") == 1, "Count decremented");
    assertTrue(!sys.removeOffer("p1", "tritanium"), "Remove nonexistent fails");
    assertTrue(!sys.removeOffer("p1", "nonexistent"), "Remove unknown fails");
}

static void testTradeWindowConfirmTrade() {
    std::cout << "\n=== TradeWindow: ConfirmTrade ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");

    assertTrue(!sys.confirmTrade("p1"), "Confirm fails when not open");
    sys.openTrade("p1", "bob");
    assertTrue(sys.confirmTrade("p1"), "Confirm succeeds when open");
    auto* comp = world.getEntity("p1")->getComponent<components::TradeWindow>();
    assertTrue(comp->owner_confirmed, "Owner confirmed flag set");

    // Cannot add offer after confirming
    assertTrue(!sys.addOffer("p1", "tritanium", "Tritanium", 100, 5.0f),
               "Cannot add offer after confirmation");
}

static void testTradeWindowCompleteTrade() {
    std::cout << "\n=== TradeWindow: CompleteTrade ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.openTrade("p1", "bob");
    sys.addOffer("p1", "plex", "PLEX", 2, 1500000.0f);

    assertTrue(!sys.completeTrade("p1"), "Complete fails without both confirmations");
    sys.confirmTrade("p1");
    assertTrue(!sys.completeTrade("p1"), "Complete fails with only owner confirmed");
    sys.setPartnerConfirmed("p1", true);
    assertTrue(sys.completeTrade("p1"), "Complete succeeds with both confirmed");
    assertTrue(sys.getState("p1") == components::TradeWindow::TradeState::Complete,
               "State is Complete");
    assertTrue(sys.getTotalTrades("p1") == 1, "Total trades incremented");
    assertTrue(!sys.completeTrade("p1"), "Cannot complete twice");
}

static void testTradeWindowCancelTrade() {
    std::cout << "\n=== TradeWindow: CancelTrade ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");

    assertTrue(!sys.cancelTrade("p1"), "Cancel fails when not open");
    sys.openTrade("p1", "bob");
    sys.addOffer("p1", "tritanium", "Tritanium", 500, 5.0f);
    assertTrue(sys.cancelTrade("p1"), "Cancel succeeds when open");
    assertTrue(sys.getState("p1") == components::TradeWindow::TradeState::Cancelled,
               "State is Cancelled");
    assertTrue(sys.getOfferCount("p1") == 0, "Offers cleared on cancel");
    assertTrue(sys.getTotalTrades("p1") == 0, "Total trades unchanged after cancel");
}

static void testTradeWindowOfferValue() {
    std::cout << "\n=== TradeWindow: OfferValue ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");
    sys.openTrade("p1", "bob");

    sys.addOffer("p1", "item1", "Item 1", 10, 100.0f);   // 1000
    sys.addOffer("p1", "item2", "Item 2", 5,  200.0f);   // 1000
    assertTrue(approxEqual(sys.getTotalOfferValue("p1"), 2000.0f), "Total value correct");
}

static void testTradeWindowSetPartnerConfirmed() {
    std::cout << "\n=== TradeWindow: SetPartnerConfirmed ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", "alice");

    assertTrue(!sys.setPartnerConfirmed("p1", true), "Set partner confirmed fails when not open");
    sys.openTrade("p1", "bob");
    assertTrue(sys.setPartnerConfirmed("p1", true), "Set partner confirmed succeeds");
    auto* comp = world.getEntity("p1")->getComponent<components::TradeWindow>();
    assertTrue(comp->partner_confirmed, "Partner confirmed flag set");
    assertTrue(sys.setPartnerConfirmed("p1", false), "Can unconfirm partner");
    assertTrue(!comp->partner_confirmed, "Partner confirmed cleared");
}

static void testTradeWindowMissing() {
    std::cout << "\n=== TradeWindow: Missing ===" << std::endl;
    ecs::World world;
    systems::TradeWindowSystem sys(&world);

    assertTrue(!sys.openTrade("nonexistent", "bob"), "Open fails on missing");
    assertTrue(!sys.cancelTrade("nonexistent"), "Cancel fails on missing");
    assertTrue(!sys.confirmTrade("nonexistent"), "Confirm fails on missing");
    assertTrue(!sys.completeTrade("nonexistent"), "Complete fails on missing");
    assertTrue(!sys.addOffer("nonexistent", "i", "I", 1, 1.0f), "AddOffer fails on missing");
    assertTrue(!sys.removeOffer("nonexistent", "i"), "RemoveOffer fails on missing");
    assertTrue(sys.getState("nonexistent") == components::TradeWindow::TradeState::Idle,
               "Idle state on missing");
    assertTrue(!sys.isTradeOpen("nonexistent"), "Not open on missing");
    assertTrue(sys.getOfferCount("nonexistent") == 0, "Zero offers on missing");
    assertTrue(approxEqual(sys.getTotalOfferValue("nonexistent"), 0.0f), "Zero value on missing");
    assertTrue(sys.getTotalTrades("nonexistent") == 0, "Zero trades on missing");
    assertTrue(sys.getPartnerId("nonexistent").empty(), "Empty partner on missing");
}

void run_trade_window_system_tests() {
    testTradeWindowInit();
    testTradeWindowInitFails();
    testTradeWindowOpenTrade();
    testTradeWindowAddOffer();
    testTradeWindowRemoveOffer();
    testTradeWindowConfirmTrade();
    testTradeWindowCompleteTrade();
    testTradeWindowCancelTrade();
    testTradeWindowOfferValue();
    testTradeWindowSetPartnerConfirmed();
    testTradeWindowMissing();
}
