#ifndef NOVAFORGE_SYSTEMS_ROVER_INTERIOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ROVER_INTERIOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Rover interior management system (Phase 14)
 *
 * Manages rover interiors with rig locker, equipment mounts, and rooms.
 * Supports room addition/removal, equipment installation, rig storage,
 * and pressurization state tracking.
 */
class RoverInteriorSystem : public ecs::SingleComponentSystem<components::RoverInterior> {
public:
    explicit RoverInteriorSystem(ecs::World* world);
    ~RoverInteriorSystem() override = default;

    std::string getName() const override { return "RoverInteriorSystem"; }

    // Interior initialization
    bool initializeInterior(const std::string& entity_id, const std::string& rover_id);
    bool removeInterior(const std::string& entity_id);

    // Room management
    bool addRoom(const std::string& entity_id, const std::string& room_id,
                 components::RoverInterior::RoomType type);
    bool removeRoom(const std::string& entity_id, const std::string& room_id);
    int getRoomCount(const std::string& entity_id) const;
    std::string getRoomType(const std::string& entity_id, const std::string& room_id) const;

    // Equipment management
    bool installEquipment(const std::string& entity_id, const std::string& room_id,
                          const std::string& equipment_id);
    bool removeEquipment(const std::string& entity_id, const std::string& room_id,
                         const std::string& equipment_id);
    int getEquipmentCount(const std::string& entity_id, const std::string& room_id) const;

    // Rig locker management
    bool storeRig(const std::string& entity_id, const std::string& rig_id);
    bool retrieveRig(const std::string& entity_id, const std::string& rig_id);
    int getStoredRigCount(const std::string& entity_id) const;
    bool hasRigLocker(const std::string& entity_id) const;

    // Pressurization
    bool setPressurized(const std::string& entity_id, bool pressurized);
    bool isPressurized(const std::string& entity_id) const;
    float getOxygenLevel(const std::string& entity_id) const;
    bool setOxygenLevel(const std::string& entity_id, float level);

    // Query
    float getTotalVolume(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::RoverInterior& interior, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ROVER_INTERIOR_SYSTEM_H
