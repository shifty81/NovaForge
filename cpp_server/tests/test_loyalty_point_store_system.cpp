// Tests for: LoyaltyPointStore System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/loyalty_point_store_system.h"

using namespace atlas;

// ==================== LoyaltyPointStore System Tests ====================

static void testLPStoreCreate() {
    std::cout << "\n=== LPStore: Create ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    assertTrue(sys.initialize("lp1", "store_caldari", "caldari_navy"), "Init succeeds");
    assertTrue(sys.getItemCount("lp1") == 0, "No items initially");
    assertTrue(sys.getPlayerCount("lp1") == 0, "No players initially");
    assertTrue(sys.getTotalPurchases("lp1") == 0, "No purchases initially");
    assertTrue(approxEqual(sys.getTotalISKCollected("lp1"), 0.0f), "No ISK collected");
}

static void testLPStoreAddItem() {
    std::cout << "\n=== LPStore: AddItem ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    sys.initialize("lp1", "store_caldari", "caldari_navy");
    assertTrue(sys.addItem("lp1", "item1", "Navy Raven", "Ship", 500000, 200000000.0f, 3), "Add Ship");
    assertTrue(sys.addItem("lp1", "item2", "Navy BCU", "Module", 50000, 5000000.0f, 2), "Add Module");
    assertTrue(sys.getItemCount("lp1") == 2, "2 items in store");
}

static void testLPStoreDuplicate() {
    std::cout << "\n=== LPStore: Duplicate ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    sys.initialize("lp1", "store_caldari", "caldari_navy");
    sys.addItem("lp1", "item1", "Navy Raven", "Ship", 500000, 200000000.0f, 3);
    assertTrue(!sys.addItem("lp1", "item1", "Other", "Module", 100, 100.0f, 1), "Duplicate item rejected");
    sys.registerPlayer("lp1", "player1");
    assertTrue(!sys.registerPlayer("lp1", "player1"), "Duplicate player rejected");
}

static void testLPStoreEarnLP() {
    std::cout << "\n=== LPStore: EarnLP ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    sys.initialize("lp1", "store_caldari", "caldari_navy");
    sys.registerPlayer("lp1", "player1");
    assertTrue(sys.earnLP("lp1", "player1", 10000), "Earn LP succeeds");
    assertTrue(sys.getBalance("lp1", "player1") == 10000, "Balance is 10000");
    assertTrue(sys.earnLP("lp1", "player1", 5000), "Earn more LP");
    assertTrue(sys.getBalance("lp1", "player1") == 15000, "Balance is 15000");
}

static void testLPStorePurchase() {
    std::cout << "\n=== LPStore: Purchase ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    sys.initialize("lp1", "store_caldari", "caldari_navy");
    sys.addItem("lp1", "item1", "Navy BCU", "Module", 50000, 5000000.0f, 2);
    sys.registerPlayer("lp1", "player1");
    sys.earnLP("lp1", "player1", 100000);
    assertTrue(sys.purchaseItem("lp1", "player1", "item1"), "Purchase succeeds");
    assertTrue(sys.getBalance("lp1", "player1") == 50000, "Balance reduced by 50000");
    assertTrue(sys.getTotalPurchases("lp1") == 1, "1 total purchase");
    assertTrue(approxEqual(sys.getTotalISKCollected("lp1"), 5000000.0f), "ISK collected 5M");
}

static void testLPStorePurchaseFail() {
    std::cout << "\n=== LPStore: PurchaseFail ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    sys.initialize("lp1", "store_caldari", "caldari_navy");
    sys.addItem("lp1", "item1", "Navy Raven", "Ship", 500000, 200000000.0f, 3);
    sys.registerPlayer("lp1", "player1");
    sys.earnLP("lp1", "player1", 100);
    assertTrue(!sys.purchaseItem("lp1", "player1", "item1"), "Insufficient LP rejected");
    assertTrue(sys.getBalance("lp1", "player1") == 100, "Balance unchanged");
    assertTrue(sys.getTotalPurchases("lp1") == 0, "No purchases");
}

