#include "systems/combat_timer_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CombatTimerSystem::CombatTimerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void CombatTimerSystem::updateComponent(ecs::Entity& /*entity*/,
                                        components::CombatTimer& comp,
                                        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    auto countdown = [](float& t, float dt) {
        if (t > 0.0f) { t -= dt; if (t < 0.0f) t = 0.0f; }
    };
    countdown(comp.aggression_timer, delta_time);
    countdown(comp.weapon_timer,     delta_time);
    countdown(comp.pod_kill_timer,   delta_time);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool CombatTimerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    entity->addComponent(std::make_unique<components::CombatTimer>());
    return true;
}

// ---------------------------------------------------------------------------
// Timer triggers
// ---------------------------------------------------------------------------

bool CombatTimerSystem::triggerAggression(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->aggression_timer = comp->aggression_duration;
    comp->total_aggressions++;
    return true;
}

bool CombatTimerSystem::triggerWeapon(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->weapon_timer = comp->weapon_duration;
    comp->total_weapon_activations++;
    return true;
}

bool CombatTimerSystem::triggerPodKill(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->pod_kill_timer = comp->pod_kill_duration;
    comp->total_pod_kills++;
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool CombatTimerSystem::setAggressionDuration(const std::string& entity_id,
                                               float seconds) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || seconds <= 0.0f) return false;
    comp->aggression_duration = seconds;
    return true;
}

bool CombatTimerSystem::setWeaponDuration(const std::string& entity_id,
                                           float seconds) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || seconds <= 0.0f) return false;
    comp->weapon_duration = seconds;
    return true;
}

bool CombatTimerSystem::setPodKillDuration(const std::string& entity_id,
                                            float seconds) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || seconds <= 0.0f) return false;
    comp->pod_kill_duration = seconds;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool CombatTimerSystem::isInCombat(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->aggression_timer > 0.0f
        || comp->weapon_timer     > 0.0f
        || comp->pod_kill_timer   > 0.0f;
}

bool CombatTimerSystem::canSafelyUndock(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return true;
    return comp->weapon_timer <= 0.0f;
}

bool CombatTimerSystem::hasAggression(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->aggression_timer > 0.0f : false;
}

bool CombatTimerSystem::hasWeaponTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->weapon_timer > 0.0f : false;
}

bool CombatTimerSystem::hasPodKillTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->pod_kill_timer > 0.0f : false;
}

float CombatTimerSystem::getAggressionTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->aggression_timer : 0.0f;
}

float CombatTimerSystem::getWeaponTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->weapon_timer : 0.0f;
}

float CombatTimerSystem::getPodKillTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->pod_kill_timer : 0.0f;
}

int CombatTimerSystem::getTotalAggressions(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_aggressions : 0;
}

int CombatTimerSystem::getTotalPodKills(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_pod_kills : 0;
}

} // namespace systems
} // namespace atlas
