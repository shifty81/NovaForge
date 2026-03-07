#include "systems/combat_after_action_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using CA = components::CombatAfterActionState;
}

CombatAfterActionSystem::CombatAfterActionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CombatAfterActionSystem::updateComponent(ecs::Entity& entity,
    components::CombatAfterActionState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool CombatAfterActionSystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CombatAfterActionState>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool CombatAfterActionSystem::startEngagement(const std::string& entity_id,
    const std::string& engagement_id, const std::string& target_name) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // Check for duplicate engagement_id
    for (const auto& e : state->engagements) {
        if (e.engagement_id == engagement_id) return false;
    }
    if (static_cast<int>(state->engagements.size()) >= state->max_engagements) return false;
    CA::Engagement eng;
    eng.engagement_id = engagement_id;
    eng.target_name = target_name;
    state->engagements.push_back(eng);
    return true;
}

bool CombatAfterActionSystem::recordHit(const std::string& entity_id,
    const std::string& engagement_id, double damage, bool is_incoming) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& e : state->engagements) {
        if (e.engagement_id == engagement_id) {
            if (is_incoming) {
                e.damage_received += damage;
            } else {
                e.damage_dealt += damage;
            }
            return true;
        }
    }
    return false;
}

bool CombatAfterActionSystem::finalizeEngagement(const std::string& entity_id,
    const std::string& engagement_id, float duration, double isc_destroyed) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& e : state->engagements) {
        if (e.engagement_id == engagement_id) {
            if (e.finalized) return false;
            e.finalized = true;
            e.duration = duration;
            e.isc_destroyed = isc_destroyed;
            return true;
        }
    }
    return false;
}

int CombatAfterActionSystem::getEngagementCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->engagements.size()) : 0;
}

double CombatAfterActionSystem::getEngagementDamageDealt(const std::string& entity_id,
    const std::string& engagement_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    for (const auto& e : state->engagements) {
        if (e.engagement_id == engagement_id) return e.damage_dealt;
    }
    return 0.0;
}

double CombatAfterActionSystem::getEngagementDamageReceived(const std::string& entity_id,
    const std::string& engagement_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    for (const auto& e : state->engagements) {
        if (e.engagement_id == engagement_id) return e.damage_received;
    }
    return 0.0;
}

double CombatAfterActionSystem::getEngagementDPS(const std::string& entity_id,
    const std::string& engagement_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    for (const auto& e : state->engagements) {
        if (e.engagement_id == engagement_id) {
            if (e.duration <= 0.0f) return 0.0;
            return e.damage_dealt / static_cast<double>(e.duration);
        }
    }
    return 0.0;
}

bool CombatAfterActionSystem::recordCasualty(const std::string& entity_id,
    const std::string& ship_name, double isc_value) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->casualties.size()) >= state->max_casualties) return false;
    CA::Casualty cas;
    cas.ship_name = ship_name;
    cas.isc_value = isc_value;
    state->casualties.push_back(cas);
    return true;
}

int CombatAfterActionSystem::getCasualtyCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->casualties.size()) : 0;
}

double CombatAfterActionSystem::getTotalIscLost(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    double total = 0.0;
    for (const auto& c : state->casualties) {
        total += c.isc_value;
    }
    return total;
}

double CombatAfterActionSystem::getTotalDamageDealt(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    double total = 0.0;
    for (const auto& e : state->engagements) {
        total += e.damage_dealt;
    }
    return total;
}

double CombatAfterActionSystem::getTotalDamageReceived(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    double total = 0.0;
    for (const auto& e : state->engagements) {
        total += e.damage_received;
    }
    return total;
}

double CombatAfterActionSystem::getTotalIscDestroyed(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    double total = 0.0;
    for (const auto& e : state->engagements) {
        total += e.isc_destroyed;
    }
    return total;
}

double CombatAfterActionSystem::getAverageDPS(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0;
    int finalized_count = 0;
    double total_dps = 0.0;
    for (const auto& e : state->engagements) {
        if (e.finalized && e.duration > 0.0f) {
            total_dps += e.damage_dealt / static_cast<double>(e.duration);
            finalized_count++;
        }
    }
    if (finalized_count == 0) return 0.0;
    return total_dps / finalized_count;
}

} // namespace systems
} // namespace atlas
