#ifndef NOVAFORGE_SYSTEMS_PLAYER_WALLET_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_WALLET_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages player wallets, Credits transactions, and taxation
 *
 * Provides deposit, withdraw, and tax-deducted transfer operations.
 * Records all transactions with timestamps for audit trail.
 */
class PlayerWalletSystem : public ecs::SingleComponentSystem<components::PlayerWallet> {
public:
    explicit PlayerWalletSystem(ecs::World* world);
    ~PlayerWalletSystem() override = default;

    std::string getName() const override { return "PlayerWalletSystem"; }

    bool deposit(const std::string& entity_id, double amount, const std::string& description);
    bool withdraw(const std::string& entity_id, double amount, const std::string& description);
    bool transferWithTax(const std::string& from_id, const std::string& to_id,
                         double amount, const std::string& description);
    double getBalance(const std::string& entity_id) const;
    double getLifetimeEarned(const std::string& entity_id) const;
    double getLifetimeSpent(const std::string& entity_id) const;
    double getTaxRate(const std::string& entity_id) const;
    bool setTaxRate(const std::string& entity_id, double rate);
    int getTransactionCount(const std::string& entity_id) const;
    double getLastTransactionAmount(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PlayerWallet& wallet, float delta_time) override;

private:
    void recordTransaction(components::PlayerWallet* wallet, const std::string& id,
                           const std::string& desc, double amount);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_WALLET_SYSTEM_H
