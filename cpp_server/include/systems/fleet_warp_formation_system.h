#ifndef NOVAFORGE_SYSTEMS_FLEET_WARP_FORMATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_WARP_FORMATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages warp-specific fleet formations with breathing and visual effects
 *
 * Phase 11: Fleet-as-Civilization
 *
 * During fleet warp, members warp visibly alongside the player (no teleport
 * pop-ins). Formation type depends on ship class:
 *   Fighters/Frigates  → TightEchelon
 *   Cruisers/BC        → LooseDiamond
 *   Capitals            → WideCapital
 *
 * A breathing oscillation (0.02–0.05 Hz) adds organic feel. Warp distortion
 * bends around larger ships, and smaller ships' wakes ripple.
 */
class FleetWarpFormationSystem : public ecs::SingleComponentSystem<components::FleetWarpState> {
public:
    explicit FleetWarpFormationSystem(ecs::World* world);
    ~FleetWarpFormationSystem() override = default;

    std::string getName() const override { return "FleetWarpFormationSystem"; }

    /**
     * @brief Begin a fleet warp for an entity (sets in_fleet_warp = true)
     */
    void beginFleetWarp(const std::string& entity_id);

    /**
     * @brief End fleet warp for an entity
     */
    void endFleetWarp(const std::string& entity_id);

    /**
     * @brief Check if an entity is in fleet warp
     */
    bool isInFleetWarp(const std::string& entity_id) const;

    /**
     * @brief Select the warp formation type based on ship class
     */
    void selectFormationByShipClass(const std::string& entity_id, const std::string& ship_class);

    /**
     * @brief Get the current breathing offset for an entity
     */
    float getBreathingOffset(const std::string& entity_id) const;

    /**
     * @brief Get the distortion bend factor for an entity
     */
    float getDistortionBend(const std::string& entity_id) const;

    /**
     * @brief Compute warp formation offsets for a ship at a given slot
     * @param slot_index Position in the formation (0 = leader)
     * @param[out] ox, oy, oz Offset in metres
     */
    void computeWarpOffset(const std::string& entity_id, int slot_index,
                           float& ox, float& oy, float& oz) const;

    /// Default spacing between warp formation slots (metres)
    static constexpr float kWarpSpacing = 800.0f;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetWarpState& ws, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_WARP_FORMATION_SYSTEM_H
