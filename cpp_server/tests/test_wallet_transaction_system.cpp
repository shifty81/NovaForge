// Tests for: Wallet Transaction System Tests
#include "test_log.h"
#include "components/core_components.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/wallet_transaction_system.h"

using namespace atlas;

// ==================== Wallet Transaction System Tests ====================

static void testWalletTransactionCreate() {
    std::cout << "\n=== WalletTransaction: Create ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    assertTrue(sys.initialize("p1", 10000.0), "Init succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("p1")), 10000.0f), "Balance is 10000");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalEarned("p1")), 0.0f), "Total earned is 0");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalSpent("p1")), 0.0f), "Total spent is 0");
    assertTrue(sys.getTransactionCount("p1") == 0, "No transactions initially");
}

static void testWalletTransactionDeposit() {
    std::cout << "\n=== WalletTransaction: Deposit ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 1000.0);
    assertTrue(sys.deposit("p1", "tx1", 5000.0, "Bounty", "Rat bounty"), "Deposit succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("p1")), 6000.0f), "Balance is 6000");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalEarned("p1")), 5000.0f), "Total earned is 5000");
    assertTrue(sys.getTransactionCount("p1") == 1, "1 transaction");
    assertTrue(!sys.deposit("p1", "tx2", -100.0, "Bounty", "Negative"), "Negative deposit rejected");
    assertTrue(!sys.deposit("p1", "tx3", 0.0, "Bounty", "Zero"), "Zero deposit rejected");
}

static void testWalletTransactionWithdraw() {
    std::cout << "\n=== WalletTransaction: Withdraw ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 10000.0);
    assertTrue(sys.withdraw("p1", "tx1", 3000.0, "Trade", "Buy Tritanium"), "Withdraw succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("p1")), 7000.0f), "Balance is 7000");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalSpent("p1")), 3000.0f), "Total spent is 3000");
    assertTrue(!sys.withdraw("p1", "tx2", 50000.0, "Trade", "Too expensive"), "Insufficient funds rejected");
    assertTrue(!sys.withdraw("p1", "tx3", -100.0, "Trade", "Negative"), "Negative withdraw rejected");
}

static void testWalletTransactionTransfer() {
    std::cout << "\n=== WalletTransaction: Transfer ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    world.createEntity("p2");
    sys.initialize("p1", 10000.0);
    sys.initialize("p2", 5000.0);
    assertTrue(sys.transfer("p1", "p2", "tx1", 3000.0, "Payment"), "Transfer succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("p1")), 7000.0f), "Sender balance 7000");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("p2")), 8000.0f), "Receiver balance 8000");
    assertTrue(sys.getTransactionCount("p1") == 1, "Sender has 1 tx");
    assertTrue(sys.getTransactionCount("p2") == 1, "Receiver has 1 tx");
    assertTrue(!sys.transfer("p1", "p2", "tx2", 50000.0, "Too much"), "Insufficient funds rejected");
    assertTrue(!sys.transfer("p1", "p2", "tx3", -100.0, "Negative"), "Negative transfer rejected");
}

static void testWalletTransactionCanAfford() {
    std::cout << "\n=== WalletTransaction: CanAfford ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 5000.0);
    assertTrue(sys.canAfford("p1", 5000.0), "Can afford exact balance");
    assertTrue(sys.canAfford("p1", 1000.0), "Can afford less than balance");
    assertTrue(!sys.canAfford("p1", 5001.0), "Cannot afford more than balance");
    assertTrue(!sys.canAfford("nonexistent", 1.0), "Missing entity returns false");
}

static void testWalletTransactionCategoryTotal() {
    std::cout << "\n=== WalletTransaction: CategoryTotal ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 100000.0);
    sys.deposit("p1", "tx1", 5000.0, "Bounty", "Bounty 1");
    sys.deposit("p1", "tx2", 3000.0, "Bounty", "Bounty 2");
    sys.deposit("p1", "tx3", 10000.0, "Mission", "Mission reward");
    sys.withdraw("p1", "tx4", 2000.0, "Trade", "Buy ammo");
    float bounty_total = static_cast<float>(sys.getCategoryTotal("p1", "Bounty"));
    float mission_total = static_cast<float>(sys.getCategoryTotal("p1", "Mission"));
    float trade_total = static_cast<float>(sys.getCategoryTotal("p1", "Trade"));
    assertTrue(approxEqual(bounty_total, 8000.0f), "Bounty total is 8000");
    assertTrue(approxEqual(mission_total, 10000.0f), "Mission total is 10000");
    assertTrue(approxEqual(trade_total, -2000.0f), "Trade total is -2000");
}

static void testWalletTransactionMaxHistory() {
    std::cout << "\n=== WalletTransaction: MaxHistory ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 1000000.0);
    auto* entity = world.getEntity("p1");
    auto* ledger = entity->getComponent<components::WalletLedger>();
    ledger->max_transactions = 3;
    sys.deposit("p1", "tx1", 100.0, "Bounty", "A");
    sys.deposit("p1", "tx2", 200.0, "Bounty", "B");
    sys.deposit("p1", "tx3", 300.0, "Bounty", "C");
    sys.deposit("p1", "tx4", 400.0, "Bounty", "D");
    assertTrue(static_cast<int>(ledger->transactions.size()) == 3, "Max 3 transactions in history");
    assertTrue(ledger->transactions[0].tx_id == "tx2", "Oldest evicted (tx1 gone)");
    assertTrue(sys.getTransactionCount("p1") == 4, "Total tx count is 4");
}

static void testWalletTransactionUpdate() {
    std::cout << "\n=== WalletTransaction: Update ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", 1000.0);
    sys.update(1.0f);
    sys.update(2.5f);
    auto* entity = world.getEntity("p1");
    auto* ledger = entity->getComponent<components::WalletLedger>();
    assertTrue(approxEqual(ledger->elapsed, 3.5f), "Elapsed time is 3.5");
}

static void testWalletTransactionNegativeStartingBalance() {
    std::cout << "\n=== WalletTransaction: NegativeStartingBalance ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    world.createEntity("p1");
    sys.initialize("p1", -5000.0);
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("p1")), 0.0f), "Negative starting balance clamped to 0");
}

