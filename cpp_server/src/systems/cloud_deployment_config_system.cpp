#include "systems/cloud_deployment_config_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

CloudDeploymentConfigSystem::CloudDeploymentConfigSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CloudDeploymentConfigSystem::updateComponent(ecs::Entity& /*entity*/,
    components::CloudDeploymentConfig& cfg, float delta_time) {
    if (!cfg.active) return;

    if (cfg.deployed) {
        cfg.uptime += delta_time;

        // Health check countdown
        if (cfg.health_check_enabled && cfg.health_check_interval > 0.0f) {
            float interval_count = std::floor(cfg.uptime / cfg.health_check_interval);
            cfg.health_check_count = static_cast<int>(interval_count);
        }
    }
}

bool CloudDeploymentConfigSystem::createConfig(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CloudDeploymentConfig>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CloudDeploymentConfigSystem::setProvider(const std::string& entity_id, int provider) {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return false;
    if (provider < 0 || provider > 2) return false;
    cfg->provider = provider;

    // Update cost estimates based on provider
    float base = 50.0f;
    if (provider == components::CloudDeploymentConfig::AWS) base = 50.0f;
    else if (provider == components::CloudDeploymentConfig::GCP) base = 45.0f;
    else if (provider == components::CloudDeploymentConfig::Azure) base = 55.0f;
    cfg->estimated_monthly_cost = base + (cfg->max_players * 2.0f);
    return true;
}

bool CloudDeploymentConfigSystem::setRegion(const std::string& entity_id,
                                             const std::string& region) {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return false;
    if (region.empty()) return false;
    cfg->region = region;
    return true;
}

bool CloudDeploymentConfigSystem::setInstanceType(const std::string& entity_id,
                                                   const std::string& instance_type) {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return false;
    if (instance_type.empty()) return false;
    cfg->instance_type = instance_type;
    return true;
}

bool CloudDeploymentConfigSystem::setMaxPlayers(const std::string& entity_id, int max_players) {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return false;
    cfg->max_players = std::max(1, std::min(max_players, 100));
    // Recalculate cost
    float base = 50.0f;
    if (cfg->provider == components::CloudDeploymentConfig::GCP) base = 45.0f;
    else if (cfg->provider == components::CloudDeploymentConfig::Azure) base = 55.0f;
    cfg->estimated_monthly_cost = base + (cfg->max_players * 2.0f);
    return true;
}

bool CloudDeploymentConfigSystem::enableHealthCheck(const std::string& entity_id,
                                                     float interval_seconds) {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return false;
    cfg->health_check_enabled = true;
    cfg->health_check_interval = std::max(5.0f, interval_seconds);
    return true;
}

bool CloudDeploymentConfigSystem::validate(const std::string& entity_id) const {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return false;

    if (cfg->region.empty()) return false;
    if (cfg->instance_type.empty()) return false;
    if (cfg->max_players < 1) return false;
    return true;
}

bool CloudDeploymentConfigSystem::deploy(const std::string& entity_id) {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return false;

    if (!validate(entity_id)) return false;
    cfg->deployed = true;
    cfg->uptime = 0.0f;
    cfg->health_check_count = 0;
    return true;
}

int CloudDeploymentConfigSystem::getProvider(const std::string& entity_id) const {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return -1;
    return cfg->provider;
}

std::string CloudDeploymentConfigSystem::getRegion(const std::string& entity_id) const {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return "";
    return cfg->region;
}

int CloudDeploymentConfigSystem::getMaxPlayers(const std::string& entity_id) const {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return 0;
    return cfg->max_players;
}

float CloudDeploymentConfigSystem::getUptime(const std::string& entity_id) const {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return 0.0f;
    return cfg->uptime;
}

int CloudDeploymentConfigSystem::getHealthCheckCount(const std::string& entity_id) const {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return 0;
    return cfg->health_check_count;
}

bool CloudDeploymentConfigSystem::isDeployed(const std::string& entity_id) const {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return false;
    return cfg->deployed;
}

float CloudDeploymentConfigSystem::getEstimatedMonthlyCost(const std::string& entity_id) const {
    auto* cfg = getComponentFor(entity_id);
    if (!cfg) return 0.0f;
    return cfg->estimated_monthly_cost;
}

} // namespace systems
} // namespace atlas
