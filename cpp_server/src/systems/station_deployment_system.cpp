#include "systems/station_deployment_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

StationDeploymentSystem::StationDeploymentSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void StationDeploymentSystem::updateComponent(ecs::Entity& /*entity*/, components::StationDeployment& comp, float delta_time) {
    if (comp.deploy_state == components::StationDeployment::DeployState::Deploying) {
        comp.deploy_timer -= delta_time;
        if (comp.deploy_timer <= 0.0f) {
            comp.deploy_timer = 0.0f;
            comp.deploy_state = components::StationDeployment::DeployState::Deployed;
        }
    }
}

bool StationDeploymentSystem::beginDeployment(const std::string& entity_id,
                                               const std::string& system_id,
                                               float x, float y, float z) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* dep = entity->getComponent<components::StationDeployment>();
    if (!dep) {
        entity->addComponent(std::make_unique<components::StationDeployment>());
        dep = entity->getComponent<components::StationDeployment>();
    }

    // Can only deploy from Mobile state
    if (dep->deploy_state != components::StationDeployment::DeployState::Mobile)
        return false;

    dep->deploy_state = components::StationDeployment::DeployState::Deploying;
    dep->deploy_timer = dep->deploy_duration;
    dep->system_id = system_id;
    dep->deploy_x = x;
    dep->deploy_y = y;
    dep->deploy_z = z;

    return true;
}

void StationDeploymentSystem::cancelDeployment(const std::string& entity_id) {
    auto* dep = getComponentFor(entity_id);
    if (!dep) return;

    if (dep->deploy_state == components::StationDeployment::DeployState::Deploying) {
        dep->deploy_state = components::StationDeployment::DeployState::Mobile;
        dep->deploy_timer = 0.0f;
    }
}

bool StationDeploymentSystem::isDeployed(const std::string& entity_id) const {
    auto* dep = getComponentFor(entity_id);
    if (!dep) return false;

    return dep->deploy_state == components::StationDeployment::DeployState::Deployed;
}

bool StationDeploymentSystem::isDeploying(const std::string& entity_id) const {
    auto* dep = getComponentFor(entity_id);
    if (!dep) return false;

    return dep->deploy_state == components::StationDeployment::DeployState::Deploying;
}

bool StationDeploymentSystem::attachModule(const std::string& entity_id,
                                            const std::string& module_type) {
    auto* dep = getComponentFor(entity_id);
    if (!dep) return false;

    // Must be deployed to attach modules
    if (dep->deploy_state != components::StationDeployment::DeployState::Deployed)
        return false;

    if (!dep->canAttachModule())
        return false;

    dep->attached_modules[module_type]++;

    // Apply module bonuses
    if (module_type == "security") {
        dep->security_bonus += 0.05f;
    } else if (module_type == "market") {
        dep->economy_bonus += 0.10f;
    } else if (module_type == "refinery") {
        dep->resource_bonus += 0.08f;
    }

    return true;
}

int StationDeploymentSystem::getAttachedModuleCount(const std::string& entity_id) const {
    auto* dep = getComponentFor(entity_id);
    if (!dep) return 0;

    return dep->getTotalAttachedModules();
}

void StationDeploymentSystem::getSystemBonuses(const std::string& entity_id,
                                                float& security, float& economy,
                                                float& resource) const {
    auto* dep = getComponentFor(entity_id);
    if (!dep) {
        security = 0.0f; economy = 0.0f; resource = 0.0f;
        return;
    }

    security = dep->security_bonus;
    economy = dep->economy_bonus;
    resource = dep->resource_bonus;
}

} // namespace systems
} // namespace atlas