static void testLPStoreCategory() {
    std::cout << "\n=== LPStore: Category ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    sys.initialize("lp1", "store_caldari", "caldari_navy");
    sys.addItem("lp1", "i1", "Navy Raven", "Ship", 500000, 200000000.0f, 3);
    sys.addItem("lp1", "i2", "Navy Caracal", "Ship", 200000, 50000000.0f, 2);
    sys.addItem("lp1", "i3", "Navy BCU", "Module", 50000, 5000000.0f, 2);
    sys.addItem("lp1", "i4", "Ammo Pack", "Ammunition", 1000, 100000.0f, 1);
    assertTrue(sys.getItemsByCategory("lp1", "Ship") == 2, "2 ships");
    assertTrue(sys.getItemsByCategory("lp1", "Module") == 1, "1 module");
    assertTrue(sys.getItemsByCategory("lp1", "Ammunition") == 1, "1 ammo");
    assertTrue(sys.getItemsByCategory("lp1", "Blueprint") == 0, "0 blueprints");
}

static void testLPStoreMultiPlayer() {
    std::cout << "\n=== LPStore: MultiPlayer ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    sys.initialize("lp1", "store_caldari", "caldari_navy");
    sys.registerPlayer("lp1", "player1");
    sys.registerPlayer("lp1", "player2");
    sys.earnLP("lp1", "player1", 5000);
    sys.earnLP("lp1", "player2", 10000);
    assertTrue(sys.getBalance("lp1", "player1") == 5000, "Player1 has 5000");
    assertTrue(sys.getBalance("lp1", "player2") == 10000, "Player2 has 10000");
    assertTrue(sys.getPlayerCount("lp1") == 2, "2 players");
}

static void testLPStoreMaxLimit() {
    std::cout << "\n=== LPStore: MaxLimit ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    world.createEntity("lp1");
    sys.initialize("lp1", "store_caldari", "caldari_navy");
    auto* entity = world.getEntity("lp1");
    auto* store = entity->getComponent<components::LoyaltyPointStore>();
    store->max_items = 2;
    store->max_players = 2;
    sys.addItem("lp1", "i1", "Item1", "Ship", 100, 100.0f, 1);
    sys.addItem("lp1", "i2", "Item2", "Module", 100, 100.0f, 1);
    assertTrue(!sys.addItem("lp1", "i3", "Item3", "Ship", 100, 100.0f, 1), "Max items enforced");
    sys.registerPlayer("lp1", "p1");
    sys.registerPlayer("lp1", "p2");
    assertTrue(!sys.registerPlayer("lp1", "p3"), "Max players enforced");
}

static void testLPStoreMissing() {
    std::cout << "\n=== LPStore: Missing ===" << std::endl;
    ecs::World world;
    systems::LoyaltyPointStoreSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", "s1", "f1"), "Init fails on missing");
    assertTrue(!sys.addItem("nonexistent", "i1", "N", "C", 1, 1.0f, 1), "AddItem fails on missing");
    assertTrue(!sys.removeItem("nonexistent", "i1"), "RemoveItem fails on missing");
    assertTrue(!sys.registerPlayer("nonexistent", "p1"), "RegisterPlayer fails on missing");
    assertTrue(!sys.earnLP("nonexistent", "p1", 100), "EarnLP fails on missing");
    assertTrue(!sys.purchaseItem("nonexistent", "p1", "i1"), "Purchase fails on missing");
    assertTrue(sys.getBalance("nonexistent", "p1") == 0, "0 balance on missing");
    assertTrue(sys.getItemCount("nonexistent") == 0, "0 items on missing");
    assertTrue(sys.getPlayerCount("nonexistent") == 0, "0 players on missing");
    assertTrue(sys.getTotalPurchases("nonexistent") == 0, "0 purchases on missing");
    assertTrue(approxEqual(sys.getTotalISKCollected("nonexistent"), 0.0f), "0 ISK on missing");
    assertTrue(sys.getItemsByCategory("nonexistent", "Ship") == 0, "0 category on missing");
}


void run_loyalty_point_store_system_tests() {
    testLPStoreCreate();
    testLPStoreAddItem();
    testLPStoreDuplicate();
    testLPStoreEarnLP();
    testLPStorePurchase();
    testLPStorePurchaseFail();
    testLPStoreCategory();
    testLPStoreMultiPlayer();
    testLPStoreMaxLimit();
    testLPStoreMissing();
}