static void testWalletTransactionMissing() {
    std::cout << "\n=== WalletTransaction: Missing ===" << std::endl;
    ecs::World world;
    systems::WalletTransactionSystem sys(&world);
    assertTrue(!sys.initialize("nonexistent", 1000.0), "Init fails on missing");
    assertTrue(!sys.deposit("nonexistent", "tx1", 100.0, "Bounty", "A"), "Deposit fails on missing");
    assertTrue(!sys.withdraw("nonexistent", "tx1", 100.0, "Trade", "A"), "Withdraw fails on missing");
    assertTrue(!sys.transfer("nonexistent", "also_missing", "tx1", 100.0, "A"), "Transfer fails on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("nonexistent")), 0.0f), "0 balance on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalEarned("nonexistent")), 0.0f), "0 earned on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getTotalSpent("nonexistent")), 0.0f), "0 spent on missing");
    assertTrue(sys.getTransactionCount("nonexistent") == 0, "0 tx on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getCategoryTotal("nonexistent", "Bounty")), 0.0f), "0 category on missing");
}


void run_wallet_transaction_system_tests() {
    testWalletTransactionCreate();
    testWalletTransactionDeposit();
    testWalletTransactionWithdraw();
    testWalletTransactionTransfer();
    testWalletTransactionCanAfford();
    testWalletTransactionCategoryTotal();
    testWalletTransactionMaxHistory();
    testWalletTransactionUpdate();
    testWalletTransactionNegativeStartingBalance();
    testWalletTransactionMissing();
}
