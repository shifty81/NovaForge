// Tests for: PlayerHangarInventorySystem
#include "test_log.h"
#include "components/core_components.h"
#include "components/ship_components.h"
#include "ecs/system.h"
#include "systems/player_hangar_inventory_system.h"

using namespace atlas;

// ==================== PlayerHangarInventorySystem Tests ====================

static void testHangarCreate() {
    std::cout << "\n=== PlayerHangarInventory: Create ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("hangar1");
    assertTrue(sys.initialize("hangar1", "player1", "station_jita", 5000.0), "Init succeeds");
    assertTrue(sys.getPlayerId("hangar1") == "player1", "Player ID set");
    assertTrue(sys.getStationId("hangar1") == "station_jita", "Station ID set");
    assertTrue(sys.getItemCount("hangar1") == 0, "No items initially");
    assertTrue(sys.getUsedVolume("hangar1") < 0.01, "Zero used volume");
    assertTrue(sys.getRemainingVolume("hangar1") > 4999.0, "Full capacity available");
}

static void testHangarInvalidInit() {
    std::cout << "\n=== PlayerHangarInventory: InvalidInit ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    assertTrue(!sys.initialize("missing", "p1", "s1", 100.0), "Missing entity fails");
    world.createEntity("h1");
    assertTrue(!sys.initialize("h1", "", "s1", 100.0), "Empty player fails");
    assertTrue(!sys.initialize("h1", "p1", "", 100.0), "Empty station fails");
    assertTrue(!sys.initialize("h1", "p1", "s1", 0.0), "Zero volume fails");
    assertTrue(!sys.initialize("h1", "p1", "s1", -100.0), "Negative volume fails");
}

static void testHangarDeposit() {
    std::cout << "\n=== PlayerHangarInventory: Deposit ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    sys.initialize("h1", "p1", "s1", 1000.0);

    assertTrue(sys.depositItem("h1", "tritanium", "Tritanium", "ore", 100, 0.01, 5.0),
               "Deposit tritanium");
    assertTrue(sys.getItemCount("h1") == 1, "1 item type");
    assertTrue(sys.getItemQuantity("h1", "tritanium") == 100, "100 units");
    assertTrue(sys.hasItem("h1", "tritanium"), "Has tritanium");
    assertTrue(sys.getTotalDeposits("h1") == 1, "1 deposit");

    double used = sys.getUsedVolume("h1");
    assertTrue(used > 0.99 && used < 1.01, "Used 1.0 m3 (100 * 0.01)");
}

static void testHangarDepositStacking() {
    std::cout << "\n=== PlayerHangarInventory: DepositStacking ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    sys.initialize("h1", "p1", "s1", 1000.0);

    sys.depositItem("h1", "tritanium", "Tritanium", "ore", 50, 0.01, 5.0);
    sys.depositItem("h1", "tritanium", "Tritanium", "ore", 30, 0.01, 5.0);

    assertTrue(sys.getItemCount("h1") == 1, "Still 1 item type (stacked)");
    assertTrue(sys.getItemQuantity("h1", "tritanium") == 80, "80 units stacked");
    assertTrue(sys.getTotalDeposits("h1") == 2, "2 deposits");
}

static void testHangarDepositCapacity() {
    std::cout << "\n=== PlayerHangarInventory: DepositCapacity ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    sys.initialize("h1", "p1", "s1", 10.0); // Only 10 m3

    assertTrue(sys.depositItem("h1", "ore1", "Ore", "ore", 5, 1.0, 10.0), "Deposit 5m3");
    assertTrue(!sys.depositItem("h1", "ore2", "Ore2", "ore", 6, 1.0, 10.0),
               "Exceeds capacity rejected");
    assertTrue(sys.getItemCount("h1") == 1, "Only first item stored");
}

static void testHangarDepositInvalid() {
    std::cout << "\n=== PlayerHangarInventory: DepositInvalid ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    sys.initialize("h1", "p1", "s1", 1000.0);

    assertTrue(!sys.depositItem("h1", "", "Ore", "ore", 1, 1.0, 10.0), "Empty ID rejected");
    assertTrue(!sys.depositItem("h1", "ore1", "", "ore", 1, 1.0, 10.0), "Empty name rejected");
    assertTrue(!sys.depositItem("h1", "ore1", "Ore", "", 1, 1.0, 10.0), "Empty type rejected");
    assertTrue(!sys.depositItem("h1", "ore1", "Ore", "ore", 0, 1.0, 10.0), "Zero qty rejected");
    assertTrue(!sys.depositItem("h1", "ore1", "Ore", "ore", -1, 1.0, 10.0), "Negative qty rejected");
    assertTrue(!sys.depositItem("h1", "ore1", "Ore", "ore", 1, 0.0, 10.0), "Zero vol rejected");
}

