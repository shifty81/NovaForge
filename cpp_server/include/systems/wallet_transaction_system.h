#ifndef NOVAFORGE_SYSTEMS_WALLET_TRANSACTION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_WALLET_TRANSACTION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Player wallet transaction ledger system
 *
 * Manages player ISK balances with deposit/withdrawal operations and
 * full transaction history. Supports the trade/economy loop in the
 * vertical slice end-to-end gameplay.
 */
class WalletTransactionSystem : public ecs::SingleComponentSystem<components::WalletLedger> {
public:
    explicit WalletTransactionSystem(ecs::World* world);
    ~WalletTransactionSystem() override = default;

    std::string getName() const override { return "WalletTransactionSystem"; }

public:
    bool initialize(const std::string& entity_id, double starting_balance);

    // Transactions
    bool deposit(const std::string& entity_id, const std::string& tx_id,
                 double amount, const std::string& category, const std::string& description);
    bool withdraw(const std::string& entity_id, const std::string& tx_id,
                  double amount, const std::string& category, const std::string& description);
    bool transfer(const std::string& from_id, const std::string& to_id,
                  const std::string& tx_id, double amount, const std::string& description);

    // Queries
    double getBalance(const std::string& entity_id) const;
    double getTotalEarned(const std::string& entity_id) const;
    double getTotalSpent(const std::string& entity_id) const;
    int getTransactionCount(const std::string& entity_id) const;
    double getCategoryTotal(const std::string& entity_id, const std::string& category) const;
    bool canAfford(const std::string& entity_id, double amount) const;

protected:
    void updateComponent(ecs::Entity& entity, components::WalletLedger& ledger, float delta_time) override;

private:
    void addTransaction(components::WalletLedger& ledger, const std::string& tx_id,
                        double amount, const std::string& category,
                        const std::string& description);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_WALLET_TRANSACTION_SYSTEM_H
