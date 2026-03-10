#include "systems/officer_spawn_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/npc_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

OfficerSpawnSystem::OfficerSpawnSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void OfficerSpawnSystem::updateComponent(ecs::Entity& /*entity*/,
    components::OfficerSpawnState& state, float delta_time) {
    if (!state.active) return;

    state.elapsed += delta_time;
    state.time_since_last_spawn += delta_time;

    // Tick alive timers
    for (auto& off : state.officers) {
        if (off.alive) off.time_alive += delta_time;
    }

    // Increase spawn chance over time
    state.current_spawn_chance = state.base_spawn_chance +
        state.spawn_chance_increase * (state.time_since_last_spawn / 60.0f);
    state.current_spawn_chance = (std::min)(1.0f, state.current_spawn_chance);

    // Evaluation tick (no actual random roll in deterministic simulation)
    state.evaluation_timer += delta_time;
    if (state.evaluation_timer >= state.evaluation_interval) {
        state.evaluation_timer -= state.evaluation_interval;
    }
}

bool OfficerSpawnSystem::initialize(const std::string& entity_id, const std::string& system_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::OfficerSpawnState>();
    comp->system_id = system_id.empty() ? entity_id : system_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool OfficerSpawnSystem::spawnOfficer(const std::string& entity_id,
    const std::string& npc_type, components::OfficerSpawnState::OfficerRank rank) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->active) return false;

    // Count alive officers
    int alive = 0;
    for (const auto& o : state->officers) {
        if (o.alive) alive++;
    }
    if (alive >= state->max_officers) return false;

    components::OfficerSpawnState::OfficerRecord rec;
    rec.officer_id = npc_type + "_" + std::to_string(state->total_spawned);
    rec.npc_type = npc_type;
    rec.rank = rank;
    // Set loot quality based on rank
    switch (rank) {
        case components::OfficerSpawnState::OfficerRank::Commander:
            rec.bounty_multiplier = 5.0f;
            rec.loot_quality = 1.0f;
            break;
        case components::OfficerSpawnState::OfficerRank::Captain:
            rec.bounty_multiplier = 10.0f;
            rec.loot_quality = 2.0f;
            break;
        case components::OfficerSpawnState::OfficerRank::Admiral:
            rec.bounty_multiplier = 20.0f;
            rec.loot_quality = 3.0f;
            break;
    }
    state->officers.push_back(rec);
    state->total_spawned++;
    state->time_since_last_spawn = 0.0f;
    state->current_spawn_chance = state->base_spawn_chance;
    return true;
}

bool OfficerSpawnSystem::killOfficer(const std::string& entity_id,
    const std::string& officer_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& o : state->officers) {
        if (o.officer_id == officer_id && o.alive) {
            o.alive = false;
            state->total_killed++;
            return true;
        }
    }
    return false;
}

int OfficerSpawnSystem::getAliveOfficerCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& o : state->officers) {
        if (o.alive) count++;
    }
    return count;
}

int OfficerSpawnSystem::getTotalSpawned(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_spawned : 0;
}

int OfficerSpawnSystem::getTotalKilled(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_killed : 0;
}

float OfficerSpawnSystem::getCurrentSpawnChance(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->current_spawn_chance : 0.0f;
}

float OfficerSpawnSystem::getTimeSinceLastSpawn(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->time_since_last_spawn : 0.0f;
}

bool OfficerSpawnSystem::setActive(const std::string& entity_id, bool active) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->active = active;
    return true;
}

bool OfficerSpawnSystem::isActive(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->active : false;
}

} // namespace systems
} // namespace atlas
