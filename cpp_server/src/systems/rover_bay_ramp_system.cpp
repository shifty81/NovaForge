#include "systems/rover_bay_ramp_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

RoverBayRampSystem::RoverBayRampSystem(ecs::World* world)
    : System(world) {
}

void RoverBayRampSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::RoverBayRamp>();
    for (auto* entity : entities) {
        auto* bay = entity->getComponent<components::RoverBayRamp>();
        if (!bay) continue;

        if (!bay->is_powered) continue;

        if (bay->state == components::RoverBayRamp::RampState::Opening) {
            bay->ramp_progress += bay->ramp_speed * delta_time;
            if (bay->ramp_progress >= 1.0f) {
                bay->ramp_progress = 1.0f;
                bay->state = components::RoverBayRamp::RampState::Open;
                bay->is_pressurized = false;
            }
        } else if (bay->state == components::RoverBayRamp::RampState::Closing) {
            bay->ramp_progress -= bay->ramp_speed * delta_time;
            if (bay->ramp_progress <= 0.0f) {
                bay->ramp_progress = 0.0f;
                bay->state = components::RoverBayRamp::RampState::Closed;
                bay->is_pressurized = true;
            }
        }
    }
}

bool RoverBayRampSystem::initializeBay(const std::string& entity_id, int max_rovers) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::RoverBayRamp>();
    if (existing) return false;

    auto comp = std::make_unique<components::RoverBayRamp>();
    comp->max_rovers = std::max(1, max_rovers);
    entity->addComponent(std::move(comp));
    return true;
}

bool RoverBayRampSystem::openRamp(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return false;

    if (!bay->is_powered) return false;
    if (bay->state != components::RoverBayRamp::RampState::Closed) return false;

    // Safety interlock: prevent opening in corrosive atmosphere
    if (bay->external_atmosphere == components::RoverBayRamp::AtmosphereType::Corrosive) return false;

    bay->state = components::RoverBayRamp::RampState::Opening;
    return true;
}

bool RoverBayRampSystem::closeRamp(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return false;

    if (bay->state == components::RoverBayRamp::RampState::Open ||
        bay->state == components::RoverBayRamp::RampState::Opening) {
        bay->state = components::RoverBayRamp::RampState::Closing;
        return true;
    }

    return false;
}

bool RoverBayRampSystem::storeRover(const std::string& entity_id, const std::string& rover_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return false;

    if (bay->state != components::RoverBayRamp::RampState::Open) return false;
    if (static_cast<int>(bay->stored_rover_ids.size()) >= bay->max_rovers) return false;

    // Check not already stored
    for (const auto& id : bay->stored_rover_ids) {
        if (id == rover_id) return false;
    }

    bay->stored_rover_ids.push_back(rover_id);
    return true;
}

bool RoverBayRampSystem::deployRover(const std::string& entity_id, const std::string& rover_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return false;

    if (bay->state != components::RoverBayRamp::RampState::Open) return false;

    // Find rover in stored list
    auto it = std::find(bay->stored_rover_ids.begin(), bay->stored_rover_ids.end(), rover_id);
    if (it == bay->stored_rover_ids.end()) return false;

    bay->stored_rover_ids.erase(it);
    bay->deployed_rover_ids.push_back(rover_id);
    bay->total_deployments++;
    return true;
}

bool RoverBayRampSystem::retrieveRover(const std::string& entity_id, const std::string& rover_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return false;

    if (bay->state != components::RoverBayRamp::RampState::Open) return false;
    if (static_cast<int>(bay->stored_rover_ids.size()) >= bay->max_rovers) return false;

    auto it = std::find(bay->deployed_rover_ids.begin(), bay->deployed_rover_ids.end(), rover_id);
    if (it == bay->deployed_rover_ids.end()) return false;

    bay->deployed_rover_ids.erase(it);
    bay->stored_rover_ids.push_back(rover_id);
    return true;
}

bool RoverBayRampSystem::setExternalAtmosphere(const std::string& entity_id,
                                                components::RoverBayRamp::AtmosphereType atmosphere) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return false;

    bay->external_atmosphere = atmosphere;
    return true;
}

bool RoverBayRampSystem::setExternalGravity(const std::string& entity_id, float gravity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return false;

    bay->external_gravity = std::max(0.0f, std::min(2.0f, gravity));
    return true;
}

bool RoverBayRampSystem::setPowerEnabled(const std::string& entity_id, bool enabled) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return false;

    bay->is_powered = enabled;
    return true;
}

std::string RoverBayRampSystem::getRampState(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return "unknown";

    return components::RoverBayRamp::stateToString(bay->state);
}

float RoverBayRampSystem::getRampProgress(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return 0.0f;

    return bay->ramp_progress;
}

int RoverBayRampSystem::getStoredCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return 0;

    return static_cast<int>(bay->stored_rover_ids.size());
}

int RoverBayRampSystem::getDeployedCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* bay = entity->getComponent<components::RoverBayRamp>();
    if (!bay) return 0;

    return static_cast<int>(bay->deployed_rover_ids.size());
}

} // namespace systems
} // namespace atlas
