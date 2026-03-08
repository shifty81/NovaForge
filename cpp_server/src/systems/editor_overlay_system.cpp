#include "systems/editor_overlay_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

EditorOverlaySystem::EditorOverlaySystem(ecs::World* world) : SingleComponentSystem(world) {}

void EditorOverlaySystem::updateComponent(ecs::Entity& /*entity*/,
    components::EditorOverlayState& /*state*/, float /*delta_time*/) {
    // State-only system — UI reads this component directly.
}

bool EditorOverlaySystem::cycleLayout(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    using LM = components::EditorOverlayState::LayoutMode;
    switch (state->layout_mode) {
        case LM::Hidden:  state->layout_mode = LM::Minimal; break;
        case LM::Minimal: state->layout_mode = LM::Full;    break;
        case LM::Full:    state->layout_mode = LM::Hidden;  break;
    }
    state->toggle_count++;
    return true;
}

bool EditorOverlaySystem::setLayoutMode(const std::string& entity_id,
    components::EditorOverlayState::LayoutMode mode) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->layout_mode = mode;
    return true;
}

components::EditorOverlayState::LayoutMode
EditorOverlaySystem::getLayoutMode(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return components::EditorOverlayState::LayoutMode::Hidden;
    return state->layout_mode;
}

bool EditorOverlaySystem::toggleInputCapture(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->captures_input = !state->captures_input;
    return true;
}

bool EditorOverlaySystem::togglePanel(const std::string& entity_id,
    const std::string& panel_name) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (panel_name == "hierarchy")      state->show_hierarchy = !state->show_hierarchy;
    else if (panel_name == "inspector") state->show_inspector = !state->show_inspector;
    else if (panel_name == "console")   state->show_console   = !state->show_console;
    else if (panel_name == "profiler")  state->show_profiler  = !state->show_profiler;
    else if (panel_name == "tools")     state->show_tools     = !state->show_tools;
    else return false;

    return true;
}

bool EditorOverlaySystem::isPanelVisible(const std::string& entity_id,
    const std::string& panel_name) const {
    const auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (panel_name == "hierarchy") return state->show_hierarchy;
    if (panel_name == "inspector") return state->show_inspector;
    if (panel_name == "console")   return state->show_console;
    if (panel_name == "profiler")  return state->show_profiler;
    if (panel_name == "tools")     return state->show_tools;
    return false;
}

float EditorOverlaySystem::getOpacity(const std::string& entity_id) const {
    const auto* state = getComponentFor(entity_id);
    return state ? state->overlay_opacity : 0.0f;
}

bool EditorOverlaySystem::setOpacity(const std::string& entity_id, float opacity) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;

    if (opacity < 0.1f) opacity = 0.1f;
    if (opacity > 1.0f) opacity = 1.0f;
    state->overlay_opacity = opacity;
    return true;
}

} // namespace systems
} // namespace atlas
