#include "systems/bounty_payout_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

BountyPayoutSystem::BountyPayoutSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void BountyPayoutSystem::updateComponent(ecs::Entity& entity,
    components::BountyPayout& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Process pending payouts
    for (auto& entry : comp.pending_payouts) {
        if (!entry.processed) {
            entry.final_payout = entry.base_bounty * comp.payout_multiplier;
            entry.processed = true;
            comp.total_isc_paid += entry.final_payout;
            comp.total_payouts_processed++;
        }
    }

    // Remove processed payouts
    comp.pending_payouts.erase(
        std::remove_if(comp.pending_payouts.begin(), comp.pending_payouts.end(),
            [](const components::BountyPayout::PayoutEntry& e) { return e.processed; }),
        comp.pending_payouts.end());
}

bool BountyPayoutSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::BountyPayout>();
    entity->addComponent(std::move(comp));
    return true;
}

bool BountyPayoutSystem::recordKill(const std::string& entity_id,
    const std::string& killer_id, const std::string& victim_id,
    const std::string& victim_type, double base_bounty) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->pending_payouts.size()) >= comp->max_pending) return false;

    components::BountyPayout::PayoutEntry entry;
    entry.killer_id = killer_id;
    entry.victim_id = victim_id;
    entry.victim_type = victim_type;
    entry.base_bounty = base_bounty;
    entry.timestamp = comp->elapsed;
    comp->pending_payouts.push_back(entry);
    return true;
}

bool BountyPayoutSystem::setPayoutMultiplier(const std::string& entity_id,
    float multiplier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->payout_multiplier = multiplier;
    return true;
}

int BountyPayoutSystem::getPendingCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->pending_payouts.size()) : 0;
}

double BountyPayoutSystem::getTotalIscPaid(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_isc_paid : 0.0;
}

int BountyPayoutSystem::getTotalPayoutsProcessed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_payouts_processed : 0;
}

float BountyPayoutSystem::getPayoutMultiplier(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->payout_multiplier : 0.0f;
}

} // namespace systems
} // namespace atlas
