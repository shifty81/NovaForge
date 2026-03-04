#ifndef NOVAFORGE_SYSTEMS_DRONE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DRONE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <cstdint>

namespace atlas {
namespace systems {

/**
 * @brief Manages drone deployment, recall, and autonomous combat
 *
 * Handles launching drones from a ship's drone bay, recalling them,
 * and processing their combat actions each tick.  Enforces bandwidth
 * limits and removes destroyed drones.
 */
class DroneSystem : public ecs::SingleComponentSystem<components::DroneBay> {
public:
    explicit DroneSystem(ecs::World* world);
    ~DroneSystem() override = default;

    std::string getName() const override { return "DroneSystem"; }

    /**
     * @brief Launch a drone from the bay into space
     * @param entity_id  Owner entity (ship)
     * @param drone_id   ID of the drone type to launch
     * @return true if the drone was deployed successfully
     */
    bool launchDrone(const std::string& entity_id,
                     const std::string& drone_id);

    /**
     * @brief Recall a single deployed drone back to the bay
     * @return true if the drone was recalled
     */
    bool recallDrone(const std::string& entity_id,
                     const std::string& drone_id);

    /**
     * @brief Recall all deployed drones back to the bay
     * @return Number of drones recalled
     */
    int recallAll(const std::string& entity_id);

    /**
     * @brief Get the number of currently deployed drones for an entity
     */
    int getDeployedCount(const std::string& entity_id);

    /**
     * @brief Set the mining target for mining drones
     * @param entity_id  Owner entity (ship)
     * @param deposit_id Entity id of the MineralDeposit
     */
    bool setMiningTarget(const std::string& entity_id,
                         const std::string& deposit_id);

    /**
     * @brief Set the salvage target for salvage drones
     * @param entity_id  Owner entity (ship)
     * @param wreck_id   Entity id of the Wreck
     */
    bool setSalvageTarget(const std::string& entity_id,
                          const std::string& wreck_id);

private:
    uint32_t salvage_seed_ = 42;
    float nextSalvageRandom();

protected:
    void updateComponent(ecs::Entity& entity, components::DroneBay& bay, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DRONE_SYSTEM_H
