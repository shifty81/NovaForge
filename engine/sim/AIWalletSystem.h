#pragma once
#include <cstdint>
#include <vector>

namespace atlas::sim {

/**
 * Credits wallet for a single NPC entity.
 */
struct AIWallet {
    uint32_t entityId         = 0;
    float    balance          = 0.0f;
    float    totalIncome      = 0.0f;
    float    totalExpenses    = 0.0f;
    uint32_t transactionCount = 0;
};

/**
 * @brief NPC credits tracking system.
 *
 * Manages wallets for AI entities, supporting deposits, withdrawals,
 * and atomic transfers between entities.
 *
 * Usage:
 *   AIWalletSystem ws;
 *   ws.CreateWallet(1, 500.0f);
 *   ws.CreateWallet(2, 100.0f);
 *   ws.Deposit(1, 200.0f);
 *   ws.Transfer(1, 2, 50.0f);
 */
class AIWalletSystem {
public:
    AIWalletSystem() = default;

    /** Create a wallet for an entity. Returns false if already exists. */
    bool CreateWallet(uint32_t entityId, float initialBalance = 0.0f);

    /** Remove a wallet by entity id. */
    bool RemoveWallet(uint32_t entityId);

    /** Deposit credits into a wallet. Returns false if not found. */
    bool Deposit(uint32_t entityId, float amount);

    /** Withdraw credits. Returns false if not found or insufficient funds. */
    bool Withdraw(uint32_t entityId, float amount);

    /** Atomic transfer between two wallets. Returns false on failure. */
    bool Transfer(uint32_t fromId, uint32_t toId, float amount);

    /** Get read-only wallet. Returns nullptr if not found. */
    const AIWallet* GetWallet(uint32_t entityId) const;

    /** Get balance for an entity. Returns 0 if not found. */
    float GetBalance(uint32_t entityId) const;

    /** Number of tracked wallets. */
    size_t WalletCount() const { return m_wallets.size(); }

    /** Sum of all wallet balances. */
    float TotalCirculation() const;

    /** Entity id with the highest balance. Returns 0 if no wallets. */
    uint32_t WealthiestEntity() const;

    /** Remove all wallets. */
    void Clear();

private:
    AIWallet* findWallet(uint32_t entityId);

    std::vector<AIWallet> m_wallets;
};

} // namespace atlas::sim
