#ifndef NOVAFORGE_SYSTEMS_CLOUD_DEPLOYMENT_CONFIG_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CLOUD_DEPLOYMENT_CONFIG_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Cloud deployment configuration management
 *
 * Manages server deployment configurations for different cloud providers
 * (AWS, GCP, Azure) with region selection, instance sizing, and health checks.
 */
class CloudDeploymentConfigSystem : public ecs::SingleComponentSystem<components::CloudDeploymentConfig> {
public:
    explicit CloudDeploymentConfigSystem(ecs::World* world);
    ~CloudDeploymentConfigSystem() override = default;

    std::string getName() const override { return "CloudDeploymentConfigSystem"; }

    bool createConfig(const std::string& entity_id);
    bool setProvider(const std::string& entity_id, int provider);
    bool setRegion(const std::string& entity_id, const std::string& region);
    bool setInstanceType(const std::string& entity_id, const std::string& instance_type);
    bool setMaxPlayers(const std::string& entity_id, int max_players);
    bool enableHealthCheck(const std::string& entity_id, float interval_seconds);
    bool validate(const std::string& entity_id) const;
    bool deploy(const std::string& entity_id);
    int getProvider(const std::string& entity_id) const;
    std::string getRegion(const std::string& entity_id) const;
    int getMaxPlayers(const std::string& entity_id) const;
    float getUptime(const std::string& entity_id) const;
    int getHealthCheckCount(const std::string& entity_id) const;
    bool isDeployed(const std::string& entity_id) const;
    float getEstimatedMonthlyCost(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CloudDeploymentConfig& cfg, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CLOUD_DEPLOYMENT_CONFIG_SYSTEM_H
