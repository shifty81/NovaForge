#ifndef NOVAFORGE_SYSTEMS_CONTROL_MODE_CONTEXT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CONTROL_MODE_CONTEXT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * ControlModeContextSystem - context-aware control scheme switching.
 *
 * Reads/Writes ControlModeContextState component.
 *
 * Design:
 *   - Switches between SpaceUI, FPS, Cockpit, FleetCommand, StationMenu, BuildMode.
 *   - Each mode sets mouse_captured, sidebar_visible, crosshair_visible, orbit_camera_active.
 *   - Tracks previous mode for undo / back navigation.
 */
class ControlModeContextSystem : public ecs::SingleComponentSystem<components::ControlModeContextState> {
public:
    explicit ControlModeContextSystem(ecs::World* world);
    ~ControlModeContextSystem() override = default;

    std::string getName() const override { return "ControlModeContextSystem"; }

    /// Switch to a new control mode. Returns false if already in that mode.
    bool switchMode(const std::string& entity_id, components::ControlModeContextState::ControlMode mode);

    /// Query helpers
    components::ControlModeContextState::ControlMode getCurrentMode(const std::string& entity_id) const;
    bool isMouseCaptured(const std::string& entity_id) const;
    bool isSidebarVisible(const std::string& entity_id) const;
    bool isCrosshairVisible(const std::string& entity_id) const;
    int getModeSwitches(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ControlModeContextState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CONTROL_MODE_CONTEXT_SYSTEM_H
