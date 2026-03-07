#include "systems/encounter_spawner_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using ES = components::EncounterState;
}

EncounterSpawnerSystem::EncounterSpawnerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void EncounterSpawnerSystem::updateComponent(ecs::Entity& entity,
    components::EncounterState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;

    // Expire active encounters that exceeded their duration
    for (auto& enc : state.encounters) {
        if (enc.status == ES::Status::Active && enc.duration > 0.0f) {
            if (state.elapsed_time - enc.started_at >= enc.duration) {
                enc.status = ES::Status::Expired;
                enc.ended_at = state.elapsed_time;
                state.active_count--;
            }
        }
    }
}

bool EncounterSpawnerSystem::initialize(const std::string& entity_id,
    const std::string& system_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::EncounterState>();
    comp->system_id = system_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool EncounterSpawnerSystem::addEncounter(const std::string& entity_id,
    const std::string& encounter_id, const std::string& encounter_type,
    int difficulty, float duration) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (static_cast<int>(state->encounters.size()) >= state->max_encounters) return false;
    for (const auto& e : state->encounters) {
        if (e.encounter_id == encounter_id) return false;
    }
    ES::Encounter enc;
    enc.encounter_id = encounter_id;
    enc.encounter_type = encounter_type;
    enc.difficulty = difficulty;
    enc.duration = duration;
    state->encounters.push_back(enc);
    return true;
}

int EncounterSpawnerSystem::getEncounterCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->encounters.size()) : 0;
}

bool EncounterSpawnerSystem::hasEncounter(const std::string& entity_id,
    const std::string& encounter_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& e : state->encounters) {
        if (e.encounter_id == encounter_id) return true;
    }
    return false;
}

bool EncounterSpawnerSystem::activateEncounter(const std::string& entity_id,
    const std::string& encounter_id, float timestamp) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& e : state->encounters) {
        if (e.encounter_id == encounter_id) {
            if (e.status != ES::Status::Pending) return false;
            e.status = ES::Status::Active;
            e.started_at = timestamp;
            state->active_count++;
            return true;
        }
    }
    return false;
}

bool EncounterSpawnerSystem::completeEncounter(const std::string& entity_id,
    const std::string& encounter_id, float timestamp) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& e : state->encounters) {
        if (e.encounter_id == encounter_id) {
            if (e.status != ES::Status::Active) return false;
            e.status = ES::Status::Completed;
            e.ended_at = timestamp;
            state->active_count--;
            state->completed_count++;
            state->total_rewards_earned += e.isc_reward;
            return true;
        }
    }
    return false;
}

bool EncounterSpawnerSystem::failEncounter(const std::string& entity_id,
    const std::string& encounter_id, float timestamp) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& e : state->encounters) {
        if (e.encounter_id == encounter_id) {
            if (e.status != ES::Status::Active) return false;
            e.status = ES::Status::Failed;
            e.ended_at = timestamp;
            state->active_count--;
            state->failed_count++;
            return true;
        }
    }
    return false;
}

int EncounterSpawnerSystem::getEncounterStatus(const std::string& entity_id,
    const std::string& encounter_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return -1;
    for (const auto& e : state->encounters) {
        if (e.encounter_id == encounter_id) return static_cast<int>(e.status);
    }
    return -1;
}

int EncounterSpawnerSystem::getActiveEncounterCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->active_count : 0;
}

int EncounterSpawnerSystem::getCompletedEncounterCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->completed_count : 0;
}

int EncounterSpawnerSystem::getFailedEncounterCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->failed_count : 0;
}

bool EncounterSpawnerSystem::setEncounterReward(const std::string& entity_id,
    const std::string& encounter_id, double isc_reward, int loot_count) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& e : state->encounters) {
        if (e.encounter_id == encounter_id) {
            e.isc_reward = isc_reward;
            e.loot_count = loot_count;
            return true;
        }
    }
    return false;
}

double EncounterSpawnerSystem::getTotalRewardsEarned(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_rewards_earned : 0.0;
}

} // namespace systems
} // namespace atlas
