#include "systems/salvage_drone_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SalvageDroneSystem::SalvageDroneSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SalvageDroneSystem::updateComponent(ecs::Entity& entity,
    components::SalvageDroneBay& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& drone : comp.drones) {
        if (drone.state != components::SalvageDroneBay::DroneState::Salvaging) continue;

        drone.cycle_progress += delta_time;
        if (drone.cycle_progress >= drone.cycle_time) {
            drone.cycle_progress -= drone.cycle_time;

            // Deterministic success check: use elapsed as seed substitute
            // Success if fractional part of elapsed * 1000 < success_chance * 1000
            float roll = comp.elapsed - static_cast<int>(comp.elapsed);
            if (roll < drone.success_chance) {
                drone.successful_salvages++;
                comp.total_salvages++;
                // After successful salvage, drone returns
                drone.state = components::SalvageDroneBay::DroneState::Returning;
            } else {
                comp.total_failures++;
                // Drone keeps trying
            }
        }
    }
}

bool SalvageDroneSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SalvageDroneBay>();
    entity->addComponent(std::move(comp));
    return true;
}

bool SalvageDroneSystem::addDrone(const std::string& entity_id,
    const std::string& drone_id, float cycle_time, float success_chance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->drones.size()) >= comp->max_drones) return false;

    // No duplicate drone IDs
    for (const auto& d : comp->drones) {
        if (d.drone_id == drone_id) return false;
    }

    components::SalvageDroneBay::SalvageDrone drone;
    drone.drone_id = drone_id;
    drone.cycle_time = cycle_time;
    drone.success_chance = std::max(0.0f, std::min(1.0f, success_chance));
    comp->drones.push_back(drone);
    return true;
}

bool SalvageDroneSystem::deployDrone(const std::string& entity_id,
    const std::string& drone_id, const std::string& wreck_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& d : comp->drones) {
        if (d.drone_id == drone_id) {
            if (d.state != components::SalvageDroneBay::DroneState::Idle) return false;
            d.wreck_target_id = wreck_id;
            d.state = components::SalvageDroneBay::DroneState::Deployed;
            d.cycle_progress = 0.0f;
            comp->total_deployed++;
            return true;
        }
    }
    return false;
}

bool SalvageDroneSystem::recallDrone(const std::string& entity_id,
    const std::string& drone_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& d : comp->drones) {
        if (d.drone_id == drone_id) {
            if (d.state == components::SalvageDroneBay::DroneState::Idle) return false;
            d.state = components::SalvageDroneBay::DroneState::Idle;
            d.wreck_target_id.clear();
            d.cycle_progress = 0.0f;
            return true;
        }
    }
    return false;
}

bool SalvageDroneSystem::recallAll(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    bool any = false;
    for (auto& d : comp->drones) {
        if (d.state != components::SalvageDroneBay::DroneState::Idle) {
            d.state = components::SalvageDroneBay::DroneState::Idle;
            d.wreck_target_id.clear();
            d.cycle_progress = 0.0f;
            any = true;
        }
    }
    return any;
}

int SalvageDroneSystem::getDroneCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->drones.size()) : 0;
}

int SalvageDroneSystem::getDeployedCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(std::count_if(comp->drones.begin(), comp->drones.end(),
        [](const components::SalvageDroneBay::SalvageDrone& d) {
            return d.state != components::SalvageDroneBay::DroneState::Idle;
        }));
}

int SalvageDroneSystem::getTotalSalvages(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_salvages : 0;
}

int SalvageDroneSystem::getTotalFailures(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_failures : 0;
}

std::string SalvageDroneSystem::getDroneState(const std::string& entity_id,
    const std::string& drone_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "unknown";
    for (const auto& d : comp->drones) {
        if (d.drone_id == drone_id) {
            switch (d.state) {
                case components::SalvageDroneBay::DroneState::Idle: return "idle";
                case components::SalvageDroneBay::DroneState::Deployed: return "deployed";
                case components::SalvageDroneBay::DroneState::Salvaging: return "salvaging";
                case components::SalvageDroneBay::DroneState::Returning: return "returning";
            }
        }
    }
    return "unknown";
}

} // namespace systems
} // namespace atlas
