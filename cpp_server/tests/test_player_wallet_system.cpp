// Tests for: PlayerWalletSystem
#include "test_log.h"
#include "components/economy_components.h"
#include "ecs/system.h"
#include "systems/player_wallet_system.h"

using namespace atlas;

// ==================== PlayerWalletSystem Tests ====================

static void testWalletDeposit() {
    std::cout << "\n=== PlayerWallet: Deposit ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerWallet>(e);

    assertTrue(sys.deposit("player_1", 10000.0, "Mining reward"), "Deposit succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("player_1")), 10000.0f), "Balance is 10000");
    assertTrue(approxEqual(static_cast<float>(sys.getLifetimeEarned("player_1")), 10000.0f), "Lifetime earned is 10000");
    assertTrue(sys.getTransactionCount("player_1") == 1, "1 transaction recorded");
}

static void testWalletWithdraw() {
    std::cout << "\n=== PlayerWallet: Withdraw ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerWallet>(e);

    sys.deposit("player_1", 5000.0, "Starting balance");
    assertTrue(sys.withdraw("player_1", 2000.0, "Module purchase"), "Withdraw succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("player_1")), 3000.0f), "Balance is 3000");
    assertTrue(approxEqual(static_cast<float>(sys.getLifetimeSpent("player_1")), 2000.0f), "Lifetime spent is 2000");
}

static void testWalletInsufficientFunds() {
    std::cout << "\n=== PlayerWallet: Insufficient Funds ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerWallet>(e);

    sys.deposit("player_1", 1000.0, "Initial");
    assertTrue(!sys.withdraw("player_1", 2000.0, "Too much"), "Withdraw rejected (insufficient)");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("player_1")), 1000.0f), "Balance unchanged");
}

static void testWalletTransferWithTax() {
    std::cout << "\n=== PlayerWallet: Transfer With Tax ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e1 = world.createEntity("seller");
    auto* e2 = world.createEntity("buyer");
    addComp<components::PlayerWallet>(e1);
    addComp<components::PlayerWallet>(e2);

    sys.deposit("seller", 20000.0, "Starting balance");
    // Default tax rate is 5%, so transferring 10000 costs 10500
    assertTrue(sys.transferWithTax("seller", "buyer", 10000.0, "Ship sale"), "Transfer succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("seller")), 9500.0f), "Seller balance after tax");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("buyer")), 10000.0f), "Buyer receives full amount");
}

static void testWalletTransferInsufficientWithTax() {
    std::cout << "\n=== PlayerWallet: Transfer Insufficient With Tax ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e1 = world.createEntity("seller");
    auto* e2 = world.createEntity("buyer");
    addComp<components::PlayerWallet>(e1);
    addComp<components::PlayerWallet>(e2);

    sys.deposit("seller", 10000.0, "Starting");
    // With 5% tax, need 10500 to transfer 10000
    assertTrue(!sys.transferWithTax("seller", "buyer", 10000.0, "Too expensive"), "Transfer rejected");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("seller")), 10000.0f), "Seller balance unchanged");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("buyer")), 0.0f), "Buyer balance unchanged");
}

static void testWalletSetTaxRate() {
    std::cout << "\n=== PlayerWallet: Set Tax Rate ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerWallet>(e);

    assertTrue(approxEqual(static_cast<float>(sys.getTaxRate("player_1")), 0.05f), "Default tax rate is 5%");
    assertTrue(sys.setTaxRate("player_1", 0.10), "Set tax rate succeeds");
    assertTrue(approxEqual(static_cast<float>(sys.getTaxRate("player_1")), 0.10f), "Tax rate is now 10%");
    assertTrue(!sys.setTaxRate("player_1", -0.01), "Negative tax rate rejected");
    assertTrue(!sys.setTaxRate("player_1", 1.01), "Tax rate > 100% rejected");
}

static void testWalletTransactionLog() {
    std::cout << "\n=== PlayerWallet: Transaction Log ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerWallet>(e);

    sys.deposit("player_1", 5000.0, "Bounty payment");
    sys.withdraw("player_1", 1000.0, "Ammo purchase");

    assertTrue(sys.getTransactionCount("player_1") == 2, "2 transactions");
    assertTrue(approxEqual(static_cast<float>(sys.getLastTransactionAmount("player_1")), -1000.0f), "Last txn is -1000");
}

