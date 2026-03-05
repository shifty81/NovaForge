#include "systems/view_mode_transition_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <memory>
#include <algorithm>

namespace atlas {
namespace systems {

static constexpr float DEFAULT_TRANSITION_DURATION = 1.5f;

ViewModeTransitionSystem::ViewModeTransitionSystem(ecs::World* world)
    : StateMachineSystem(world) {
}

void ViewModeTransitionSystem::updateComponent(ecs::Entity& /*entity*/, components::ViewModeState& comp, float delta_time) {
    if (comp.cooldown_remaining > 0.0f) {
        comp.cooldown_remaining = std::max(0.0f, comp.cooldown_remaining - delta_time);
    }

    if (!comp.transitioning) return;

    if (comp.transition_duration <= 0.0f) {
        comp.transition_duration = DEFAULT_TRANSITION_DURATION;
    }

    comp.transition_progress += delta_time / comp.transition_duration;
    if (comp.transition_progress >= 1.0f) {
        comp.transition_progress = 1.0f;
        comp.transitioning = false;
        comp.previous_mode = comp.current_mode;
        comp.current_mode = comp.target_mode;
    }
}

bool ViewModeTransitionSystem::initializePlayer(const std::string& player_id) {
    std::string entity_id = std::string(VIEWMODE_ENTITY_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (entity) {
        // Already initialized
        auto* state = entity->getComponent<components::ViewModeState>();
        return state != nullptr;
    }

    entity = world_->createEntity(entity_id);
    auto comp = std::make_unique<components::ViewModeState>();
    comp->player_id = player_id;
    comp->current_mode = static_cast<int>(components::ViewModeState::Mode::Interior);
    comp->previous_mode = comp->current_mode;
    entity->addComponent(std::move(comp));
    return true;
}

bool ViewModeTransitionSystem::requestTransition(const std::string& player_id, int target_mode) {
    std::string entity_id = std::string(VIEWMODE_ENTITY_PREFIX) + player_id;
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    // Can't transition if already transitioning
    if (state->transitioning) return false;

    // Can't transition to current mode
    if (state->current_mode == target_mode) return false;

    // Validate transition path
    if (!isValidTransition(state->current_mode, target_mode)) return false;

    // Check cooldown
    if (state->cooldown_remaining > 0.0f) return false;

    state->target_mode = target_mode;
    state->transitioning = true;
    state->transition_progress = 0.0f;
    state->transition_duration = DEFAULT_TRANSITION_DURATION;
    return true;
}

int ViewModeTransitionSystem::getCurrentMode(const std::string& player_id) const {
    std::string entity_id = std::string(VIEWMODE_ENTITY_PREFIX) + player_id;
    const auto* state = getComponentFor(entity_id);
    return state ? state->current_mode : -1;
}

bool ViewModeTransitionSystem::isTransitioning(const std::string& player_id) const {
    std::string entity_id = std::string(VIEWMODE_ENTITY_PREFIX) + player_id;
    const auto* state = getComponentFor(entity_id);
    return state && state->transitioning;
}

float ViewModeTransitionSystem::getTransitionProgress(const std::string& player_id) const {
    std::string entity_id = std::string(VIEWMODE_ENTITY_PREFIX) + player_id;
    const auto* state = getComponentFor(entity_id);
    return state ? state->transition_progress : 0.0f;
}

bool ViewModeTransitionSystem::cancelTransition(const std::string& player_id) {
    std::string entity_id = std::string(VIEWMODE_ENTITY_PREFIX) + player_id;
    auto* state = getComponentFor(entity_id);
    if (!state || !state->transitioning) return false;

    state->transitioning = false;
    state->transition_progress = 0.0f;
    state->target_mode = state->current_mode;
    return true;
}

bool ViewModeTransitionSystem::isValidTransition(int from_mode, int to_mode) const {
    using Mode = components::ViewModeState::Mode;

    // Define valid transition adjacency:
    // Cockpit <-> Interior <-> EVA
    // Cockpit <-> RTS
    // Interior <-> RTS

    int cockpit = static_cast<int>(Mode::Cockpit);
    int interior = static_cast<int>(Mode::Interior);
    int eva = static_cast<int>(Mode::EVA);
    int rts = static_cast<int>(Mode::RTSOverlay);

    // Cockpit <-> Interior
    if ((from_mode == cockpit && to_mode == interior) ||
        (from_mode == interior && to_mode == cockpit)) return true;

    // Interior <-> EVA
    if ((from_mode == interior && to_mode == eva) ||
        (from_mode == eva && to_mode == interior)) return true;

    // Cockpit <-> RTS
    if ((from_mode == cockpit && to_mode == rts) ||
        (from_mode == rts && to_mode == cockpit)) return true;

    // Interior <-> RTS
    if ((from_mode == interior && to_mode == rts) ||
        (from_mode == rts && to_mode == interior)) return true;

    return false;
}

std::string ViewModeTransitionSystem::getModeName(int mode) {
    using Mode = components::ViewModeState::Mode;
    switch (static_cast<Mode>(mode)) {
        case Mode::Cockpit: return "Cockpit";
        case Mode::Interior: return "Interior";
        case Mode::EVA: return "EVA";
        case Mode::RTSOverlay: return "RTS Overlay";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
