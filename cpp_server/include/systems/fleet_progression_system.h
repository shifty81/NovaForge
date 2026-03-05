#ifndef NOVAFORGE_SYSTEMS_FLEET_PROGRESSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_PROGRESSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages fleet growth through 3 progression stages
 *
 * Phase 11: Fleet-as-Civilization
 *
 * Stage Early (max 5 ships)  — Player + 4 captains
 * Stage Mid   (max 15 ships) — 3 wings, role specialization
 * Stage End   (max 25 ships) — 5 wings, full doctrine
 */
class FleetProgressionSystem : public ecs::SingleComponentSystem<components::FleetProgression> {
public:
    explicit FleetProgressionSystem(ecs::World* world);
    ~FleetProgressionSystem() override = default;

    std::string getName() const override { return "FleetProgressionSystem"; }

    /**
     * @brief Get the current progression stage for a fleet entity
     */
    components::FleetProgression::Stage getStage(const std::string& entity_id) const;

    /**
     * @brief Add experience to a fleet, potentially advancing its stage
     */
    void addExperience(const std::string& entity_id, float xp);

    /**
     * @brief Get the current max ships allowed for a fleet
     */
    int getMaxShips(const std::string& entity_id) const;

    /**
     * @brief Get the current max wings allowed for a fleet
     */
    int getMaxWings(const std::string& entity_id) const;

    /**
     * @brief Check if a fleet can accept another ship
     */
    bool canAddShip(const std::string& entity_id) const;

    /**
     * @brief Register a ship joining the fleet
     */
    bool addShipToFleet(const std::string& entity_id);

    /**
     * @brief Register a ship leaving the fleet
     */
    void removeShipFromFleet(const std::string& entity_id);

    /**
     * @brief Unlock a wing specialization role
     */
    void unlockWingRole(const std::string& entity_id, const std::string& role);

    /**
     * @brief Check if a wing role is unlocked
     */
    bool isWingRoleUnlocked(const std::string& entity_id, const std::string& role) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FleetProgression& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_PROGRESSION_SYSTEM_H
