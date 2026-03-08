#include "systems/star_system_population_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

StarSystemPopulationSystem::StarSystemPopulationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void StarSystemPopulationSystem::updateComponent(ecs::Entity& /*entity*/,
    components::StarSystemPopulationState& state, float delta_time) {
    if (!state.active) return;

    state.time_since_spawn += delta_time;

    // Spawn check on interval
    float effective_interval = state.spawn_interval / std::max(state.activity_level, 0.01f);
    if (state.time_since_spawn >= effective_interval && state.current_npcs < state.max_npcs) {
        // Deterministic role selection based on ratios:
        // 30% miners, 20% haulers, 20% traders, 15% pirates, 15% security
        int slot = state.total_spawned % 20;
        if (slot < 6) {
            state.miners_active++;
        } else if (slot < 10) {
            state.haulers_active++;
        } else if (slot < 14) {
            state.traders_active++;
        } else if (slot < 17) {
            state.pirates_active++;
        } else {
            state.security_active++;
        }
        state.current_npcs++;
        state.total_spawned++;
        state.time_since_spawn = 0.0f;
    }
}

bool StarSystemPopulationSystem::spawnNPC(const std::string& entity_id, const std::string& role) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->current_npcs >= state->max_npcs) return false;

    if (role == "miner") state->miners_active++;
    else if (role == "hauler") state->haulers_active++;
    else if (role == "trader") state->traders_active++;
    else if (role == "pirate") state->pirates_active++;
    else if (role == "security") state->security_active++;
    else return false;

    state->current_npcs++;
    state->total_spawned++;
    return true;
}

bool StarSystemPopulationSystem::despawnNPC(const std::string& entity_id, const std::string& role) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (role == "miner" && state->miners_active > 0) state->miners_active--;
    else if (role == "hauler" && state->haulers_active > 0) state->haulers_active--;
    else if (role == "trader" && state->traders_active > 0) state->traders_active--;
    else if (role == "pirate" && state->pirates_active > 0) state->pirates_active--;
    else if (role == "security" && state->security_active > 0) state->security_active--;
    else return false;

    state->current_npcs--;
    state->total_despawned++;
    return true;
}

int StarSystemPopulationSystem::getCurrentPopulation(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->current_npcs : 0;
}

int StarSystemPopulationSystem::getRoleCount(const std::string& entity_id, const std::string& role) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return 0;

    if (role == "miner") return state->miners_active;
    if (role == "hauler") return state->haulers_active;
    if (role == "trader") return state->traders_active;
    if (role == "pirate") return state->pirates_active;
    if (role == "security") return state->security_active;
    return 0;
}

float StarSystemPopulationSystem::getActivityLevel(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->activity_level : 0.0f;
}

bool StarSystemPopulationSystem::isAtCapacity(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;
    return state->current_npcs >= state->max_npcs;
}

int StarSystemPopulationSystem::getTotalSpawned(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_spawned : 0;
}

int StarSystemPopulationSystem::getTotalDespawned(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_despawned : 0;
}

} // namespace systems
} // namespace atlas
