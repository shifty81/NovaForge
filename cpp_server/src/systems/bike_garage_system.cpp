#include "systems/bike_garage_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

BikeGarageSystem::BikeGarageSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void BikeGarageSystem::updateComponent(ecs::Entity& /*entity*/, components::BikeGarage& garage, float delta_time) {
    // Door animation - only move if powered
    if (garage.power_enabled) {
        if (garage.is_open && garage.bay_door_progress < 1.0f) {
            garage.bay_door_progress += delta_time * garage.door_speed;
            if (garage.bay_door_progress > 1.0f) {
                garage.bay_door_progress = 1.0f;
            }
        } else if (!garage.is_open && garage.bay_door_progress > 0.0f) {
            garage.bay_door_progress -= delta_time * garage.door_speed;
            if (garage.bay_door_progress < 0.0f) {
                garage.bay_door_progress = 0.0f;
            }
        }
    }
}

bool BikeGarageSystem::initializeGarage(const std::string& entity_id,
                                         const std::string& owner_id,
                                         int capacity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::BikeGarage>();
    if (existing) return false;

    auto comp = std::make_unique<components::BikeGarage>();
    comp->owner_entity_id = owner_id;
    comp->max_capacity = std::max(1, capacity);
    comp->power_enabled = true;
    comp->is_open = false;
    comp->bay_door_progress = 0.0f;
    entity->addComponent(std::move(comp));
    return true;
}

bool BikeGarageSystem::removeGarage(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* garage = entity->getComponent<components::BikeGarage>();
    if (!garage) return false;

    entity->removeComponent<components::BikeGarage>();
    return true;
}

bool BikeGarageSystem::storeBike(const std::string& entity_id,
                                  const std::string& bike_id,
                                  uint64_t bike_seed,
                                  const std::string& faction_style) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    if (!garage->canStoreBike()) return false;

    // Check for duplicate
    if (garage->hasBike(bike_id)) return false;

    components::BikeGarage::StoredBike bike;
    bike.bike_id = bike_id;
    bike.bike_seed = bike_seed;
    bike.faction_style = faction_style;
    bike.fuel_level = 100.0f;
    bike.hull_integrity = 100.0f;
    bike.is_locked = false;

    garage->stored_bikes.push_back(bike);
    return true;
}

bool BikeGarageSystem::retrieveBike(const std::string& entity_id,
                                     const std::string& bike_id) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    auto it = std::find_if(garage->stored_bikes.begin(), garage->stored_bikes.end(),
        [&bike_id](const components::BikeGarage::StoredBike& b) {
            return b.bike_id == bike_id;
        });

    if (it == garage->stored_bikes.end()) return false;

    // Cannot retrieve locked bike
    if (it->is_locked) return false;

    garage->stored_bikes.erase(it);
    return true;
}

bool BikeGarageSystem::hasBike(const std::string& entity_id,
                                const std::string& bike_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    return garage->hasBike(bike_id);
}

int BikeGarageSystem::getBikeCount(const std::string& entity_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return 0;

    return garage->getBikeCount();
}

int BikeGarageSystem::getCapacity(const std::string& entity_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return 0;

    return garage->max_capacity;
}

bool BikeGarageSystem::isFull(const std::string& entity_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return true;

    return garage->isFull();
}

bool BikeGarageSystem::setBikeFuel(const std::string& entity_id,
                                    const std::string& bike_id,
                                    float fuel_percent) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    for (auto& bike : garage->stored_bikes) {
        if (bike.bike_id == bike_id) {
            bike.fuel_level = std::max(0.0f, std::min(100.0f, fuel_percent));
            return true;
        }
    }
    return false;
}

bool BikeGarageSystem::setBikeHullIntegrity(const std::string& entity_id,
                                             const std::string& bike_id,
                                             float integrity) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    for (auto& bike : garage->stored_bikes) {
        if (bike.bike_id == bike_id) {
            bike.hull_integrity = std::max(0.0f, std::min(100.0f, integrity));
            return true;
        }
    }
    return false;
}

bool BikeGarageSystem::lockBike(const std::string& entity_id,
                                 const std::string& bike_id) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    for (auto& bike : garage->stored_bikes) {
        if (bike.bike_id == bike_id) {
            bike.is_locked = true;
            return true;
        }
    }
    return false;
}

bool BikeGarageSystem::unlockBike(const std::string& entity_id,
                                   const std::string& bike_id) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    for (auto& bike : garage->stored_bikes) {
        if (bike.bike_id == bike_id) {
            bike.is_locked = false;
            return true;
        }
    }
    return false;
}

bool BikeGarageSystem::isBikeLocked(const std::string& entity_id,
                                     const std::string& bike_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    for (const auto& bike : garage->stored_bikes) {
        if (bike.bike_id == bike_id) {
            return bike.is_locked;
        }
    }
    return false;
}

float BikeGarageSystem::getBikeFuel(const std::string& entity_id,
                                     const std::string& bike_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return 0.0f;

    for (const auto& bike : garage->stored_bikes) {
        if (bike.bike_id == bike_id) {
            return bike.fuel_level;
        }
    }
    return 0.0f;
}

float BikeGarageSystem::getBikeHullIntegrity(const std::string& entity_id,
                                              const std::string& bike_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return 0.0f;

    for (const auto& bike : garage->stored_bikes) {
        if (bike.bike_id == bike_id) {
            return bike.hull_integrity;
        }
    }
    return 0.0f;
}

bool BikeGarageSystem::openDoor(const std::string& entity_id) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    if (!garage->power_enabled) return false;

    garage->is_open = true;
    return true;
}

bool BikeGarageSystem::closeDoor(const std::string& entity_id) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    if (!garage->power_enabled) return false;

    garage->is_open = false;
    return true;
}

bool BikeGarageSystem::isDoorOpen(const std::string& entity_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    return garage->is_open;
}

float BikeGarageSystem::getDoorProgress(const std::string& entity_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return 0.0f;

    return garage->bay_door_progress;
}

bool BikeGarageSystem::setPowerEnabled(const std::string& entity_id, bool enabled) {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    garage->power_enabled = enabled;
    return true;
}

bool BikeGarageSystem::isPowerEnabled(const std::string& entity_id) const {
    auto* garage = getComponentFor(entity_id);
    if (!garage) return false;

    return garage->power_enabled;
}

} // namespace systems
} // namespace atlas
