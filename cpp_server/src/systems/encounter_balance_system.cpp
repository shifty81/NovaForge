#include "systems/encounter_balance_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

EncounterBalanceSystem::EncounterBalanceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void EncounterBalanceSystem::updateComponent(ecs::Entity& /*entity*/,
    components::EncounterBalanceState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed += delta_time;
}

bool EncounterBalanceSystem::initialize(const std::string& entity_id,
    const std::string& encounter_id, float base_difficulty, float base_reward) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (encounter_id.empty()) return false;
    if (base_difficulty <= 0.0f || base_reward <= 0.0f) return false;
    auto comp = std::make_unique<components::EncounterBalanceState>();
    comp->encounter_id = encounter_id;
    comp->base_difficulty = base_difficulty;
    comp->base_reward = base_reward;
    comp->difficulty_multiplier = base_difficulty;
    comp->reward_multiplier = 1.0f;
    entity->addComponent(std::move(comp));
    return true;
}

bool EncounterBalanceSystem::setPlayerCount(const std::string& entity_id, int count) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->player_count = std::max(1, std::min(20, count));
    return true;
}

bool EncounterBalanceSystem::setAvgShipClass(const std::string& entity_id, int ship_class) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->avg_ship_class = std::max(1, std::min(5, ship_class));
    return true;
}

bool EncounterBalanceSystem::addKills(const std::string& entity_id, int kills) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (kills <= 0) return false;
    state->total_kills += kills;
    return true;
}

bool EncounterBalanceSystem::recalculate(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // difficulty = base * (1 + 0.25*(players-1)) * (1 + 0.1*ship_class)
    float fleet_factor = 1.0f + 0.25f * static_cast<float>(state->player_count - 1);
    float class_factor = 1.0f + 0.1f * static_cast<float>(state->avg_ship_class);
    state->difficulty_multiplier = state->base_difficulty * fleet_factor * class_factor;
    state->difficulty_multiplier = std::max(0.5f, std::min(3.0f, state->difficulty_multiplier));
    // reward = base * difficulty * (1 + 0.01*kills)
    float kill_factor = 1.0f + 0.01f * static_cast<float>(state->total_kills);
    state->reward_multiplier = state->difficulty_multiplier * kill_factor;
    state->recalc_count++;
    return true;
}

float EncounterBalanceSystem::getDifficultyMultiplier(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->difficulty_multiplier : 1.0f;
}

float EncounterBalanceSystem::getRewardMultiplier(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->reward_multiplier : 1.0f;
}

float EncounterBalanceSystem::getEffectiveReward(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->base_reward * state->reward_multiplier : 0.0f;
}

int EncounterBalanceSystem::getRecalcCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->recalc_count : 0;
}

int EncounterBalanceSystem::getPlayerCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->player_count : 0;
}

int EncounterBalanceSystem::getAvgShipClass(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->avg_ship_class : 0;
}

int EncounterBalanceSystem::getTotalKills(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_kills : 0;
}

} // namespace systems
} // namespace atlas
