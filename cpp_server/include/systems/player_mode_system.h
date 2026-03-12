#ifndef NOVAFORGE_SYSTEMS_PLAYER_MODE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_MODE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages player view/control mode transitions (FPS, Cockpit, Turret, etc.)
 *
 * Players can switch between different interaction modes when entering or
 * leaving a ship, turret, drone, or fleet command view.  Transitions are
 * timed and progress is ticked each frame.
 */
class PlayerModeSystem
    : public ecs::SingleComponentSystem<components::PlayerModeState> {
public:
    explicit PlayerModeSystem(ecs::World* world);
    ~PlayerModeSystem() override = default;

    std::string getName() const override { return "PlayerModeSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Mode transitions ---
    bool switch_mode(const std::string& entity_id,
                     components::PlayerModeState::PlayerMode new_mode);
    bool complete_transition(const std::string& entity_id);

    // --- Binding ---
    bool bind_entity(const std::string& entity_id,
                     const std::string& target_id);

    // --- Queries ---
    int         get_current_mode(const std::string& entity_id) const;
    int         get_previous_mode(const std::string& entity_id) const;
    std::string get_bound_entity(const std::string& entity_id) const;
    bool        is_in_transition(const std::string& entity_id) const;
    int         get_total_switches(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PlayerModeState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_MODE_SYSTEM_H
