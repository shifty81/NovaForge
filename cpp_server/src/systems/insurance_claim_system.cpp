#include "systems/insurance_claim_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using IC = components::InsuranceClaim;

const char* stateToString(IC::PolicyState s) {
    switch (s) {
        case IC::PolicyState::Uninsured:    return "Uninsured";
        case IC::PolicyState::Active:       return "Active";
        case IC::PolicyState::ClaimPending: return "ClaimPending";
        case IC::PolicyState::ClaimPaid:    return "ClaimPaid";
        case IC::PolicyState::Expired:      return "Expired";
    }
    return "Unknown";
}

} // anonymous namespace

InsuranceClaimSystem::InsuranceClaimSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void InsuranceClaimSystem::updateComponent(ecs::Entity& entity,
    components::InsuranceClaim& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.state == IC::PolicyState::Active) {
        comp.policy_time_remaining -= delta_time;
        if (comp.policy_time_remaining <= 0.0f) {
            comp.policy_time_remaining = 0.0f;
            comp.state = IC::PolicyState::Expired;
            comp.total_policies_expired++;
        }
    }
}

bool InsuranceClaimSystem::initialize(const std::string& entity_id,
    const std::string& ship_id, const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::InsuranceClaim>();
    comp->ship_id = ship_id;
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool InsuranceClaimSystem::addTier(const std::string& entity_id,
    const std::string& tier_name, double premium, double payout, float duration) {
    auto* ic = getComponentFor(entity_id);
    if (!ic) return false;
    for (const auto& t : ic->available_tiers) {
        if (t.tier_name == tier_name) return false;
    }
    IC::PolicyTier tier;
    tier.tier_name = tier_name;
    tier.premium_cost = premium;
    tier.payout_amount = payout;
    tier.duration = duration;
    ic->available_tiers.push_back(tier);
    return true;
}

bool InsuranceClaimSystem::purchasePolicy(const std::string& entity_id,
    const std::string& tier_name) {
    auto* ic = getComponentFor(entity_id);
    if (!ic) return false;
    if (ic->state == IC::PolicyState::Active) return false; // already insured

    const IC::PolicyTier* found = nullptr;
    for (const auto& t : ic->available_tiers) {
        if (t.tier_name == tier_name) { found = &t; break; }
    }
    if (!found) return false;

    ic->active_tier_name = found->tier_name;
    ic->active_premium_paid = found->premium_cost;
    ic->active_payout = found->payout_amount;
    ic->policy_time_remaining = found->duration;
    ic->total_premiums_paid += found->premium_cost;
    ic->total_policies_purchased++;
    ic->state = IC::PolicyState::Active;
    return true;
}

bool InsuranceClaimSystem::fileClaim(const std::string& entity_id) {
    auto* ic = getComponentFor(entity_id);
    if (!ic) return false;
    if (ic->state != IC::PolicyState::Active) return false;

    ic->total_payouts_received += ic->active_payout;
    ic->total_claims++;
    ic->state = IC::PolicyState::ClaimPaid;
    return true;
}

bool InsuranceClaimSystem::cancelPolicy(const std::string& entity_id) {
    auto* ic = getComponentFor(entity_id);
    if (!ic) return false;
    if (ic->state != IC::PolicyState::Active) return false;
    ic->state = IC::PolicyState::Uninsured;
    ic->active_tier_name.clear();
    ic->active_payout = 0.0;
    ic->policy_time_remaining = 0.0f;
    return true;
}

int InsuranceClaimSystem::getTierCount(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    return ic ? static_cast<int>(ic->available_tiers.size()) : 0;
}

std::string InsuranceClaimSystem::getPolicyState(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    if (!ic) return "Unknown";
    return stateToString(ic->state);
}

std::string InsuranceClaimSystem::getActiveTier(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    if (!ic) return "";
    return ic->active_tier_name;
}

double InsuranceClaimSystem::getActivePayout(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    return ic ? ic->active_payout : 0.0;
}

float InsuranceClaimSystem::getTimeRemaining(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    return ic ? ic->policy_time_remaining : 0.0f;
}

double InsuranceClaimSystem::getTotalPremiumsPaid(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    return ic ? ic->total_premiums_paid : 0.0;
}

double InsuranceClaimSystem::getTotalPayoutsReceived(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    return ic ? ic->total_payouts_received : 0.0;
}

int InsuranceClaimSystem::getTotalClaims(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    return ic ? ic->total_claims : 0;
}

int InsuranceClaimSystem::getTotalPoliciesPurchased(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    return ic ? ic->total_policies_purchased : 0;
}

int InsuranceClaimSystem::getTotalPoliciesExpired(const std::string& entity_id) const {
    auto* ic = getComponentFor(entity_id);
    return ic ? ic->total_policies_expired : 0;
}

} // namespace systems
} // namespace atlas
