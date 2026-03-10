#include "systems/anomaly_spawning_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/exploration_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

AnomalySpawningSystem::AnomalySpawningSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AnomalySpawningSystem::updateComponent(ecs::Entity& /*entity*/,
    components::AnomalySpawningState& state, float delta_time) {
    if (!state.active) return;

    state.elapsed += delta_time;

    // Tick anomaly lifetimes
    for (auto& a : state.anomalies) {
        if (!a.completed) a.lifetime += delta_time;
    }

    // Despawn check: remove expired or completed anomalies
    state.despawn_timer += delta_time;
    if (state.despawn_timer >= state.despawn_check_interval) {
        state.despawn_timer -= state.despawn_check_interval;

        auto it = std::remove_if(state.anomalies.begin(), state.anomalies.end(),
            [&](const components::AnomalySpawningState::SpawnedAnomaly& a) {
                if (a.completed || a.lifetime >= a.max_lifetime) {
                    state.total_despawned++;
                    return true;
                }
                return false;
            });
        state.anomalies.erase(it, state.anomalies.end());
    }

    // Recalculate max anomalies based on security level
    // Lower security = more anomalies  (0.0 sec → base*3, 1.0 sec → base*1)
    float sec_mult = 1.0f + 2.0f * (1.0f - state.security_level);
    state.max_anomalies = static_cast<int>(std::round(state.base_max_anomalies * sec_mult));
    if (state.max_anomalies < 1) state.max_anomalies = 1;

    // Spawn timer (actual spawning is driven externally via spawnAnomaly())
    state.spawn_timer += delta_time;
    if (state.spawn_timer >= state.spawn_interval) {
        state.spawn_timer -= state.spawn_interval;
    }
}

bool AnomalySpawningSystem::initialize(const std::string& entity_id,
    const std::string& system_id, float security_level) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AnomalySpawningState>();
    comp->system_id = system_id.empty() ? entity_id : system_id;
    comp->security_level = (std::max)(0.0f, (std::min)(1.0f, security_level));
    entity->addComponent(std::move(comp));
    return true;
}

bool AnomalySpawningSystem::spawnAnomaly(const std::string& entity_id,
    const std::string& anomaly_id, components::AnomalySpawningState::AnomalyType type,
    int difficulty) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->active) return false;
    if (static_cast<int>(state->anomalies.size()) >= state->max_anomalies) return false;

    // No duplicate IDs
    for (const auto& a : state->anomalies) {
        if (a.anomaly_id == anomaly_id) return false;
    }

    components::AnomalySpawningState::SpawnedAnomaly anom;
    anom.anomaly_id = anomaly_id;
    anom.type = type;
    anom.difficulty = (std::max)(1, (std::min)(5, difficulty));
    state->anomalies.push_back(anom);
    state->total_spawned++;
    return true;
}

bool AnomalySpawningSystem::completeAnomaly(const std::string& entity_id,
    const std::string& anomaly_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& a : state->anomalies) {
        if (a.anomaly_id == anomaly_id && !a.completed) {
            a.completed = true;
            state->total_completed++;
            return true;
        }
    }
    return false;
}

bool AnomalySpawningSystem::removeAnomaly(const std::string& entity_id,
    const std::string& anomaly_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->anomalies.begin(), state->anomalies.end(),
        [&](const components::AnomalySpawningState::SpawnedAnomaly& a) {
            return a.anomaly_id == anomaly_id;
        });
    if (it == state->anomalies.end()) return false;
    state->anomalies.erase(it);
    return true;
}

int AnomalySpawningSystem::getAnomalyCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->anomalies.size()) : 0;
}

int AnomalySpawningSystem::getTotalSpawned(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_spawned : 0;
}

int AnomalySpawningSystem::getTotalCompleted(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_completed : 0;
}

int AnomalySpawningSystem::getTotalDespawned(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_despawned : 0;
}

int AnomalySpawningSystem::getMaxAnomalies(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->max_anomalies : 0;
}

bool AnomalySpawningSystem::setActive(const std::string& entity_id, bool active) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->active = active;
    return true;
}

bool AnomalySpawningSystem::isActive(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->active : false;
}

} // namespace systems
} // namespace atlas
