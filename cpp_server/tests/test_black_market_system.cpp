// Tests for: Black Market System Tests
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/black_market_system.h"
#include "systems/market_system.h"

using namespace atlas;

// ==================== Black Market System Tests ====================

static void testBlackMarketDefaults() {
    std::cout << "\n=== Black Market Defaults ===" << std::endl;
    components::BlackMarket bm;
    assertTrue(bm.listings.empty(), "Default listings is empty");
    assertTrue(approxEqual(bm.security_level, 0.5f), "Default security_level is 0.5");
    assertTrue(approxEqual(bm.detection_chance_base, 0.1f), "Default detection_chance_base is 0.1");
    assertTrue(approxEqual(bm.price_markup, 1.5f), "Default price_markup is 1.5");
    assertTrue(bm.max_listings == 20, "Default max_listings is 20");
    assertTrue(approxEqual(bm.listing_refresh_interval, 120.0f), "Default listing_refresh_interval is 120");
}

static void testBlackMarketAddListing() {
    std::cout << "\n=== Black Market Add Listing ===" << std::endl;
    ecs::World world;
    systems::BlackMarketSystem bmSys(&world);
    auto* entity = world.createEntity("market_1");
    addComp<components::BlackMarket>(entity);

    bmSys.addListing("market_1", "stolen_ore", "seller_a", 100.0f, 5, true, 0.3f);
    assertTrue(bmSys.getListingCount("market_1") == 1, "One listing added");

    auto items = bmSys.getAvailableItems("market_1");
    assertTrue(items.size() == 1 && items[0] == "stolen_ore", "Available items contains stolen_ore");
}

static void testBlackMarketPurchase() {
    std::cout << "\n=== Black Market Purchase ===" << std::endl;
    ecs::World world;
    systems::BlackMarketSystem bmSys(&world);
    auto* entity = world.createEntity("market_2");
    auto* bm = addComp<components::BlackMarket>(entity);
    bm->addListing("rare_gem", "seller_b", 200.0f, 3, false, 0.1f);

    bool ok = bmSys.purchaseItem("market_2", "rare_gem", "buyer_x");
    assertTrue(ok, "Purchase succeeds");
    auto* listing = bm->findListing("rare_gem");
    assertTrue(listing != nullptr && listing->quantity == 2, "Quantity decreased to 2");
}

static void testBlackMarketPurchaseRemovesEmpty() {
    std::cout << "\n=== Black Market Purchase Removes Empty ===" << std::endl;
    ecs::World world;
    systems::BlackMarketSystem bmSys(&world);
    auto* entity = world.createEntity("market_3");
    auto* bm = addComp<components::BlackMarket>(entity);
    bm->addListing("last_item", "seller_c", 50.0f, 1, true, 0.5f);

    bool ok = bmSys.purchaseItem("market_3", "last_item", "buyer_y");
    assertTrue(ok, "Purchase of last item succeeds");
    assertTrue(bm->getListingCount() == 0, "Listing removed when quantity reaches 0");
}

static void testBlackMarketExpiry() {
    std::cout << "\n=== Black Market Expiry ===" << std::endl;
    ecs::World world;
    systems::BlackMarketSystem bmSys(&world);
    auto* entity = world.createEntity("market_4");
    auto* bm = addComp<components::BlackMarket>(entity);
    bm->addListing("temp_goods", "seller_d", 80.0f, 2, false, 0.2f);
    bm->listings[0].max_expiry = 10.0f;

    bmSys.update(5.0f);
    assertTrue(bm->getListingCount() == 1, "Listing still present before expiry");

    bmSys.update(6.0f);
    assertTrue(bm->getListingCount() == 0, "Listing removed after expiry");
}

static void testBlackMarketMaxListings() {
    std::cout << "\n=== Black Market Max Listings ===" << std::endl;
    components::BlackMarket bm;
    bm.max_listings = 3;
    bm.addListing("item_a", "s1", 10.0f, 1, false, 0.1f);
    bm.addListing("item_b", "s2", 20.0f, 1, false, 0.1f);
    bm.addListing("item_c", "s3", 30.0f, 1, false, 0.1f);
    assertTrue(bm.getListingCount() == 3, "3 listings at max");

    bm.addListing("item_d", "s4", 40.0f, 1, false, 0.1f);
    assertTrue(bm.getListingCount() == 3, "Still at max after adding 4th");
    assertTrue(bm.findListing("item_a") == nullptr, "Oldest listing (item_a) removed");
    assertTrue(bm.findListing("item_d") != nullptr, "Newest listing (item_d) present");
}

static void testBlackMarketDetectionChance() {
    std::cout << "\n=== Black Market Detection Chance ===" << std::endl;
    ecs::World world;
    systems::BlackMarketSystem bmSys(&world);
    auto* entity = world.createEntity("market_5");
    auto* bm = addComp<components::BlackMarket>(entity);
    bm->detection_chance_base = 0.2f;
    bm->security_level = 0.5f;

    float chance = bmSys.getDetectionChance("market_5");
    assertTrue(approxEqual(chance, 0.1f), "Detection chance = base * security (0.2 * 0.5 = 0.1)");

    bmSys.setSecurityLevel("market_5", 1.0f);
    chance = bmSys.getDetectionChance("market_5");
    assertTrue(approxEqual(chance, 0.2f), "Detection chance scales with security level");
}


void run_black_market_system_tests() {
    testBlackMarketDefaults();
    testBlackMarketAddListing();
    testBlackMarketPurchase();
    testBlackMarketPurchaseRemovesEmpty();
    testBlackMarketExpiry();
    testBlackMarketMaxListings();
    testBlackMarketDetectionChance();
}
