#include "systems/rover_interior_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

RoverInteriorSystem::RoverInteriorSystem(ecs::World* world)
    : System(world) {
}

void RoverInteriorSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::RoverInterior>();
    for (auto* entity : entities) {
        auto* interior = entity->getComponent<components::RoverInterior>();
        if (!interior) continue;

        // Calculate total volume from rooms
        float total_volume = 0.0f;
        for (const auto& room : interior->rooms) {
            total_volume += room.size_x * room.size_y * room.size_z;
        }
        interior->total_interior_volume = total_volume;

        // Oxygen level decay if not sealed
        if (!interior->is_sealed && interior->oxygen_level > 0.0f) {
            interior->oxygen_level -= delta_time * 5.0f; // 5% per second when unsealed
            if (interior->oxygen_level < 0.0f) {
                interior->oxygen_level = 0.0f;
            }
        }

        // Check for rig locker and equipment bay presence
        interior->has_rig_locker = false;
        interior->has_equipment_bay = false;
        for (const auto& room : interior->rooms) {
            if (room.type == components::RoverInterior::RoomType::RigLocker) {
                interior->has_rig_locker = true;
            }
            if (room.type == components::RoverInterior::RoomType::EquipmentBay) {
                interior->has_equipment_bay = true;
            }
        }
    }
}

bool RoverInteriorSystem::initializeInterior(const std::string& entity_id,
                                              const std::string& rover_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::RoverInterior>();
    if (existing) return false;

    auto comp = std::make_unique<components::RoverInterior>();
    comp->rover_id = rover_id;
    comp->is_sealed = true;
    comp->oxygen_level = 100.0f;
    entity->addComponent(std::move(comp));
    return true;
}

bool RoverInteriorSystem::removeInterior(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    entity->removeComponent<components::RoverInterior>();
    return true;
}

bool RoverInteriorSystem::addRoom(const std::string& entity_id,
                                   const std::string& room_id,
                                   components::RoverInterior::RoomType type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    if (!interior->canAddRoom()) return false;

    // Check for duplicate room_id
    for (const auto& room : interior->rooms) {
        if (room.room_id == room_id) return false;
    }

    components::RoverInterior::InteriorRoom room;
    room.room_id = room_id;
    room.type = type;

    // Set default sizes based on room type
    switch (type) {
        case components::RoverInterior::RoomType::Cockpit:
            room.size_x = 2.0f;
            room.size_y = 1.5f;
            room.size_z = 2.0f;
            room.equipment_slots = 2;
            break;
        case components::RoverInterior::RoomType::CargoHold:
            room.size_x = 3.0f;
            room.size_y = 2.0f;
            room.size_z = 2.5f;
            room.equipment_slots = 1;
            break;
        case components::RoverInterior::RoomType::RigLocker:
            room.size_x = 2.0f;
            room.size_y = 2.0f;
            room.size_z = 1.5f;
            room.equipment_slots = 4;
            break;
        case components::RoverInterior::RoomType::EquipmentBay:
            room.size_x = 2.5f;
            room.size_y = 2.0f;
            room.size_z = 2.0f;
            room.equipment_slots = 6;
            break;
        case components::RoverInterior::RoomType::Scanner:
            room.size_x = 1.5f;
            room.size_y = 1.5f;
            room.size_z = 1.5f;
            room.equipment_slots = 3;
            break;
        case components::RoverInterior::RoomType::Airlock:
            room.size_x = 1.5f;
            room.size_y = 2.0f;
            room.size_z = 1.5f;
            room.equipment_slots = 0;
            break;
    }

    interior->rooms.push_back(room);
    return true;
}

bool RoverInteriorSystem::removeRoom(const std::string& entity_id,
                                      const std::string& room_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    auto it = std::find_if(interior->rooms.begin(), interior->rooms.end(),
        [&room_id](const components::RoverInterior::InteriorRoom& r) {
            return r.room_id == room_id;
        });

    if (it == interior->rooms.end()) return false;
    interior->rooms.erase(it);
    return true;
}

int RoverInteriorSystem::getRoomCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return 0;

    return interior->getRoomCount();
}

std::string RoverInteriorSystem::getRoomType(const std::string& entity_id,
                                              const std::string& room_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return "unknown";

    for (const auto& room : interior->rooms) {
        if (room.room_id == room_id) {
            return components::RoverInterior::roomTypeToString(room.type);
        }
    }
    return "unknown";
}

bool RoverInteriorSystem::installEquipment(const std::string& entity_id,
                                            const std::string& room_id,
                                            const std::string& equipment_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    for (auto& room : interior->rooms) {
        if (room.room_id == room_id) {
            if (static_cast<int>(room.installed_equipment_ids.size()) >= room.equipment_slots) {
                return false;
            }
            room.installed_equipment_ids.push_back(equipment_id);
            return true;
        }
    }
    return false;
}

bool RoverInteriorSystem::removeEquipment(const std::string& entity_id,
                                           const std::string& room_id,
                                           const std::string& equipment_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    for (auto& room : interior->rooms) {
        if (room.room_id == room_id) {
            auto it = std::find(room.installed_equipment_ids.begin(),
                               room.installed_equipment_ids.end(),
                               equipment_id);
            if (it == room.installed_equipment_ids.end()) return false;
            room.installed_equipment_ids.erase(it);
            return true;
        }
    }
    return false;
}

int RoverInteriorSystem::getEquipmentCount(const std::string& entity_id,
                                            const std::string& room_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return 0;

    for (const auto& room : interior->rooms) {
        if (room.room_id == room_id) {
            return static_cast<int>(room.installed_equipment_ids.size());
        }
    }
    return 0;
}

bool RoverInteriorSystem::storeRig(const std::string& entity_id,
                                    const std::string& rig_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    if (!interior->has_rig_locker) return false;
    if (static_cast<int>(interior->stored_rig_ids.size()) >= interior->rig_locker_capacity) {
        return false;
    }

    // Check for duplicate
    if (std::find(interior->stored_rig_ids.begin(),
                  interior->stored_rig_ids.end(),
                  rig_id) != interior->stored_rig_ids.end()) {
        return false;
    }

    interior->stored_rig_ids.push_back(rig_id);
    return true;
}

bool RoverInteriorSystem::retrieveRig(const std::string& entity_id,
                                       const std::string& rig_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    auto it = std::find(interior->stored_rig_ids.begin(),
                       interior->stored_rig_ids.end(),
                       rig_id);
    if (it == interior->stored_rig_ids.end()) return false;
    interior->stored_rig_ids.erase(it);
    return true;
}

int RoverInteriorSystem::getStoredRigCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return 0;

    return static_cast<int>(interior->stored_rig_ids.size());
}

bool RoverInteriorSystem::hasRigLocker(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    return interior->has_rig_locker;
}

bool RoverInteriorSystem::setPressurized(const std::string& entity_id, bool pressurized) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    interior->is_sealed = pressurized;
    return true;
}

bool RoverInteriorSystem::isPressurized(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    return interior->is_sealed;
}

float RoverInteriorSystem::getOxygenLevel(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return 0.0f;

    return interior->oxygen_level;
}

bool RoverInteriorSystem::setOxygenLevel(const std::string& entity_id, float level) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return false;

    interior->oxygen_level = std::max(0.0f, std::min(100.0f, level));
    return true;
}

float RoverInteriorSystem::getTotalVolume(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* interior = entity->getComponent<components::RoverInterior>();
    if (!interior) return 0.0f;

    return interior->total_interior_volume;
}

} // namespace systems
} // namespace atlas
