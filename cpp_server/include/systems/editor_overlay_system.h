#ifndef NOVAFORGE_SYSTEMS_EDITOR_OVERLAY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_EDITOR_OVERLAY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * EditorOverlaySystem - manages editor overlay layout modes (Hidden / Minimal / Full)
 * toggled with F12. Controls panel visibility, input capture, and opacity.
 *
 * Reads/Writes EditorOverlayState component.
 */
class EditorOverlaySystem : public ecs::SingleComponentSystem<components::EditorOverlayState> {
public:
    explicit EditorOverlaySystem(ecs::World* world);
    ~EditorOverlaySystem() override = default;

    std::string getName() const override { return "EditorOverlaySystem"; }

    /// Cycle layout: Hidden -> Minimal -> Full -> Hidden. Increments toggle_count.
    bool cycleLayout(const std::string& entity_id);

    /// Set a specific layout mode.
    bool setLayoutMode(const std::string& entity_id, components::EditorOverlayState::LayoutMode mode);

    /// Get the current layout mode.
    components::EditorOverlayState::LayoutMode getLayoutMode(const std::string& entity_id) const;

    /// Toggle whether the editor captures input.
    bool toggleInputCapture(const std::string& entity_id);

    /// Toggle a named panel (hierarchy, inspector, console, profiler, tools).
    bool togglePanel(const std::string& entity_id, const std::string& panel_name);

    /// Check if a named panel is currently visible.
    bool isPanelVisible(const std::string& entity_id, const std::string& panel_name) const;

    /// Get current overlay opacity.
    float getOpacity(const std::string& entity_id) const;

    /// Set overlay opacity, clamped to [0.1, 1.0].
    bool setOpacity(const std::string& entity_id, float opacity);

protected:
    void updateComponent(ecs::Entity& entity, components::EditorOverlayState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_EDITOR_OVERLAY_SYSTEM_H
