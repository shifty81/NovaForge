#ifndef NOVAFORGE_SYSTEMS_BIKE_GARAGE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_BIKE_GARAGE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Bike garage management system (Phase 14)
 *
 * Manages grav bike storage in rovers and ships. Supports store/retrieve
 * operations with capacity limits, door state management, and power requirements.
 */
class BikeGarageSystem : public ecs::SingleComponentSystem<components::BikeGarage> {
public:
    explicit BikeGarageSystem(ecs::World* world);
    ~BikeGarageSystem() override = default;

    std::string getName() const override { return "BikeGarageSystem"; }

    // Garage initialization
    bool initializeGarage(const std::string& entity_id, const std::string& owner_id, int capacity = 2);
    bool removeGarage(const std::string& entity_id);

    // Bike storage operations
    bool storeBike(const std::string& entity_id, const std::string& bike_id,
                   uint64_t bike_seed, const std::string& faction_style);
    bool retrieveBike(const std::string& entity_id, const std::string& bike_id);
    bool hasBike(const std::string& entity_id, const std::string& bike_id) const;
    int getBikeCount(const std::string& entity_id) const;
    int getCapacity(const std::string& entity_id) const;
    bool isFull(const std::string& entity_id) const;

    // Bike state management
    bool setBikeFuel(const std::string& entity_id, const std::string& bike_id, float fuel_percent);
    bool setBikeHullIntegrity(const std::string& entity_id, const std::string& bike_id, float integrity);
    bool lockBike(const std::string& entity_id, const std::string& bike_id);
    bool unlockBike(const std::string& entity_id, const std::string& bike_id);
    bool isBikeLocked(const std::string& entity_id, const std::string& bike_id) const;
    float getBikeFuel(const std::string& entity_id, const std::string& bike_id) const;
    float getBikeHullIntegrity(const std::string& entity_id, const std::string& bike_id) const;

    // Door management
    bool openDoor(const std::string& entity_id);
    bool closeDoor(const std::string& entity_id);
    bool isDoorOpen(const std::string& entity_id) const;
    float getDoorProgress(const std::string& entity_id) const;

    // Power management
    bool setPowerEnabled(const std::string& entity_id, bool enabled);
    bool isPowerEnabled(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::BikeGarage& garage, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_BIKE_GARAGE_SYSTEM_H
