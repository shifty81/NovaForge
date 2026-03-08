#ifndef NOVAFORGE_SYSTEMS_SHIP_INSURANCE_PAYOUT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_INSURANCE_PAYOUT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Processes insurance claims when ships are destroyed
 *
 * Manages insurance policies with multiple tiers, processes claims,
 * and tracks premiums collected vs payouts issued. Integrates with
 * the economy system for ISC transfers.
 */
class ShipInsurancePayoutSystem : public ecs::SingleComponentSystem<components::ShipInsurancePayoutState> {
public:
    explicit ShipInsurancePayoutSystem(ecs::World* world);
    ~ShipInsurancePayoutSystem() override = default;

    std::string getName() const override { return "ShipInsurancePayoutSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addPolicy(const std::string& entity_id, const std::string& policy_id,
                   const std::string& ship_id, const std::string& tier,
                   float ship_value);
    bool removePolicy(const std::string& entity_id, const std::string& policy_id);
    bool fileClaim(const std::string& entity_id, const std::string& policy_id);
    float getClaimPayout(const std::string& entity_id, const std::string& policy_id) const;
    int getPolicyCount(const std::string& entity_id) const;
    int getActiveCount(const std::string& entity_id) const;
    float getTotalPremiums(const std::string& entity_id) const;
    float getTotalPayouts(const std::string& entity_id) const;
    float getProfitLoss(const std::string& entity_id) const;
    int getTotalClaims(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ShipInsurancePayoutState& state, float delta_time) override;

private:
    float computePremium(const std::string& tier, float ship_value) const;
    float computeCoverage(const std::string& tier) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_INSURANCE_PAYOUT_SYSTEM_H
