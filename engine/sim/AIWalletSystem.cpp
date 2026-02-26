#include "sim/AIWalletSystem.h"
#include <algorithm>

namespace atlas::sim {

bool AIWalletSystem::CreateWallet(uint32_t entityId, float initialBalance) {
    if (initialBalance < 0.0f) return false;
    for (auto& w : m_wallets) {
        if (w.entityId == entityId) return false;
    }
    AIWallet wallet;
    wallet.entityId = entityId;
    wallet.balance  = initialBalance;
    m_wallets.push_back(wallet);
    return true;
}

bool AIWalletSystem::RemoveWallet(uint32_t entityId) {
    auto it = std::remove_if(m_wallets.begin(), m_wallets.end(),
        [entityId](const AIWallet& w) { return w.entityId == entityId; });
    if (it == m_wallets.end()) return false;
    m_wallets.erase(it, m_wallets.end());
    return true;
}

bool AIWalletSystem::Deposit(uint32_t entityId, float amount) {
    if (amount <= 0.0f) return false;
    AIWallet* w = findWallet(entityId);
    if (!w) return false;
    w->balance += amount;
    w->totalIncome += amount;
    w->transactionCount++;
    return true;
}

bool AIWalletSystem::Withdraw(uint32_t entityId, float amount) {
    if (amount <= 0.0f) return false;
    AIWallet* w = findWallet(entityId);
    if (!w || w->balance < amount) return false;
    w->balance -= amount;
    w->totalExpenses += amount;
    w->transactionCount++;
    return true;
}

bool AIWalletSystem::Transfer(uint32_t fromId, uint32_t toId, float amount) {
    if (amount <= 0.0f || fromId == toId) return false;
    AIWallet* from = findWallet(fromId);
    AIWallet* to   = findWallet(toId);
    if (!from || !to || from->balance < amount) return false;
    from->balance -= amount;
    from->totalExpenses += amount;
    from->transactionCount++;
    to->balance += amount;
    to->totalIncome += amount;
    to->transactionCount++;
    return true;
}

const AIWallet* AIWalletSystem::GetWallet(uint32_t entityId) const {
    for (auto& w : m_wallets) {
        if (w.entityId == entityId) return &w;
    }
    return nullptr;
}

float AIWalletSystem::GetBalance(uint32_t entityId) const {
    for (auto& w : m_wallets) {
        if (w.entityId == entityId) return w.balance;
    }
    return 0.0f;
}

float AIWalletSystem::TotalCirculation() const {
    float sum = 0.0f;
    for (auto& w : m_wallets) {
        sum += w.balance;
    }
    return sum;
}

uint32_t AIWalletSystem::WealthiestEntity() const {
    if (m_wallets.empty()) return 0;
    const AIWallet* best = &m_wallets[0];
    for (auto& w : m_wallets) {
        if (w.balance > best->balance) {
            best = &w;
        }
    }
    return best->entityId;
}

void AIWalletSystem::Clear() {
    m_wallets.clear();
}

AIWallet* AIWalletSystem::findWallet(uint32_t entityId) {
    for (auto& w : m_wallets) {
        if (w.entityId == entityId) return &w;
    }
    return nullptr;
}

} // namespace atlas::sim
