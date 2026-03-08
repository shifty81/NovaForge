#include "systems/ship_insurance_payout_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ShipInsurancePayoutSystem::ShipInsurancePayoutSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ShipInsurancePayoutSystem::updateComponent(ecs::Entity& entity,
    components::ShipInsurancePayoutState& state, float delta_time) {
    if (!state.active) return;

    int active = 0;
    for (const auto& p : state.policies) {
        if (!p.claimed) active++;
    }
    state.total_active_policies = active;
    state.elapsed += delta_time;
}

float ShipInsurancePayoutSystem::computePremium(const std::string& tier, float ship_value) const {
    float rate = 0.10f; // basic: 10%
    if (tier == "standard") rate = 0.20f;
    else if (tier == "platinum") rate = 0.35f;
    return ship_value * rate;
}

float ShipInsurancePayoutSystem::computeCoverage(const std::string& tier) const {
    if (tier == "basic") return 0.50f;
    if (tier == "standard") return 0.75f;
    if (tier == "platinum") return 1.00f;
    return 0.50f;
}

bool ShipInsurancePayoutSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ShipInsurancePayoutState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipInsurancePayoutSystem::addPolicy(const std::string& entity_id,
    const std::string& policy_id, const std::string& ship_id,
    const std::string& tier, float ship_value) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (policy_id.empty() || ship_id.empty() || tier.empty()) return false;
    if (ship_value <= 0.0f) return false;
    if (static_cast<int>(state->policies.size()) >= state->max_policies) return false;
    // Check duplicate policy_id
    for (const auto& p : state->policies) {
        if (p.policy_id == policy_id) return false;
    }
    components::ShipInsurancePayoutState::Policy policy;
    policy.policy_id = policy_id;
    policy.ship_id = ship_id;
    policy.tier = tier;
    policy.coverage = computeCoverage(tier);
    policy.premium = computePremium(tier, ship_value);
    policy.ship_value = ship_value;
    state->policies.push_back(policy);
    state->total_premiums_collected += policy.premium;
    return true;
}

bool ShipInsurancePayoutSystem::removePolicy(const std::string& entity_id,
    const std::string& policy_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->policies.begin(), state->policies.end(),
        [&](const components::ShipInsurancePayoutState::Policy& p) {
            return p.policy_id == policy_id;
        });
    if (it == state->policies.end()) return false;
    state->policies.erase(it);
    return true;
}

bool ShipInsurancePayoutSystem::fileClaim(const std::string& entity_id,
    const std::string& policy_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->policies.begin(), state->policies.end(),
        [&](const components::ShipInsurancePayoutState::Policy& p) {
            return p.policy_id == policy_id;
        });
    if (it == state->policies.end()) return false;
    if (it->claimed) return false;
    it->claimed = true;
    it->claim_time = state->elapsed;
    float payout = it->ship_value * it->coverage;
    state->total_payouts_issued += payout;
    state->total_claims++;
    return true;
}

float ShipInsurancePayoutSystem::getClaimPayout(const std::string& entity_id,
    const std::string& policy_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    for (const auto& p : state->policies) {
        if (p.policy_id == policy_id) {
            return p.ship_value * p.coverage;
        }
    }
    return 0.0f;
}

int ShipInsurancePayoutSystem::getPolicyCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->policies.size()) : 0;
}

int ShipInsurancePayoutSystem::getActiveCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& p : state->policies) {
        if (!p.claimed) count++;
    }
    return count;
}

float ShipInsurancePayoutSystem::getTotalPremiums(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_premiums_collected : 0.0f;
}

float ShipInsurancePayoutSystem::getTotalPayouts(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_payouts_issued : 0.0f;
}

float ShipInsurancePayoutSystem::getProfitLoss(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->profitLoss() : 0.0f;
}

int ShipInsurancePayoutSystem::getTotalClaims(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_claims : 0;
}

} // namespace systems
} // namespace atlas
