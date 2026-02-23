#ifndef EVE_SYSTEMS_STATION_DEPLOYMENT_SYSTEM_H
#define EVE_SYSTEMS_STATION_DEPLOYMENT_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages ship-to-station deployment and station module attachment
 *
 * Phase 11: Fleet-as-Civilization
 *
 * Ships with StationDeployment component can deploy into permanent
 * stations. Once deployed, modules can be attached to upgrade the
 * station and improve the solar system's stats.
 */
class StationDeploymentSystem : public ecs::System {
public:
    explicit StationDeploymentSystem(ecs::World* world);
    ~StationDeploymentSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "StationDeploymentSystem"; }

    /**
     * @brief Begin deploying a ship into a station
     * @return true if deployment started successfully
     */
    bool beginDeployment(const std::string& entity_id,
                         const std::string& system_id,
                         float x, float y, float z);

    /**
     * @brief Cancel an in-progress deployment
     */
    void cancelDeployment(const std::string& entity_id);

    /**
     * @brief Check if a ship has completed deployment
     */
    bool isDeployed(const std::string& entity_id) const;

    /**
     * @brief Check if a ship is currently deploying
     */
    bool isDeploying(const std::string& entity_id) const;

    /**
     * @brief Attach a module to a deployed station
     * @return true if module was attached successfully
     */
    bool attachModule(const std::string& entity_id, const std::string& module_type);

    /**
     * @brief Get the total number of attached modules
     */
    int getAttachedModuleCount(const std::string& entity_id) const;

    /**
     * @brief Get the system bonuses provided by a deployed station
     */
    void getSystemBonuses(const std::string& entity_id,
                          float& security, float& economy, float& resource) const;
};

} // namespace systems
} // namespace atlas

#endif // EVE_SYSTEMS_STATION_DEPLOYMENT_SYSTEM_H
