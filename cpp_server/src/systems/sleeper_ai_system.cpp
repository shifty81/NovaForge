#include "systems/sleeper_ai_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SleeperAISystem::SleeperAISystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SleeperAISystem::updateComponent(ecs::Entity& entity,
    components::SleeperAIState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Escalation check
    if (comp.alert_level == components::SleeperAIState::AlertLevel::Combat) {
        comp.escalation_timer += delta_time;
        if (comp.damage_taken >= comp.damage_threshold &&
            comp.escalation_timer >= comp.escalation_cooldown &&
            comp.escalation_wave < comp.max_escalation_waves) {
            comp.escalation_wave++;
            comp.escalation_timer = 0.0f;
            comp.alert_level = components::SleeperAIState::AlertLevel::Escalated;
            // Spawn reinforcement: add 2 sentry units
            for (int i = 0; i < 2; i++) {
                if (static_cast<int>(comp.units.size()) >= comp.max_units) break;
                components::SleeperAIState::SleeperUnit u;
                u.unit_id = "reinforce_w" + std::to_string(comp.escalation_wave) +
                            "_" + std::to_string(i);
                u.role = components::SleeperAIState::SleeperRole::Sentry;
                comp.units.push_back(u);
            }
        }
    }

    // Coordinated target switching
    comp.target_switch_timer += delta_time;
    if (comp.target_switch_timer >= comp.target_switch_interval) {
        comp.target_switch_timer = 0.0f;
        // All alive units pick the first alive Guardian's target, or keep their own
        std::string shared_target;
        for (const auto& u : comp.units) {
            if (u.alive && !u.current_target.empty()) {
                shared_target = u.current_target;
                break;
            }
        }
        if (!shared_target.empty()) {
            for (auto& u : comp.units) {
                if (u.alive && u.role != components::SleeperAIState::SleeperRole::Guardian) {
                    u.current_target = shared_target;
                }
            }
        }
    }

    // Guardian remote repair: heal lowest HP unit
    for (const auto& u : comp.units) {
        if (u.alive && u.role == components::SleeperAIState::SleeperRole::Guardian
            && u.remote_rep_amount > 0.0f) {
            components::SleeperAIState::SleeperUnit* lowest = nullptr;
            for (auto& other : comp.units) {
                if (&other != &u && other.alive && other.hp < other.max_hp) {
                    if (!lowest || other.hp < lowest->hp) {
                        lowest = &other;
                    }
                }
            }
            if (lowest) {
                lowest->hp = (std::min)(lowest->hp + u.remote_rep_amount * delta_time,
                                        lowest->max_hp);
            }
        }
    }
}

bool SleeperAISystem::initialize(const std::string& entity_id,
    const std::string& site_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SleeperAIState>();
    comp->site_id = site_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool SleeperAISystem::addUnit(const std::string& entity_id,
    const std::string& unit_id, components::SleeperAIState::SleeperRole role,
    float hp, float dps, float remote_rep) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->units.size()) >= comp->max_units) return false;

    components::SleeperAIState::SleeperUnit u;
    u.unit_id = unit_id;
    u.role = role;
    u.hp = hp;
    u.max_hp = hp;
    u.dps = dps;
    u.remote_rep_amount = remote_rep;
    comp->units.push_back(u);

    if (comp->alert_level == components::SleeperAIState::AlertLevel::Dormant) {
        comp->alert_level = components::SleeperAIState::AlertLevel::Alerted;
    }
    return true;
}

bool SleeperAISystem::removeUnit(const std::string& entity_id,
    const std::string& unit_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->units.begin(), comp->units.end(),
        [&](const components::SleeperAIState::SleeperUnit& u) {
            return u.unit_id == unit_id;
        });
    if (it == comp->units.end()) return false;
    comp->units.erase(it);
    return true;
}

bool SleeperAISystem::applyDamage(const std::string& entity_id,
    const std::string& unit_id, float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& u : comp->units) {
        if (u.unit_id == unit_id && u.alive) {
            u.hp -= amount;
            comp->damage_taken += amount;
            if (comp->alert_level == components::SleeperAIState::AlertLevel::Alerted) {
                comp->alert_level = components::SleeperAIState::AlertLevel::Combat;
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

bool SleeperAISystem::setAlertLevel(const std::string& entity_id,
    components::SleeperAIState::AlertLevel level) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->alert_level = level;
    return true;
}

int SleeperAISystem::getUnitCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->units.size()) : 0;
}

int SleeperAISystem::getAliveCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& u : comp->units) {
        if (u.alive) count++;
    }
    return count;
}

int SleeperAISystem::getEscalationWave(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->escalation_wave : 0;
}

float SleeperAISystem::getDamageTaken(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->damage_taken : 0.0f;
}

int SleeperAISystem::getTotalKills(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_kills : 0;
}

int SleeperAISystem::getTotalLosses(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_losses : 0;
}

components::SleeperAIState::AlertLevel SleeperAISystem::getAlertLevel(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->alert_level : components::SleeperAIState::AlertLevel::Dormant;
}

} // namespace systems
} // namespace atlas
