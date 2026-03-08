#include "systems/dynamic_difficulty_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

DynamicDifficultySystem::DynamicDifficultySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void DynamicDifficultySystem::updateComponent(ecs::Entity& /*entity*/,
    components::DynamicDifficultyState& state, float delta_time) {
    if (!state.active) return;

    state.time_since_last_encounter += delta_time;

    // Recompute difficulty multiplier each tick
    state.encounter_multiplier = calculateMultiplier(
        state.player_combat_rating,
        state.ship_power_level,
        state.consecutive_wins,
        state.consecutive_losses,
        state.base_difficulty,
        state.adjustment_rate,
        state.min_difficulty,
        state.max_difficulty
    );
}

bool DynamicDifficultySystem::recordWin(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    state->encounters_won++;
    state->consecutive_wins++;
    state->consecutive_losses = 0;
    state->time_since_last_encounter = 0.0f;

    // Recalculate
    state->encounter_multiplier = calculateMultiplier(
        state->player_combat_rating,
        state->ship_power_level,
        state->consecutive_wins,
        state->consecutive_losses,
        state->base_difficulty,
        state->adjustment_rate,
        state->min_difficulty,
        state->max_difficulty
    );
    return true;
}

bool DynamicDifficultySystem::recordLoss(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    state->encounters_lost++;
    state->consecutive_losses++;
    state->consecutive_wins = 0;
    state->time_since_last_encounter = 0.0f;

    // Recalculate
    state->encounter_multiplier = calculateMultiplier(
        state->player_combat_rating,
        state->ship_power_level,
        state->consecutive_wins,
        state->consecutive_losses,
        state->base_difficulty,
        state->adjustment_rate,
        state->min_difficulty,
        state->max_difficulty
    );
    return true;
}

float DynamicDifficultySystem::computeDifficulty(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return 1.0f;

    return calculateMultiplier(
        state->player_combat_rating,
        state->ship_power_level,
        state->consecutive_wins,
        state->consecutive_losses,
        state->base_difficulty,
        state->adjustment_rate,
        state->min_difficulty,
        state->max_difficulty
    );
}

float DynamicDifficultySystem::getEncounterMultiplier(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->encounter_multiplier : 1.0f;
}

int DynamicDifficultySystem::getConsecutiveWins(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->consecutive_wins : 0;
}

int DynamicDifficultySystem::getConsecutiveLosses(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->consecutive_losses : 0;
}

int DynamicDifficultySystem::getTotalEncounters(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    return state->encounters_won + state->encounters_lost;
}

float DynamicDifficultySystem::calculateMultiplier(float combat_rating, float ship_power,
                                                    int consecutive_wins, int consecutive_losses,
                                                    float base_difficulty, float adjustment_rate,
                                                    float min_diff, float max_diff) {
    // Base: combine combat rating and ship power (geometric mean)
    float player_strength = std::sqrt(std::max(combat_rating, 0.1f) * std::max(ship_power, 0.1f));

    // Win streak pushes difficulty up; loss streak pushes it down
    float streak_adjustment = (consecutive_wins - consecutive_losses) * adjustment_rate;

    float result = base_difficulty * player_strength + streak_adjustment;

    return std::max(min_diff, std::min(result, max_diff));
}

} // namespace systems
} // namespace atlas
