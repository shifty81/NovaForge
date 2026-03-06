#include "systems/wallet_transaction_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

WalletTransactionSystem::WalletTransactionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void WalletTransactionSystem::updateComponent(ecs::Entity& entity,
    components::WalletLedger& ledger, float delta_time) {
    if (!ledger.active) return;
    ledger.elapsed += delta_time;
}

bool WalletTransactionSystem::initialize(const std::string& entity_id,
    double starting_balance) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::WalletLedger>();
    comp->balance = std::max(0.0, starting_balance);
    entity->addComponent(std::move(comp));
    return true;
}

void WalletTransactionSystem::addTransaction(components::WalletLedger& ledger,
    const std::string& tx_id, double amount, const std::string& category,
    const std::string& description) {
    components::WalletLedger::Transaction tx;
    tx.tx_id = tx_id;
    tx.description = description;
    tx.category = category;
    tx.amount = amount;
    tx.balance_after = ledger.balance;
    tx.timestamp = ledger.elapsed;

    if (static_cast<int>(ledger.transactions.size()) >= ledger.max_transactions) {
        ledger.transactions.erase(ledger.transactions.begin());
    }
    ledger.transactions.push_back(tx);
    ledger.total_tx_count++;
}

bool WalletTransactionSystem::deposit(const std::string& entity_id,
    const std::string& tx_id, double amount, const std::string& category,
    const std::string& description) {
    if (amount <= 0.0) return false;
    auto* ledger = getComponentFor(entity_id);
    if (!ledger) return false;

    ledger->balance += amount;
    ledger->total_earned += amount;
    addTransaction(*ledger, tx_id, amount, category, description);
    return true;
}

bool WalletTransactionSystem::withdraw(const std::string& entity_id,
    const std::string& tx_id, double amount, const std::string& category,
    const std::string& description) {
    if (amount <= 0.0) return false;
    auto* ledger = getComponentFor(entity_id);
    if (!ledger) return false;
    if (ledger->balance < amount) return false;

    ledger->balance -= amount;
    ledger->total_spent += amount;
    addTransaction(*ledger, tx_id, -amount, category, description);
    return true;
}

bool WalletTransactionSystem::transfer(const std::string& from_id,
    const std::string& to_id, const std::string& tx_id, double amount,
    const std::string& description) {
    if (amount <= 0.0) return false;
    auto* from_ledger = getComponentFor(from_id);
    auto* to_ledger = getComponentFor(to_id);
    if (!from_ledger || !to_ledger) return false;
    if (from_ledger->balance < amount) return false;

    from_ledger->balance -= amount;
    from_ledger->total_spent += amount;
    addTransaction(*from_ledger, tx_id, -amount, "Transfer", description);

    to_ledger->balance += amount;
    to_ledger->total_earned += amount;
    addTransaction(*to_ledger, tx_id, amount, "Transfer", description);
    return true;
}

double WalletTransactionSystem::getBalance(const std::string& entity_id) const {
    auto* ledger = getComponentFor(entity_id);
    return ledger ? ledger->balance : 0.0;
}

double WalletTransactionSystem::getTotalEarned(const std::string& entity_id) const {
    auto* ledger = getComponentFor(entity_id);
    return ledger ? ledger->total_earned : 0.0;
}

double WalletTransactionSystem::getTotalSpent(const std::string& entity_id) const {
    auto* ledger = getComponentFor(entity_id);
    return ledger ? ledger->total_spent : 0.0;
}

int WalletTransactionSystem::getTransactionCount(const std::string& entity_id) const {
    auto* ledger = getComponentFor(entity_id);
    return ledger ? ledger->total_tx_count : 0;
}

double WalletTransactionSystem::getCategoryTotal(const std::string& entity_id,
    const std::string& category) const {
    auto* ledger = getComponentFor(entity_id);
    if (!ledger) return 0.0;
    double total = 0.0;
    for (const auto& tx : ledger->transactions) {
        if (tx.category == category) {
            total += tx.amount;
        }
    }
    return total;
}

bool WalletTransactionSystem::canAfford(const std::string& entity_id, double amount) const {
    auto* ledger = getComponentFor(entity_id);
    return ledger && ledger->balance >= amount;
}

} // namespace systems
} // namespace atlas
