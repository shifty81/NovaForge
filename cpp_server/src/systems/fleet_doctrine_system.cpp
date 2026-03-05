#include "systems/fleet_doctrine_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetDoctrineSystem::FleetDoctrineSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void FleetDoctrineSystem::updateComponent(ecs::Entity& /*entity*/,
                                           components::FleetDoctrine& doctrine,
                                           float /*delta_time*/) {
    // Calculate readiness: fraction of required slots that meet minimum count
    int required_slots = 0;
    int filled_required = 0;
    int total_target = 0;
    int total_current = 0;

    for (const auto& slot : doctrine.slots) {
        total_target += slot.max_count;
        total_current += slot.current_count;
        if (slot.required) {
            required_slots++;
            if (slot.current_count >= slot.min_count) {
                filled_required++;
            }
        }
    }

    doctrine.readiness = (required_slots > 0)
        ? static_cast<float>(filled_required) / static_cast<float>(required_slots)
        : 1.0f;  // no required slots = always ready

    doctrine.total_ship_target = total_target;
    doctrine.current_ship_count = total_current;
}

bool FleetDoctrineSystem::createDoctrine(const std::string& entity_id, const std::string& doctrine_id, const std::string& name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::FleetDoctrine>();
    if (existing) return false; // already has a doctrine

    auto doctrine_comp = std::make_unique<components::FleetDoctrine>();
    doctrine_comp->doctrine_id = doctrine_id;
    doctrine_comp->doctrine_name = name;
    doctrine_comp->is_locked = false;
    entity->addComponent(std::move(doctrine_comp));
    return true;
}

bool FleetDoctrineSystem::addSlot(const std::string& entity_id, const std::string& role, const std::string& ship_class,
                                   int min_count, int max_count, bool required) {
    auto* doctrine = getComponentFor(entity_id);
    if (!doctrine || doctrine->is_locked) return false;

    // Check for duplicate role
    for (const auto& s : doctrine->slots) {
        if (s.role == role) return false;
    }

    components::FleetDoctrine::DoctrineSlot slot;
    slot.role = role;
    slot.ship_class = ship_class;
    slot.min_count = min_count;
    slot.max_count = max_count;
    slot.required = required;
    slot.current_count = 0;
    doctrine->slots.push_back(slot);
    return true;
}

bool FleetDoctrineSystem::removeSlot(const std::string& entity_id, const std::string& role) {
    auto* doctrine = getComponentFor(entity_id);
    if (!doctrine || doctrine->is_locked) return false;

    auto it = std::remove_if(doctrine->slots.begin(), doctrine->slots.end(),
        [&role](const components::FleetDoctrine::DoctrineSlot& s) {
            return s.role == role;
        });

    if (it == doctrine->slots.end()) return false;
    doctrine->slots.erase(it, doctrine->slots.end());
    return true;
}

bool FleetDoctrineSystem::assignShip(const std::string& entity_id, const std::string& role) {
    auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return false;

    for (auto& slot : doctrine->slots) {
        if (slot.role == role) {
            if (slot.current_count >= slot.max_count) return false;
            slot.current_count++;
            return true;
        }
    }
    return false;
}

bool FleetDoctrineSystem::unassignShip(const std::string& entity_id, const std::string& role) {
    auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return false;

    for (auto& slot : doctrine->slots) {
        if (slot.role == role) {
            if (slot.current_count <= 0) return false;
            slot.current_count--;
            return true;
        }
    }
    return false;
}

bool FleetDoctrineSystem::lockDoctrine(const std::string& entity_id, bool locked) {
    auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return false;

    doctrine->is_locked = locked;
    return true;
}

float FleetDoctrineSystem::getReadiness(const std::string& entity_id) const {
    const auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return 0.0f;

    return doctrine->readiness;
}

int FleetDoctrineSystem::getSlotCount(const std::string& entity_id) const {
    const auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return 0;

    return static_cast<int>(doctrine->slots.size());
}

int FleetDoctrineSystem::getCurrentShipCount(const std::string& entity_id) const {
    const auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return 0;

    return doctrine->current_ship_count;
}

int FleetDoctrineSystem::getTargetShipCount(const std::string& entity_id) const {
    const auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return 0;

    return doctrine->total_ship_target;
}

bool FleetDoctrineSystem::isReady(const std::string& entity_id) const {
    const auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return false;

    return doctrine->readiness >= 1.0f;
}

bool FleetDoctrineSystem::isLocked(const std::string& entity_id) const {
    const auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return false;

    return doctrine->is_locked;
}

std::string FleetDoctrineSystem::getDoctrineName(const std::string& entity_id) const {
    const auto* doctrine = getComponentFor(entity_id);
    if (!doctrine) return "";

    return doctrine->doctrine_name;
}

} // namespace systems
} // namespace atlas
