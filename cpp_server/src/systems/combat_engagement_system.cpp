#include "systems/combat_engagement_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/combat_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

using CE = components::CombatEngagement;

CombatEngagementSystem::CombatEngagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CombatEngagementSystem::updateComponent(ecs::Entity& /*entity*/,
    components::CombatEngagement& ce, float delta_time) {
    if (!ce.active) return;

    ce.time_in_state += delta_time;

    switch (ce.state) {
        case CE::State::Safe:
            ce.warp_blocked = false;
            ce.dock_blocked = false;
            break;

        case CE::State::Engaging:
            ce.warp_blocked = true;
            ce.dock_blocked = true;
            if (!ce.attackers.empty()) {
                ce.state = CE::State::InCombat;
                ce.time_in_state = 0.0f;
                ce.engagement_count++;
            }
            break;

        case CE::State::InCombat:
            ce.warp_blocked = true;
            ce.dock_blocked = true;
            ce.total_combat_time += delta_time;
            if (ce.attackers.empty()) {
                ce.state = CE::State::Disengaging;
                ce.time_in_state = 0.0f;
                ce.disengage_timer = ce.disengage_duration;
            }
            break;

        case CE::State::Disengaging:
            ce.warp_blocked = true;
            ce.dock_blocked = true;
            ce.disengage_timer -= delta_time;
            if (!ce.attackers.empty()) {
                ce.state = CE::State::InCombat;
                ce.time_in_state = 0.0f;
            } else if (ce.disengage_timer <= 0.0f) {
                ce.state = CE::State::Safe;
                ce.time_in_state = 0.0f;
                ce.disengage_timer = 0.0f;
                ce.warp_blocked = false;
                ce.dock_blocked = false;
            }
            break;
    }
}

bool CombatEngagementSystem::initializeEngagement(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CombatEngagement>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CombatEngagementSystem::addAttacker(const std::string& entity_id,
    const std::string& attacker_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    if (!ce) return false;

    // Check for duplicate
    for (const auto& a : ce->attackers) {
        if (a == attacker_id) return false;
    }
    ce->attackers.push_back(attacker_id);

    if (ce->state == CE::State::Safe) {
        ce->state = CE::State::Engaging;
        ce->time_in_state = 0.0f;
    }
    return true;
}

bool CombatEngagementSystem::removeAttacker(const std::string& entity_id,
    const std::string& attacker_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    if (!ce) return false;

    auto it = std::remove(ce->attackers.begin(), ce->attackers.end(), attacker_id);
    if (it == ce->attackers.end()) return false;
    ce->attackers.erase(it, ce->attackers.end());
    return true;
}

bool CombatEngagementSystem::setTarget(const std::string& entity_id,
    const std::string& target_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    if (!ce) return false;
    ce->primary_target_id = target_id;
    if (ce->state == CE::State::Safe && !target_id.empty()) {
        ce->state = CE::State::Engaging;
        ce->time_in_state = 0.0f;
    }
    return true;
}

int CombatEngagementSystem::getAttackerCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    return ce ? static_cast<int>(ce->attackers.size()) : 0;
}

int CombatEngagementSystem::getState(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    return ce ? static_cast<int>(ce->state) : 0;
}

bool CombatEngagementSystem::isInCombat(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    if (!ce) return false;
    return ce->state == CE::State::InCombat || ce->state == CE::State::Engaging;
}

bool CombatEngagementSystem::isWarpBlocked(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    return ce ? ce->warp_blocked : false;
}

bool CombatEngagementSystem::isDockBlocked(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    return ce ? ce->dock_blocked : false;
}

float CombatEngagementSystem::getTimeInState(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    return ce ? ce->time_in_state : 0.0f;
}

int CombatEngagementSystem::getEngagementCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    return ce ? ce->engagement_count : 0;
}

float CombatEngagementSystem::getTotalCombatTime(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ce = entity->getComponent<components::CombatEngagement>();
    return ce ? ce->total_combat_time : 0.0f;
}

} // namespace systems
} // namespace atlas
