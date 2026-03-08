#include "systems/gameplay_loop_tracker_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

GameplayLoopTrackerSystem::GameplayLoopTrackerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void GameplayLoopTrackerSystem::updateComponent(ecs::Entity& /*entity*/,
    components::GameplayLoopTrackerState& state, float delta_time) {
    if (!state.active) return;

    state.time_in_current_phase += delta_time;

    // Accumulate per-phase time
    switch (state.current_phase) {
        case components::GameplayLoopTrackerState::LoopPhase::Flying:
        case components::GameplayLoopTrackerState::LoopPhase::Hauling:
            state.total_flight_time += delta_time;
            break;
        case components::GameplayLoopTrackerState::LoopPhase::Mining:
            state.total_mining_time += delta_time;
            break;
        case components::GameplayLoopTrackerState::LoopPhase::Combat:
            state.total_combat_time += delta_time;
            break;
        case components::GameplayLoopTrackerState::LoopPhase::Docked:
            state.total_docked_time += delta_time;
            break;
        default:
            break;
    }
}

bool GameplayLoopTrackerSystem::transitionTo(const std::string& entity_id,
    components::GameplayLoopTrackerState::LoopPhase phase) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (!isValidTransition(state->current_phase, phase)) return false;

    state->previous_phase = state->current_phase;
    state->current_phase = phase;
    state->time_in_current_phase = 0.0f;

    // Track milestones
    using Phase = components::GameplayLoopTrackerState::LoopPhase;
    switch (phase) {
        case Phase::Undocking:
            state->total_undocks++;
            state->has_undocked = true;
            break;
        case Phase::Mining:
            state->total_mining_sessions++;
            state->has_mined = true;
            break;
        case Phase::Trading:
            state->total_trades++;
            state->has_traded = true;
            break;
        case Phase::Combat:
            state->total_combat_encounters++;
            state->has_fought = true;
            break;
        case Phase::Docked:
            state->total_docks++;
            // Count a full loop if we've undocked before
            if (state->has_undocked) {
                state->loops_completed++;
            }
            break;
        default:
            break;
    }

    return true;
}

components::GameplayLoopTrackerState::LoopPhase
GameplayLoopTrackerSystem::getCurrentPhase(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return components::GameplayLoopTrackerState::LoopPhase::Docked;
    return state->current_phase;
}

int GameplayLoopTrackerSystem::getLoopsCompleted(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->loops_completed : 0;
}

float GameplayLoopTrackerSystem::getTimeInCurrentPhase(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->time_in_current_phase : 0.0f;
}

bool GameplayLoopTrackerSystem::hasCompletedAllActivities(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;
    return state->has_undocked && state->has_mined && state->has_traded && state->has_fought;
}

int GameplayLoopTrackerSystem::getTotalUndocks(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_undocks : 0;
}

bool GameplayLoopTrackerSystem::isValidTransition(
    components::GameplayLoopTrackerState::LoopPhase from,
    components::GameplayLoopTrackerState::LoopPhase to) {
    using Phase = components::GameplayLoopTrackerState::LoopPhase;

    // Same-phase transitions are invalid
    if (from == to) return false;

    switch (from) {
        case Phase::Docked:
            return to == Phase::Undocking;
        case Phase::Undocking:
            return to == Phase::Flying;
        case Phase::Flying:
            return to == Phase::Mining || to == Phase::Trading ||
                   to == Phase::Combat || to == Phase::Docking ||
                   to == Phase::Hauling;
        case Phase::Mining:
            return to == Phase::Flying || to == Phase::Hauling;
        case Phase::Hauling:
            return to == Phase::Flying || to == Phase::Trading ||
                   to == Phase::Docking;
        case Phase::Trading:
            return to == Phase::Flying || to == Phase::Docking;
        case Phase::Combat:
            return to == Phase::Flying || to == Phase::Docking;
        case Phase::Docking:
            return to == Phase::Docked;
    }
    return false;
}

} // namespace systems
} // namespace atlas
