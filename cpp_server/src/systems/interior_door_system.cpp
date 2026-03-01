#include "systems/interior_door_system.h"
#include "ecs/world.h"
#include <memory>
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

InteriorDoorSystem::InteriorDoorSystem(ecs::World* world)
    : System(world) {
}

void InteriorDoorSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::InteriorDoor>();
    for (auto* entity : entities) {
        auto* door = entity->getComponent<components::InteriorDoor>();
        if (!door) continue;

        using DS = components::InteriorDoor::DoorState;

        // Update pressure warning
        float diff = std::fabs(door->pressure_a - door->pressure_b);
        door->pressure_warning = (diff > door->pressure_threshold);

        int state = door->door_state;

        if (state == static_cast<int>(DS::Opening)) {
            float step = delta_time / std::max(0.01f, door->open_speed);
            door->open_progress = std::min(1.0f, door->open_progress + step);
            if (door->open_progress >= 1.0f) {
                door->door_state = static_cast<int>(DS::Open);
                door->auto_close_timer = door->auto_close_delay;
            }
        }
        else if (state == static_cast<int>(DS::Closing)) {
            float step = delta_time / std::max(0.01f, door->open_speed);
            door->open_progress = std::max(0.0f, door->open_progress - step);
            if (door->open_progress <= 0.0f) {
                door->door_state = static_cast<int>(DS::Closed);
            }
        }
        else if (state == static_cast<int>(DS::Open)) {
            // Auto-close countdown
            if (door->auto_close_delay > 0.0f) {
                door->auto_close_timer -= delta_time;
                if (door->auto_close_timer <= 0.0f) {
                    door->door_state = static_cast<int>(DS::Closing);
                    door->auto_close_timer = 0.0f;
                }
            }
        }
    }
}

bool InteriorDoorSystem::createDoor(
        const std::string& door_id,
        const std::string& interior_id,
        const std::string& room_a_id,
        const std::string& room_b_id,
        int door_type) {
    if (world_->getEntity(door_id)) return false;

    auto* entity = world_->createEntity(door_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::InteriorDoor>();
    comp->door_id = door_id;
    comp->interior_id = interior_id;
    comp->room_a_id = room_a_id;
    comp->room_b_id = room_b_id;
    comp->door_type = door_type;
    comp->door_state = static_cast<int>(components::InteriorDoor::DoorState::Closed);
    entity->addComponent(std::move(comp));
    return true;
}

bool InteriorDoorSystem::requestOpen(const std::string& door_id,
                                      const std::string& player_access) {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return false;
    auto* door = entity->getComponent<components::InteriorDoor>();
    if (!door) return false;

    using DS = components::InteriorDoor::DoorState;
    using DT = components::InteriorDoor::DoorType;

    // Can't open if locked
    if (door->is_locked) return false;

    // Can only open from Closed state
    if (door->door_state != static_cast<int>(DS::Closed)) return false;

    // Security door access check
    if (door->door_type == static_cast<int>(DT::Security)) {
        if (!door->required_access.empty() && player_access != door->required_access) {
            return false;
        }
    }

    // Airlock pressure check
    if (door->door_type == static_cast<int>(DT::Airlock)) {
        float diff = std::fabs(door->pressure_a - door->pressure_b);
        if (diff > door->pressure_threshold) {
            return false;  // Pressure differential too high
        }
    }

    door->door_state = static_cast<int>(DS::Opening);
    return true;
}

bool InteriorDoorSystem::requestClose(const std::string& door_id) {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return false;
    auto* door = entity->getComponent<components::InteriorDoor>();
    if (!door) return false;

    using DS = components::InteriorDoor::DoorState;

    // Can only close from Open state
    if (door->door_state != static_cast<int>(DS::Open)) return false;

    door->door_state = static_cast<int>(DS::Closing);
    return true;
}

bool InteriorDoorSystem::lockDoor(const std::string& door_id) {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return false;
    auto* door = entity->getComponent<components::InteriorDoor>();
    if (!door) return false;

    // Can only lock when closed
    using DS = components::InteriorDoor::DoorState;
    if (door->door_state != static_cast<int>(DS::Closed)) return false;

    door->is_locked = true;
    door->door_state = static_cast<int>(DS::Locked);
    return true;
}

bool InteriorDoorSystem::unlockDoor(const std::string& door_id) {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return false;
    auto* door = entity->getComponent<components::InteriorDoor>();
    if (!door) return false;

    using DS = components::InteriorDoor::DoorState;
    if (door->door_state != static_cast<int>(DS::Locked)) return false;

    door->is_locked = false;
    door->door_state = static_cast<int>(DS::Closed);
    return true;
}

bool InteriorDoorSystem::setPressure(const std::string& door_id,
                                      float pressure_a, float pressure_b) {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return false;
    auto* door = entity->getComponent<components::InteriorDoor>();
    if (!door) return false;

    door->pressure_a = std::max(0.0f, pressure_a);
    door->pressure_b = std::max(0.0f, pressure_b);
    return true;
}

int InteriorDoorSystem::getDoorState(const std::string& door_id) const {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return -1;
    auto* door = entity->getComponent<components::InteriorDoor>();
    return door ? door->door_state : -1;
}

float InteriorDoorSystem::getOpenProgress(const std::string& door_id) const {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return 0.0f;
    auto* door = entity->getComponent<components::InteriorDoor>();
    return door ? door->open_progress : 0.0f;
}

bool InteriorDoorSystem::hasPressureWarning(const std::string& door_id) const {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return false;
    auto* door = entity->getComponent<components::InteriorDoor>();
    return door ? door->pressure_warning : false;
}

bool InteriorDoorSystem::isLocked(const std::string& door_id) const {
    auto* entity = world_->getEntity(door_id);
    if (!entity) return false;
    auto* door = entity->getComponent<components::InteriorDoor>();
    return door ? door->is_locked : false;
}

std::string InteriorDoorSystem::stateName(int state) {
    using DS = components::InteriorDoor::DoorState;
    switch (static_cast<DS>(state)) {
        case DS::Closed:  return "Closed";
        case DS::Opening: return "Opening";
        case DS::Open:    return "Open";
        case DS::Closing: return "Closing";
        case DS::Locked:  return "Locked";
        default: return "Unknown";
    }
}

std::string InteriorDoorSystem::typeName(int type) {
    using DT = components::InteriorDoor::DoorType;
    switch (static_cast<DT>(type)) {
        case DT::Standard: return "Standard";
        case DT::Airlock:  return "Airlock";
        case DT::Security: return "Security";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
