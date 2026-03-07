#ifndef NOVAFORGE_SYSTEMS_INSURANCE_CLAIM_SYSTEM_H
#define NOVAFORGE_SYSTEMS_INSURANCE_CLAIM_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship insurance system — buy policies and claim payouts on destruction
 *
 * Players purchase an insurance tier for their current ship.  The policy
 * ticks down in real time; if the ship is destroyed while the policy is
 * active, the player receives the payout.  Expired policies are marked
 * and must be renewed.  Only one policy at a time per entity.
 */
class InsuranceClaimSystem : public ecs::SingleComponentSystem<components::InsuranceClaim> {
public:
    explicit InsuranceClaimSystem(ecs::World* world);
    ~InsuranceClaimSystem() override = default;

    std::string getName() const override { return "InsuranceClaimSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& ship_id,
                    const std::string& owner_id);
    bool addTier(const std::string& entity_id, const std::string& tier_name,
                 double premium, double payout, float duration);
    bool purchasePolicy(const std::string& entity_id, const std::string& tier_name);
    bool fileClaim(const std::string& entity_id);
    bool cancelPolicy(const std::string& entity_id);
    int getTierCount(const std::string& entity_id) const;
    std::string getPolicyState(const std::string& entity_id) const;
    std::string getActiveTier(const std::string& entity_id) const;
    double getActivePayout(const std::string& entity_id) const;
    float getTimeRemaining(const std::string& entity_id) const;
    double getTotalPremiumsPaid(const std::string& entity_id) const;
    double getTotalPayoutsReceived(const std::string& entity_id) const;
    int getTotalClaims(const std::string& entity_id) const;
    int getTotalPoliciesPurchased(const std::string& entity_id) const;
    int getTotalPoliciesExpired(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::InsuranceClaim& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_INSURANCE_CLAIM_SYSTEM_H