static void testHangarWithdraw() {
    std::cout << "\n=== PlayerHangarInventory: Withdraw ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    sys.initialize("h1", "p1", "s1", 1000.0);

    sys.depositItem("h1", "tritanium", "Tritanium", "ore", 100, 0.01, 5.0);

    assertTrue(sys.withdrawItem("h1", "tritanium", 30), "Withdraw 30");
    assertTrue(sys.getItemQuantity("h1", "tritanium") == 70, "70 remaining");
    assertTrue(sys.getTotalWithdrawals("h1") == 1, "1 withdrawal");

    // Withdraw all remaining
    assertTrue(sys.withdrawItem("h1", "tritanium", 70), "Withdraw rest");
    assertTrue(sys.getItemCount("h1") == 0, "Item removed when empty");
    assertTrue(!sys.hasItem("h1", "tritanium"), "No more tritanium");
}

static void testHangarWithdrawInvalid() {
    std::cout << "\n=== PlayerHangarInventory: WithdrawInvalid ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    sys.initialize("h1", "p1", "s1", 1000.0);

    sys.depositItem("h1", "ore1", "Ore", "ore", 10, 1.0, 5.0);

    assertTrue(!sys.withdrawItem("h1", "ore1", 11), "Over-withdraw rejected");
    assertTrue(!sys.withdrawItem("h1", "nonexistent", 1), "Missing item rejected");
    assertTrue(!sys.withdrawItem("h1", "", 1), "Empty ID rejected");
    assertTrue(!sys.withdrawItem("h1", "ore1", 0), "Zero qty rejected");
    assertTrue(!sys.withdrawItem("h1", "ore1", -1), "Negative qty rejected");
}

static void testHangarTransfer() {
    std::cout << "\n=== PlayerHangarInventory: Transfer ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    world.createEntity("h2");
    sys.initialize("h1", "p1", "s1", 1000.0);
    sys.initialize("h2", "p1", "s2", 1000.0);

    sys.depositItem("h1", "ore1", "Ore", "ore", 50, 1.0, 10.0);

    assertTrue(sys.transferItem("h1", "h2", "ore1", 20), "Transfer 20 succeeds");
    assertTrue(sys.getItemQuantity("h1", "ore1") == 30, "30 left in source");
    assertTrue(sys.getItemQuantity("h2", "ore1") == 20, "20 in destination");
}

static void testHangarSetMaxVolume() {
    std::cout << "\n=== PlayerHangarInventory: SetMaxVolume ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    sys.initialize("h1", "p1", "s1", 100.0);

    sys.depositItem("h1", "ore1", "Ore", "ore", 50, 1.0, 5.0);

    assertTrue(sys.setMaxVolume("h1", 200.0), "Increase capacity succeeds");
    assertTrue(!sys.setMaxVolume("h1", 30.0), "Shrink below used rejected");
    assertTrue(!sys.setMaxVolume("h1", 0.0), "Zero capacity rejected");
}

static void testHangarUpdate() {
    std::cout << "\n=== PlayerHangarInventory: Update ===" << std::endl;
    ecs::World world;
    systems::PlayerHangarInventorySystem sys(&world);
    world.createEntity("h1");
    sys.initialize("h1", "p1", "s1", 1000.0);

    sys.update(5.0f);
    // Just verifies update doesn't crash
    assertTrue(sys.getItemCount("h1") == 0, "Update doesn't create items");
}

void run_player_hangar_inventory_system_tests() {
    testHangarCreate();
    testHangarInvalidInit();
    testHangarDeposit();
    testHangarDepositStacking();
    testHangarDepositCapacity();
    testHangarDepositInvalid();
    testHangarWithdraw();
    testHangarWithdrawInvalid();
    testHangarTransfer();
    testHangarSetMaxVolume();
    testHangarUpdate();
}
