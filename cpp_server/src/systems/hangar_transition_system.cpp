#include "systems/hangar_transition_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

HangarTransitionSystem::HangarTransitionSystem(ecs::World* world) : SingleComponentSystem(world) {}

void HangarTransitionSystem::updateComponent(ecs::Entity& /*entity*/,
    components::HangarTransitionState& state, float delta_time) {
    if (!state.active) return;

    using Phase = components::HangarTransitionState::TransitionPhase;

    if (state.phase == Phase::Idle || state.phase == Phase::DockComplete) {
        state.animation_playing = false;
        return;
    }

    state.animation_playing = true;
    state.phase_timer += delta_time;

    float duration = 0.0f;
    switch (state.phase) {
        case Phase::DockApproach:    duration = state.dock_approach_duration;   break;
        case Phase::DockSequence:    duration = state.dock_sequence_duration;   break;
        case Phase::UndockSequence:  duration = state.undock_sequence_duration; break;
        case Phase::UndockLaunch:    duration = state.undock_launch_duration;   break;
        default: break;
    }

    if (duration > 0.0f) {
        state.animation_progress = state.phase_timer / duration;
        if (state.animation_progress > 1.0f) state.animation_progress = 1.0f;
    }

    if (duration > 0.0f && state.phase_timer >= duration) {
        state.phase_timer = 0.0f;
        state.animation_progress = 0.0f;

        switch (state.phase) {
            case Phase::DockApproach:
                state.phase = Phase::DockSequence;
                break;
            case Phase::DockSequence:
                state.phase = Phase::DockComplete;
                state.animation_playing = false;
                state.total_docks++;
                break;
            case Phase::UndockSequence:
                state.phase = Phase::UndockLaunch;
                break;
            case Phase::UndockLaunch:
                state.phase = Phase::UndockComplete;
                state.animation_playing = false;
                state.total_undocks++;
                break;
            case Phase::UndockComplete:
                state.phase = Phase::Idle;
                state.animation_playing = false;
                break;
            default:
                break;
        }
    }

    // UndockComplete auto-advances to Idle immediately
    if (state.phase == Phase::UndockComplete) {
        state.phase = Phase::Idle;
        state.animation_playing = false;
        state.animation_progress = 0.0f;
    }
}

bool HangarTransitionSystem::beginDock(const std::string& entity_id, const std::string& station_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (state->phase != components::HangarTransitionState::TransitionPhase::Idle) return false;

    state->phase = components::HangarTransitionState::TransitionPhase::DockApproach;
    state->phase_timer = 0.0f;
    state->animation_progress = 0.0f;
    state->animation_playing = true;
    state->target_station_id = station_id;
    return true;
}

bool HangarTransitionSystem::beginUndock(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (state->phase != components::HangarTransitionState::TransitionPhase::DockComplete) return false;

    state->phase = components::HangarTransitionState::TransitionPhase::UndockSequence;
    state->phase_timer = 0.0f;
    state->animation_progress = 0.0f;
    state->animation_playing = true;
    return true;
}

components::HangarTransitionState::TransitionPhase
HangarTransitionSystem::getPhase(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return components::HangarTransitionState::TransitionPhase::Idle;
    return state->phase;
}

float HangarTransitionSystem::getAnimationProgress(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->animation_progress : 0.0f;
}

bool HangarTransitionSystem::isDocking(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;
    using Phase = components::HangarTransitionState::TransitionPhase;
    return state->phase == Phase::DockApproach || state->phase == Phase::DockSequence;
}

bool HangarTransitionSystem::isUndocking(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;
    using Phase = components::HangarTransitionState::TransitionPhase;
    return state->phase == Phase::UndockSequence || state->phase == Phase::UndockLaunch;
}

bool HangarTransitionSystem::isIdle(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return true;
    return state->phase == components::HangarTransitionState::TransitionPhase::Idle;
}

bool HangarTransitionSystem::isDocked(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;
    return state->phase == components::HangarTransitionState::TransitionPhase::DockComplete;
}

int HangarTransitionSystem::getTotalDocks(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_docks : 0;
}

int HangarTransitionSystem::getTotalUndocks(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->total_undocks : 0;
}

} // namespace systems
} // namespace atlas
