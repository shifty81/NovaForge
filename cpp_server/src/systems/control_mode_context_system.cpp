#include "systems/control_mode_context_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

ControlModeContextSystem::ControlModeContextSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ControlModeContextSystem::updateComponent(ecs::Entity& /*entity*/,
    components::ControlModeContextState& state, float delta_time) {
    if (!state.active) return;
    state.time_in_current_mode += delta_time;
}

bool ControlModeContextSystem::switchMode(const std::string& entity_id,
    components::ControlModeContextState::ControlMode mode) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    // Same-mode switch is a no-op
    if (state->current_mode == mode) return false;

    state->previous_mode = state->current_mode;
    state->current_mode = mode;
    state->time_in_current_mode = 0.0f;
    state->mode_switches++;

    using CM = components::ControlModeContextState::ControlMode;
    switch (mode) {
        case CM::SpaceUI:
            state->mouse_captured = false;
            state->sidebar_visible = true;
            state->crosshair_visible = false;
            state->orbit_camera_active = false;
            break;
        case CM::FPS:
            state->mouse_captured = true;
            state->sidebar_visible = false;
            state->crosshair_visible = true;
            state->orbit_camera_active = false;
            break;
        case CM::Cockpit:
            state->mouse_captured = false;
            state->sidebar_visible = true;
            state->crosshair_visible = true;
            state->orbit_camera_active = false;
            break;
        case CM::FleetCommand:
            state->mouse_captured = false;
            state->sidebar_visible = true;
            state->crosshair_visible = false;
            state->orbit_camera_active = true;
            break;
        case CM::StationMenu:
            state->mouse_captured = false;
            state->sidebar_visible = true;
            state->crosshair_visible = false;
            state->orbit_camera_active = false;
            break;
        case CM::BuildMode:
            state->mouse_captured = false;
            state->sidebar_visible = false;
            state->crosshair_visible = true;
            state->orbit_camera_active = true;
            break;
    }

    return true;
}

components::ControlModeContextState::ControlMode
ControlModeContextSystem::getCurrentMode(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return components::ControlModeContextState::ControlMode::SpaceUI;
    return state->current_mode;
}

bool ControlModeContextSystem::isMouseCaptured(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->mouse_captured : false;
}

bool ControlModeContextSystem::isSidebarVisible(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->sidebar_visible : false;
}

bool ControlModeContextSystem::isCrosshairVisible(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->crosshair_visible : false;
}

int ControlModeContextSystem::getModeSwitches(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->mode_switches : 0;
}

} // namespace systems
} // namespace atlas