static void testWalletMaxTransactions() {
    std::cout << "\n=== PlayerWallet: Max Transactions ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    auto* w = addComp<components::PlayerWallet>(e);
    w->max_transactions = 3;

    sys.deposit("player_1", 100.0, "A");
    sys.deposit("player_1", 200.0, "B");
    sys.deposit("player_1", 300.0, "C");
    sys.deposit("player_1", 400.0, "D");

    assertTrue(sys.getTransactionCount("player_1") == 3, "Max transactions enforced");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("player_1")), 1000.0f), "Balance still correct (1000)");
}

static void testWalletNegativeDeposit() {
    std::cout << "\n=== PlayerWallet: Negative Deposit ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerWallet>(e);

    assertTrue(!sys.deposit("player_1", -100.0, "Invalid"), "Negative deposit rejected");
    assertTrue(!sys.deposit("player_1", 0.0, "Zero"), "Zero deposit rejected");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("player_1")), 0.0f), "Balance unchanged");
}

static void testWalletMissingEntity() {
    std::cout << "\n=== PlayerWallet: Missing Entity ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    assertTrue(!sys.deposit("nonexistent", 100.0, "test"), "Deposit fails on missing");
    assertTrue(!sys.withdraw("nonexistent", 100.0, "test"), "Withdraw fails on missing");
    assertTrue(approxEqual(static_cast<float>(sys.getBalance("nonexistent")), 0.0f), "Balance is 0 for missing");
    assertTrue(approxEqual(static_cast<float>(sys.getLifetimeEarned("nonexistent")), 0.0f), "Earned is 0 for missing");
    assertTrue(approxEqual(static_cast<float>(sys.getLifetimeSpent("nonexistent")), 0.0f), "Spent is 0 for missing");
    assertTrue(sys.getTransactionCount("nonexistent") == 0, "0 transactions for missing");
}

static void testWalletMultipleDepositsWithdraws() {
    std::cout << "\n=== PlayerWallet: Multiple Deposits/Withdraws ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerWallet>(e);

    sys.deposit("player_1", 1000.0, "Mining 1");
    sys.deposit("player_1", 2000.0, "Mining 2");
    sys.deposit("player_1", 3000.0, "Mission reward");
    sys.withdraw("player_1", 500.0, "Repair");
    sys.withdraw("player_1", 1500.0, "Module buy");

    assertTrue(approxEqual(static_cast<float>(sys.getBalance("player_1")), 4000.0f), "Balance is 4000");
    assertTrue(approxEqual(static_cast<float>(sys.getLifetimeEarned("player_1")), 6000.0f), "Earned 6000 total");
    assertTrue(approxEqual(static_cast<float>(sys.getLifetimeSpent("player_1")), 2000.0f), "Spent 2000 total");
    assertTrue(sys.getTransactionCount("player_1") == 5, "5 transactions");
}

static void testWalletUpdateTimestamp() {
    std::cout << "\n=== PlayerWallet: Update Timestamp ===" << std::endl;
    ecs::World world;
    systems::PlayerWalletSystem sys(&world);

    auto* e = world.createEntity("player_1");
    addComp<components::PlayerWallet>(e);

    sys.update(10.0f);
    sys.deposit("player_1", 100.0, "Test");

    auto* wallet = e->getComponent<components::PlayerWallet>();
    assertTrue(approxEqual(wallet->elapsed, 10.0f), "Elapsed time updated");
    assertTrue(approxEqual(wallet->transactions.back().timestamp, 10.0f), "Transaction timestamp is 10s");
}

void run_player_wallet_system_tests() {
    testWalletDeposit();
    testWalletWithdraw();
    testWalletInsufficientFunds();
    testWalletTransferWithTax();
    testWalletTransferInsufficientWithTax();
    testWalletSetTaxRate();
    testWalletTransactionLog();
    testWalletMaxTransactions();
    testWalletNegativeDeposit();
    testWalletMissingEntity();
    testWalletMultipleDepositsWithdraws();
    testWalletUpdateTimestamp();
}
