#include "systems/aggression_switching_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/combat_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

AggressionSwitchingSystem::AggressionSwitchingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AggressionSwitchingSystem::updateComponent(ecs::Entity& /*entity*/,
    components::AggressionSwitchingState& state, float delta_time) {
    if (!state.active) return;

    state.elapsed += delta_time;

    // Decay all threat values
    for (auto& entry : state.threat_table) {
        entry.accumulated_threat = (std::max)(0.0f,
            entry.accumulated_threat - state.threat_decay_rate * delta_time);
        entry.time_on_list += delta_time;
    }

    // Periodic re-evaluation of primary target
    state.evaluation_timer += delta_time;
    if (state.evaluation_timer >= state.evaluation_interval && !state.locked) {
        state.evaluation_timer -= state.evaluation_interval;

        if (state.threat_table.empty()) {
            state.current_target_id.clear();
            return;
        }

        // Find highest threat
        auto best = std::max_element(state.threat_table.begin(), state.threat_table.end(),
            [](const components::AggressionSwitchingState::ThreatEntry& a,
               const components::AggressionSwitchingState::ThreatEntry& b) {
                return a.accumulated_threat < b.accumulated_threat;
            });

        // Apply hysteresis: only switch if new target exceeds threshold
        if (best->entity_id != state.current_target_id) {
            float current_threat = 0.0f;
            for (const auto& e : state.threat_table) {
                if (e.entity_id == state.current_target_id) {
                    current_threat = e.accumulated_threat;
                    break;
                }
            }
            if (current_threat <= 0.0f || best->accumulated_threat >= current_threat * state.switch_threshold) {
                state.current_target_id = best->entity_id;
                state.total_switches++;
            }
        }
    }
}

bool AggressionSwitchingSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AggressionSwitchingState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool AggressionSwitchingSystem::addThreatSource(const std::string& entity_id,
    const std::string& source_id, float dps, float ewar, float proximity) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->active) return false;

    // No duplicates
    for (const auto& e : state->threat_table) {
        if (e.entity_id == source_id) return false;
    }

    components::AggressionSwitchingState::ThreatEntry entry;
    entry.entity_id = source_id;
    entry.dps_contribution = (std::max)(0.0f, dps);
    entry.ewar_contribution = (std::max)(0.0f, ewar);
    entry.proximity_bonus = (std::max)(0.0f, proximity);
    entry.accumulated_threat = entry.dps_contribution + entry.ewar_contribution + entry.proximity_bonus;
    state->threat_table.push_back(entry);

    // If this is the first source, auto-assign as current target
    if (state->threat_table.size() == 1) {
        state->current_target_id = source_id;
    }
    return true;
}

bool AggressionSwitchingSystem::removeThreatSource(const std::string& entity_id,
    const std::string& source_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->threat_table.begin(), state->threat_table.end(),
        [&](const components::AggressionSwitchingState::ThreatEntry& e) {
            return e.entity_id == source_id;
        });
    if (it == state->threat_table.end()) return false;
    state->threat_table.erase(it);

    // If removed the current target, clear it
    if (state->current_target_id == source_id) {
        state->current_target_id.clear();
    }
    return true;
}

bool AggressionSwitchingSystem::applyDamage(const std::string& entity_id,
    const std::string& source_id, float damage) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->active) return false;
    for (auto& e : state->threat_table) {
        if (e.entity_id == source_id) {
            e.accumulated_threat += (std::max)(0.0f, damage);
            return true;
        }
    }
    return false;
}

int AggressionSwitchingSystem::getThreatSourceCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->threat_table.size()) : 0;
}

float AggressionSwitchingSystem::getThreatFor(const std::string& entity_id,
    const std::string& source_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    for (const auto& e : state->threat_table) {
        if (e.entity_id == source_id) return e.accumulated_threat;
    }
    return 0.0f;
}

std::string AggressionSwitchingSystem::getCurrentTarget(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->current_target_id : "";
}

int AggressionSwitchingSystem::getTotalSwitches(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_switches : 0;
}

bool AggressionSwitchingSystem::setLocked(const std::string& entity_id, bool locked) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->locked = locked;
    return true;
}

bool AggressionSwitchingSystem::isLocked(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->locked : false;
}

} // namespace systems
} // namespace atlas
