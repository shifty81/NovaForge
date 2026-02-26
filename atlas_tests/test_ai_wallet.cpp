/**
 * Tests for AIWalletSystem — NPC credits tracking with deposits,
 * withdrawals, and atomic transfers between entities.
 */

#include <cassert>
#include <cmath>
#include "../engine/sim/AIWalletSystem.h"

using namespace atlas::sim;

static bool approxEq(float a, float b, float eps = 0.01f) {
    return std::fabs(a - b) < eps;
}

// ══════════════════════════════════════════════════════════════════
// Defaults and construction
// ══════════════════════════════════════════════════════════════════

void test_wallet_defaults() {
    AIWalletSystem ws;
    assert(ws.WalletCount() == 0);
    assert(approxEq(ws.TotalCirculation(), 0.0f));
    assert(ws.WealthiestEntity() == 0);
}

void test_wallet_create() {
    AIWalletSystem ws;
    assert(ws.CreateWallet(1, 500.0f));
    assert(ws.WalletCount() == 1);
    const AIWallet* w = ws.GetWallet(1);
    assert(w != nullptr);
    assert(approxEq(w->balance, 500.0f));
    assert(approxEq(w->totalIncome, 0.0f));
    assert(approxEq(w->totalExpenses, 0.0f));
    assert(w->transactionCount == 0);
}

void test_wallet_create_duplicate_rejected() {
    AIWalletSystem ws;
    assert(ws.CreateWallet(1, 100.0f));
    assert(!ws.CreateWallet(1, 200.0f));
    assert(ws.WalletCount() == 1);
    assert(approxEq(ws.GetBalance(1), 100.0f)); // original unchanged
}

void test_wallet_create_negative_balance_rejected() {
    AIWalletSystem ws;
    assert(!ws.CreateWallet(1, -50.0f));
    assert(ws.WalletCount() == 0);
}

void test_wallet_remove() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 100.0f);
    assert(ws.RemoveWallet(1));
    assert(ws.WalletCount() == 0);
    assert(ws.GetWallet(1) == nullptr);
}

void test_wallet_remove_nonexistent() {
    AIWalletSystem ws;
    assert(!ws.RemoveWallet(999));
}

// ══════════════════════════════════════════════════════════════════
// Deposits and withdrawals
// ══════════════════════════════════════════════════════════════════

void test_wallet_deposit() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 100.0f);
    assert(ws.Deposit(1, 50.0f));
    const AIWallet* w = ws.GetWallet(1);
    assert(approxEq(w->balance, 150.0f));
    assert(approxEq(w->totalIncome, 50.0f));
    assert(w->transactionCount == 1);
}

void test_wallet_deposit_nonexistent() {
    AIWalletSystem ws;
    assert(!ws.Deposit(999, 50.0f));
}

void test_wallet_deposit_zero_rejected() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 100.0f);
    assert(!ws.Deposit(1, 0.0f));
    assert(!ws.Deposit(1, -10.0f));
}

void test_wallet_withdraw() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 200.0f);
    assert(ws.Withdraw(1, 75.0f));
    const AIWallet* w = ws.GetWallet(1);
    assert(approxEq(w->balance, 125.0f));
    assert(approxEq(w->totalExpenses, 75.0f));
    assert(w->transactionCount == 1);
}

void test_wallet_withdraw_insufficient() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 50.0f);
    assert(!ws.Withdraw(1, 100.0f));
    assert(approxEq(ws.GetBalance(1), 50.0f)); // unchanged
}

void test_wallet_withdraw_zero_rejected() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 100.0f);
    assert(!ws.Withdraw(1, 0.0f));
    assert(!ws.Withdraw(1, -10.0f));
}

// ══════════════════════════════════════════════════════════════════
// Transfers
// ══════════════════════════════════════════════════════════════════

void test_wallet_transfer() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 500.0f);
    ws.CreateWallet(2, 100.0f);
    assert(ws.Transfer(1, 2, 200.0f));
    assert(approxEq(ws.GetBalance(1), 300.0f));
    assert(approxEq(ws.GetBalance(2), 300.0f));
    // Check transaction counts
    assert(ws.GetWallet(1)->transactionCount == 1);
    assert(ws.GetWallet(2)->transactionCount == 1);
}

void test_wallet_transfer_insufficient() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 50.0f);
    ws.CreateWallet(2, 100.0f);
    assert(!ws.Transfer(1, 2, 100.0f));
    assert(approxEq(ws.GetBalance(1), 50.0f));  // unchanged
    assert(approxEq(ws.GetBalance(2), 100.0f)); // unchanged
}

void test_wallet_transfer_self_rejected() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 500.0f);
    assert(!ws.Transfer(1, 1, 100.0f));
    assert(approxEq(ws.GetBalance(1), 500.0f)); // unchanged
}

// ══════════════════════════════════════════════════════════════════
// Aggregate queries
// ══════════════════════════════════════════════════════════════════

void test_wallet_total_circulation() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 100.0f);
    ws.CreateWallet(2, 200.0f);
    ws.CreateWallet(3, 300.0f);
    assert(approxEq(ws.TotalCirculation(), 600.0f));
}

void test_wallet_wealthiest_entity() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 100.0f);
    ws.CreateWallet(2, 500.0f);
    ws.CreateWallet(3, 200.0f);
    assert(ws.WealthiestEntity() == 2);
}

void test_wallet_clear() {
    AIWalletSystem ws;
    ws.CreateWallet(1, 100.0f);
    ws.CreateWallet(2, 200.0f);
    ws.Clear();
    assert(ws.WalletCount() == 0);
    assert(approxEq(ws.TotalCirculation(), 0.0f));
}

void test_wallet_get_balance_nonexistent() {
    AIWalletSystem ws;
    assert(approxEq(ws.GetBalance(999), 0.0f));
}
