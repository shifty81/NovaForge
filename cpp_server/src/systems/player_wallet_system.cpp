#include "systems/player_wallet_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PlayerWalletSystem::PlayerWalletSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PlayerWalletSystem::updateComponent(ecs::Entity& /*entity*/,
    components::PlayerWallet& wallet, float delta_time) {
    if (!wallet.active) return;
    wallet.elapsed += delta_time;
}

void PlayerWalletSystem::recordTransaction(components::PlayerWallet* wallet,
    const std::string& id, const std::string& desc, double amount) {
    if (static_cast<int>(wallet->transactions.size()) >= wallet->max_transactions) {
        wallet->transactions.erase(wallet->transactions.begin());
    }
    components::PlayerWallet::Transaction txn;
    txn.transaction_id = id;
    txn.description = desc;
    txn.amount = amount;
    txn.balance_after = wallet->balance;
    txn.timestamp = wallet->elapsed;
    wallet->transactions.push_back(txn);
}

bool PlayerWalletSystem::deposit(const std::string& entity_id, double amount,
    const std::string& description) {
    if (amount <= 0.0) return false;
    auto* wallet = getComponentFor(entity_id);
    if (!wallet) return false;
    wallet->balance += amount;
    wallet->lifetime_earned += amount;
    recordTransaction(wallet, "dep_" + std::to_string(static_cast<int>(wallet->transactions.size())),
                      description, amount);
    return true;
}

bool PlayerWalletSystem::withdraw(const std::string& entity_id, double amount,
    const std::string& description) {
    if (amount <= 0.0) return false;
    auto* wallet = getComponentFor(entity_id);
    if (!wallet) return false;
    if (wallet->balance < amount) return false;
    wallet->balance -= amount;
    wallet->lifetime_spent += amount;
    recordTransaction(wallet, "wd_" + std::to_string(static_cast<int>(wallet->transactions.size())),
                      description, -amount);
    return true;
}

bool PlayerWalletSystem::transferWithTax(const std::string& from_id,
    const std::string& to_id, double amount, const std::string& description) {
    if (amount <= 0.0) return false;
    auto* from_wallet = getComponentFor(from_id);
    auto* to_wallet = getComponentFor(to_id);
    if (!from_wallet || !to_wallet) return false;

    double tax = amount * from_wallet->tax_rate;
    double total_debit = amount + tax;
    if (from_wallet->balance < total_debit) return false;

    from_wallet->balance -= total_debit;
    from_wallet->lifetime_spent += total_debit;
    recordTransaction(from_wallet, "xfr_out_" + std::to_string(static_cast<int>(from_wallet->transactions.size())),
                      description + " (tax: " + std::to_string(tax) + ")", -total_debit);

    to_wallet->balance += amount;
    to_wallet->lifetime_earned += amount;
    recordTransaction(to_wallet, "xfr_in_" + std::to_string(static_cast<int>(to_wallet->transactions.size())),
                      description, amount);
    return true;
}

double PlayerWalletSystem::getBalance(const std::string& entity_id) const {
    auto* wallet = getComponentFor(entity_id);
    return wallet ? wallet->balance : 0.0;
}

double PlayerWalletSystem::getLifetimeEarned(const std::string& entity_id) const {
    auto* wallet = getComponentFor(entity_id);
    return wallet ? wallet->lifetime_earned : 0.0;
}

double PlayerWalletSystem::getLifetimeSpent(const std::string& entity_id) const {
    auto* wallet = getComponentFor(entity_id);
    return wallet ? wallet->lifetime_spent : 0.0;
}

double PlayerWalletSystem::getTaxRate(const std::string& entity_id) const {
    auto* wallet = getComponentFor(entity_id);
    return wallet ? wallet->tax_rate : 0.0;
}

bool PlayerWalletSystem::setTaxRate(const std::string& entity_id, double rate) {
    if (rate < 0.0 || rate > 1.0) return false;
    auto* wallet = getComponentFor(entity_id);
    if (!wallet) return false;
    wallet->tax_rate = rate;
    return true;
}

int PlayerWalletSystem::getTransactionCount(const std::string& entity_id) const {
    auto* wallet = getComponentFor(entity_id);
    return wallet ? static_cast<int>(wallet->transactions.size()) : 0;
}

double PlayerWalletSystem::getLastTransactionAmount(const std::string& entity_id) const {
    auto* wallet = getComponentFor(entity_id);
    if (!wallet || wallet->transactions.empty()) return 0.0;
    return wallet->transactions.back().amount;
}

} // namespace systems
} // namespace atlas
