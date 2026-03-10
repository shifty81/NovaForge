#include "systems/drifter_ai_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

DrifterAISystem::DrifterAISystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void DrifterAISystem::updateComponent(ecs::Entity& entity,
    components::DrifterAIState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Area denial field timer
    if (comp.area_denial_active) {
        comp.area_denial_timer += delta_time;
        if (comp.area_denial_timer >= comp.area_denial_duration) {
            comp.area_denial_active = false;
            comp.area_denial_timer = 0.0f;
        }
    }

    // Reinforcement spawning when Berserk
    if (comp.threat_level == components::DrifterAIState::ThreatLevel::Berserk) {
        comp.reinforcement_timer += delta_time;
        if (comp.reinforcement_timer >= comp.reinforcement_cooldown &&
            comp.reinforcement_wave < comp.max_reinforcement_waves) {
            comp.reinforcement_wave++;
            comp.reinforcement_timer = 0.0f;
            // Spawn 2 Response-class units
            for (int i = 0; i < 2; i++) {
                if (static_cast<int>(comp.units.size()) >= comp.max_units) break;
                components::DrifterAIState::DrifterUnit u;
                u.unit_id = "drifter_reinf_w" + std::to_string(comp.reinforcement_wave) +
                            "_" + std::to_string(i);
                u.role = components::DrifterAIState::DrifterRole::Response;
                u.hp = 1500.0f;
                u.max_hp = 1500.0f;
                u.base_dps = 250.0f;
                comp.units.push_back(u);
            }
        }
    }

    // Damage ramp for units firing at the same target
    for (auto& u : comp.units) {
        if (u.alive && !u.current_target.empty()) {
            u.ramp_multiplier = (std::min)(u.ramp_multiplier + u.ramp_rate * delta_time,
                                            u.max_ramp);
        } else {
            u.ramp_multiplier = 1.0f;
        }
    }
}

bool DrifterAISystem::initialize(const std::string& entity_id,
    const std::string& site_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DrifterAIState>();
    comp->site_id = site_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool DrifterAISystem::addUnit(const std::string& entity_id,
    const std::string& unit_id, components::DrifterAIState::DrifterRole role,
    float hp, float base_dps) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->units.size()) >= comp->max_units) return false;

    components::DrifterAIState::DrifterUnit u;
    u.unit_id = unit_id;
    u.role = role;
    u.hp = hp;
    u.max_hp = hp;
    u.base_dps = base_dps;
    comp->units.push_back(u);

    if (comp->threat_level == components::DrifterAIState::ThreatLevel::Passive) {
        comp->threat_level = components::DrifterAIState::ThreatLevel::Aggressive;
    }
    return true;
}

bool DrifterAISystem::removeUnit(const std::string& entity_id,
    const std::string& unit_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->units.begin(), comp->units.end(),
        [&](const components::DrifterAIState::DrifterUnit& u) {
            return u.unit_id == unit_id;
        });
    if (it == comp->units.end()) return false;
    comp->units.erase(it);
    return true;
}

bool DrifterAISystem::applyDamage(const std::string& entity_id,
    const std::string& unit_id, float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& u : comp->units) {
        if (u.unit_id == unit_id && u.alive) {
            u.hp -= amount;
            comp->damage_taken += amount;
            if (comp->damage_taken >= comp->provocation_threshold &&
                comp->threat_level != components::DrifterAIState::ThreatLevel::Berserk) {
                comp->threat_level = components::DrifterAIState::ThreatLevel::Berserk;
            }
            if (u.hp <= 0.0f) {
                u.hp = 0.0f;
                u.alive = false;
                comp->total_losses++;
            }
            return true;
        }
    }
    return false;
}

bool DrifterAISystem::setThreatLevel(const std::string& entity_id,
    components::DrifterAIState::ThreatLevel level) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->threat_level = level;
    return true;
}

bool DrifterAISystem::activateAreaDenial(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->area_denial_active = true;
    comp->area_denial_timer = 0.0f;
    return true;
}

int DrifterAISystem::getUnitCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->units.size()) : 0;
}

int DrifterAISystem::getAliveCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& u : comp->units) {
        if (u.alive) count++;
    }
    return count;
}

int DrifterAISystem::getReinforcementWave(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->reinforcement_wave : 0;
}

float DrifterAISystem::getDamageTaken(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->damage_taken : 0.0f;
}

int DrifterAISystem::getTotalKills(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_kills : 0;
}

int DrifterAISystem::getTotalLosses(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_losses : 0;
}

bool DrifterAISystem::isAreaDenialActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->area_denial_active : false;
}

components::DrifterAIState::ThreatLevel DrifterAISystem::getThreatLevel(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->threat_level : components::DrifterAIState::ThreatLevel::Passive;
}

} // namespace systems
} // namespace atlas
