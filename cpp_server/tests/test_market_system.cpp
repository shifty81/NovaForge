// Tests for: MarketSystem Tests, Market Ore Pricing Tests, NPC Market Seeding Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/market_system.h"
#include <sys/stat.h>

using namespace atlas;

// ==================== MarketSystem Tests ====================

static void testMarketPlaceSellOrder() {
    std::cout << "\n=== Market Place Sell Order ===" << std::endl;
    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("station_1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_1";

    auto* seller = world.createEntity("seller_1");
    auto* pc = addComp<components::Player>(seller);
    pc->credits = 100000.0;

    std::string oid = marketSys.placeSellOrder("station_1", "seller_1", "stellium", "Stellium", 100, 5.0);
    assertTrue(!oid.empty(), "Sell order created");
    assertTrue(marketSys.getOrderCount("station_1") == 1, "One order on station");
    assertTrue(pc->credits < 100000.0, "Broker fee deducted from seller");
}

static void testMarketBuyFromMarket() {
    std::cout << "\n=== Market Buy From Market ===" << std::endl;
    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("station_1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_1";

    auto* seller = world.createEntity("seller_1");
    auto* seller_pc = addComp<components::Player>(seller);
    seller_pc->credits = 100000.0;

    auto* buyer = world.createEntity("buyer_1");
    auto* buyer_pc = addComp<components::Player>(buyer);
    buyer_pc->credits = 100000.0;

    marketSys.placeSellOrder("station_1", "seller_1", "stellium", "Stellium", 100, 5.0);

    int bought = marketSys.buyFromMarket("station_1", "buyer_1", "stellium", 50);
    assertTrue(bought == 50, "Bought 50 units");
    assertTrue(buyer_pc->credits < 100000.0, "Buyer Credits decreased");
    assertTrue(seller_pc->credits > 100000.0 - 100000.0 * 0.02, "Seller Credits increased from sale");
}

static void testMarketPriceQueries() {
    std::cout << "\n=== Market Price Queries ===" << std::endl;
    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("station_1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_1";

    auto* seller1 = world.createEntity("seller_1");
    auto* pc1 = addComp<components::Player>(seller1);
    pc1->credits = 1000000.0;

    auto* seller2 = world.createEntity("seller_2");
    auto* pc2 = addComp<components::Player>(seller2);
    pc2->credits = 1000000.0;

    auto* buyer1 = world.createEntity("buyer_1");
    auto* bpc = addComp<components::Player>(buyer1);
    bpc->credits = 1000000.0;

    marketSys.placeSellOrder("station_1", "seller_1", "stellium", "Stellium", 100, 5.0);
    marketSys.placeSellOrder("station_1", "seller_2", "stellium", "Stellium", 50, 4.5);
    marketSys.placeBuyOrder("station_1", "buyer_1", "stellium", "Stellium", 200, 4.0);

    double lowest = marketSys.getLowestSellPrice("station_1", "stellium");
    assertTrue(approxEqual(static_cast<float>(lowest), 4.5f), "Lowest sell is 4.5");

    double highest = marketSys.getHighestBuyPrice("station_1", "stellium");
    assertTrue(approxEqual(static_cast<float>(highest), 4.0f), "Highest buy is 4.0");

    double no_item = marketSys.getLowestSellPrice("station_1", "nonexistent");
    assertTrue(no_item < 0, "No sell price for nonexistent item");
}

static void testMarketOrderExpiry() {
    std::cout << "\n=== Market Order Expiry ===" << std::endl;
    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("station_1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_1";

    auto* seller = world.createEntity("seller_1");
    auto* pc = addComp<components::Player>(seller);
    pc->credits = 1000000.0;

    marketSys.placeSellOrder("station_1", "seller_1", "stellium", "Stellium", 100, 5.0);
    assertTrue(marketSys.getOrderCount("station_1") == 1, "One active order");

    // Set order duration
    hub->orders[0].duration_remaining = 5.0f;

    marketSys.update(6.0f);
    assertTrue(marketSys.getOrderCount("station_1") == 0, "Order expired and removed");
}


// ==================== Market Ore Pricing Tests ====================

static void testMarketOrePricing() {
    std::cout << "\n=== Market: NPC Ore Pricing ===" << std::endl;

    ecs::World world;
    systems::MarketSystem marketSys(&world);

    // Create a station with market hub
    auto* station = world.createEntity("trade_hub");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "trade_hub";
    hub->broker_fee_rate = 0.0;
    hub->sales_tax_rate = 0.0;

    // Create an NPC seller
    auto* npc_seller = world.createEntity("npc_ore_seller");
    auto* npc_player = addComp<components::Player>(npc_seller);
    npc_player->credits = 1000000.0;

    // NPC places sell orders for common ores
    std::string o1 = marketSys.placeSellOrder("trade_hub", "npc_ore_seller",
                                               "Ferrite", "Ferrite", 10000, 15.0);
    std::string o2 = marketSys.placeSellOrder("trade_hub", "npc_ore_seller",
                                               "Galvite", "Galvite", 5000, 38.0);
    std::string o3 = marketSys.placeSellOrder("trade_hub", "npc_ore_seller",
                                               "Cryolite", "Cryolite", 3000, 70.0);

    assertTrue(!o1.empty(), "Ferrite sell order placed");
    assertTrue(!o2.empty(), "Galvite sell order placed");
    assertTrue(!o3.empty(), "Cryolite sell order placed");

    double veldspar_price = marketSys.getLowestSellPrice("trade_hub", "Ferrite");
    double scordite_price = marketSys.getLowestSellPrice("trade_hub", "Galvite");
    double pyroxeres_price = marketSys.getLowestSellPrice("trade_hub", "Cryolite");

    assertTrue(veldspar_price == 15.0, "Ferrite price is 15 Credits");
    assertTrue(scordite_price == 38.0, "Galvite price is 38 Credits");
    assertTrue(pyroxeres_price == 70.0, "Cryolite price is 70 Credits");
    assertTrue(marketSys.getOrderCount("trade_hub") == 3, "3 orders on market");
}

static void testMarketMineralPricing() {
    std::cout << "\n=== Market: NPC Mineral Buy Orders ===" << std::endl;

    ecs::World world;
    systems::MarketSystem marketSys(&world);

    auto* station = world.createEntity("mineral_hub");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "mineral_hub";
    hub->broker_fee_rate = 0.0;
    hub->sales_tax_rate = 0.0;

    auto* npc_buyer = world.createEntity("npc_mineral_buyer");
    auto* npc_player = addComp<components::Player>(npc_buyer);
    npc_player->credits = 10000000.0;

    // NPC places buy orders for refined minerals
    std::string b1 = marketSys.placeBuyOrder("mineral_hub", "npc_mineral_buyer",
                                              "Stellium", "Stellium", 50000, 6.0);
    std::string b2 = marketSys.placeBuyOrder("mineral_hub", "npc_mineral_buyer",
                                              "Vanthium", "Vanthium", 20000, 9.0);

    assertTrue(!b1.empty(), "Stellium buy order placed");
    assertTrue(!b2.empty(), "Vanthium buy order placed");

    double trit_price = marketSys.getHighestBuyPrice("mineral_hub", "Stellium");
    double pyer_price = marketSys.getHighestBuyPrice("mineral_hub", "Vanthium");

    assertTrue(trit_price == 6.0, "Stellium buy price is 6 Credits");
    assertTrue(pyer_price == 9.0, "Vanthium buy price is 9 Credits");
}


// ==================== NPC Market Seeding Tests ====================

static void testNPCMarketSeedCreatesOrders() {
    std::cout << "\n=== NPC Market Seed Creates Orders ===" << std::endl;
    ecs::World world;
    systems::MarketSystem market(&world);

    auto* station = world.createEntity("station_seed1");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_seed1";

    int created = market.seedNPCOrders("station_seed1");
    assertTrue(created == 4, "seedNPCOrders creates 4 NPC sell orders");
    assertTrue(hub->orders.size() == 4, "MarketHub has 4 orders after seeding");
}

static void testNPCMarketSeedPricesCorrect() {
    std::cout << "\n=== NPC Market Seed Prices Correct ===" << std::endl;
    ecs::World world;
    systems::MarketSystem market(&world);

    auto* station = world.createEntity("station_seed2");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_seed2";

    market.seedNPCOrders("station_seed2");

    assertTrue(hub->orders[0].item_name == "Stellium", "First order is Stellium");
    assertTrue(approxEqual(hub->orders[0].price_per_unit, 6.0), "Stellium price is 6.0 Credits");
    assertTrue(hub->orders[1].item_name == "Vanthium", "Second order is Vanthium");
    assertTrue(approxEqual(hub->orders[1].price_per_unit, 10.0), "Vanthium price is 10.0 Credits");
    assertTrue(hub->orders[2].item_name == "Cydrium", "Third order is Cydrium");
    assertTrue(approxEqual(hub->orders[2].price_per_unit, 40.0), "Cydrium price is 40.0 Credits");
    assertTrue(hub->orders[3].item_name == "Nocxidium", "Fourth order is Nocxidium");
    assertTrue(approxEqual(hub->orders[3].price_per_unit, 800.0, 1.0), "Nocxidium price is 800.0 Credits");
}

static void testNPCMarketSeedOrdersPermanent() {
    std::cout << "\n=== NPC Market Seed Orders Permanent ===" << std::endl;
    ecs::World world;
    systems::MarketSystem market(&world);

    auto* station = world.createEntity("station_seed3");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_seed3";

    market.seedNPCOrders("station_seed3");

    for (const auto& order : hub->orders) {
        assertTrue(order.duration_remaining < 0.0f, "NPC order is permanent (duration_remaining < 0)");
        assertTrue(order.owner_id == "npc_market", "NPC order owned by npc_market");
        assertTrue(!order.is_buy_order, "NPC seed orders are sell orders");
    }
}

static void testNPCMarketSeedBuyableByPlayer() {
    std::cout << "\n=== NPC Market Seed Buyable By Player ===" << std::endl;
    ecs::World world;
    systems::MarketSystem market(&world);

    auto* station = world.createEntity("station_seed4");
    auto* hub = addComp<components::MarketHub>(station);
    hub->station_id = "station_seed4";

    auto* buyer = world.createEntity("player_buyer");
    auto* player = addComp<components::Player>(buyer);
    player->credits = 10000.0;
    addComp<components::Inventory>(buyer);

    market.seedNPCOrders("station_seed4");

    double tritPrice = market.getLowestSellPrice("station_seed4", "mineral_tritanium");
    assertTrue(tritPrice > 0.0, "Stellium sell price exists after seeding");
    assertTrue(approxEqual(tritPrice, 6.0), "Stellium sell price is 6.0 Credits");
}


void run_market_system_tests() {
    testMarketPlaceSellOrder();
    testMarketBuyFromMarket();
    testMarketPriceQueries();
    testMarketOrderExpiry();
    testMarketOrePricing();
    testMarketMineralPricing();
    testNPCMarketSeedCreatesOrders();
    testNPCMarketSeedPricesCorrect();
    testNPCMarketSeedOrdersPermanent();
    testNPCMarketSeedBuyableByPlayer();
}
