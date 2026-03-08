#include "systems/hull_class_capability_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

HullClassCapabilitySystem::HullClassCapabilitySystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void HullClassCapabilitySystem::updateComponent(ecs::Entity& /*entity*/,
                                                 components::HullClassCapabilityProfile& /*cap*/,
                                                 float /*delta_time*/) {
    // Capability profiles are static data; no per-tick update needed.
}

bool HullClassCapabilitySystem::initializeProfile(const std::string& entity_id,
                                                   const std::string& hull_class) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->getComponent<components::HullClassCapabilityProfile>()) return false;

    auto comp = std::make_unique<components::HullClassCapabilityProfile>();
    comp->hull_class = hull_class;

    // Apply capability table from ideas.md design (lines 1824-1877)
    if (hull_class == "Frigate" || hull_class == "Shuttle") {
        // Sub-destroyer: no bays, minimal slots
        comp->rover_bay_count = 0;
        comp->grav_bike_bay_count = 0;
        comp->ship_hangar_count = 0;
        comp->has_survival_module = false;
        comp->has_rig_locker = false;
        comp->max_power_grid = 50.0f;
        comp->max_cpu = 150.0f;
        comp->max_module_slots = 4;
        comp->max_cargo_volume = 500.0f;
    } else if (hull_class == "Destroyer") {
        comp->rover_bay_count = 1;
        comp->rover_bay_class = "S";
        comp->grav_bike_bay_count = 1;
        comp->grav_bike_bay_class = "S";
        comp->ship_hangar_count = 1;
        comp->ship_hangar_class = "S";
        comp->has_survival_module = true;
        comp->has_rig_locker = true;
        comp->max_power_grid = 200.0f;
        comp->max_cpu = 400.0f;
        comp->max_module_slots = 8;
        comp->max_cargo_volume = 2000.0f;
    } else if (hull_class == "Cruiser") {
        comp->rover_bay_count = 1;
        comp->rover_bay_class = "M";
        comp->grav_bike_bay_count = 1;
        comp->grav_bike_bay_class = "S";
        comp->ship_hangar_count = 1;
        comp->ship_hangar_class = "S";
        comp->has_survival_module = true;
        comp->has_rig_locker = true;
        comp->max_power_grid = 400.0f;
        comp->max_cpu = 600.0f;
        comp->max_module_slots = 12;
        comp->max_cargo_volume = 5000.0f;
    } else if (hull_class == "Battlecruiser") {
        comp->rover_bay_count = 1;
        comp->rover_bay_class = "M";
        comp->grav_bike_bay_count = 1;
        comp->grav_bike_bay_class = "M";
        comp->ship_hangar_count = 1;
        comp->ship_hangar_class = "M";
        comp->has_survival_module = true;
        comp->has_rig_locker = true;
        comp->max_power_grid = 600.0f;
        comp->max_cpu = 800.0f;
        comp->max_module_slots = 14;
        comp->max_cargo_volume = 8000.0f;
    } else if (hull_class == "Battleship") {
        comp->rover_bay_count = 1;
        comp->rover_bay_class = "M";
        comp->grav_bike_bay_count = 1;
        comp->grav_bike_bay_class = "M";
        comp->ship_hangar_count = 1;
        comp->ship_hangar_class = "M";
        comp->has_survival_module = true;
        comp->has_rig_locker = true;
        comp->max_power_grid = 800.0f;
        comp->max_cpu = 1000.0f;
        comp->max_module_slots = 16;
        comp->max_cargo_volume = 10000.0f;
    } else if (hull_class == "Carrier") {
        comp->rover_bay_count = 2;
        comp->rover_bay_class = "L";
        comp->grav_bike_bay_count = 2;
        comp->grav_bike_bay_class = "M";
        comp->ship_hangar_count = 3;
        comp->ship_hangar_class = "L";
        comp->has_survival_module = true;
        comp->has_rig_locker = true;
        comp->max_power_grid = 1500.0f;
        comp->max_cpu = 2000.0f;
        comp->max_module_slots = 20;
        comp->max_cargo_volume = 25000.0f;
    } else if (hull_class == "Dreadnought") {
        comp->rover_bay_count = 2;
        comp->rover_bay_class = "L";
        comp->grav_bike_bay_count = 2;
        comp->grav_bike_bay_class = "L";
        comp->ship_hangar_count = 2;
        comp->ship_hangar_class = "L";
        comp->has_survival_module = true;
        comp->has_rig_locker = true;
        comp->max_power_grid = 2000.0f;
        comp->max_cpu = 2500.0f;
        comp->max_module_slots = 24;
        comp->max_cargo_volume = 35000.0f;
    } else if (hull_class == "Titan") {
        comp->rover_bay_count = 3;
        comp->rover_bay_class = "XL";
        comp->grav_bike_bay_count = 3;
        comp->grav_bike_bay_class = "L";
        comp->ship_hangar_count = 4;
        comp->ship_hangar_class = "L";
        comp->has_survival_module = true;
        comp->has_rig_locker = true;
        comp->max_power_grid = 5000.0f;
        comp->max_cpu = 5000.0f;
        comp->max_module_slots = 32;
        comp->max_cargo_volume = 100000.0f;
    } else {
        // Unknown hull class — defaults (same as Frigate)
        comp->max_power_grid = 50.0f;
        comp->max_cpu = 150.0f;
        comp->max_module_slots = 4;
        comp->max_cargo_volume = 500.0f;
    }

    entity->addComponent(std::move(comp));
    return true;
}

std::string HullClassCapabilitySystem::getHullClass(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return "";
    return cap->hull_class;
}

int HullClassCapabilitySystem::getRoverBayCount(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return 0;
    return cap->rover_bay_count;
}

int HullClassCapabilitySystem::getGravBikeBayCount(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return 0;
    return cap->grav_bike_bay_count;
}

int HullClassCapabilitySystem::getShipHangarCount(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return 0;
    return cap->ship_hangar_count;
}

std::string HullClassCapabilitySystem::getRoverBayClass(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return "";
    return cap->rover_bay_class;
}

std::string HullClassCapabilitySystem::getGravBikeBayClass(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return "";
    return cap->grav_bike_bay_class;
}

std::string HullClassCapabilitySystem::getShipHangarClass(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return "";
    return cap->ship_hangar_class;
}

bool HullClassCapabilitySystem::hasSurvivalModule(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return false;
    return cap->has_survival_module;
}

bool HullClassCapabilitySystem::hasRigLocker(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return false;
    return cap->has_rig_locker;
}

float HullClassCapabilitySystem::getMaxPowerGrid(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return 0.0f;
    return cap->max_power_grid;
}

float HullClassCapabilitySystem::getMaxCPU(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return 0.0f;
    return cap->max_cpu;
}

int HullClassCapabilitySystem::getMaxModuleSlots(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return 0;
    return cap->max_module_slots;
}

float HullClassCapabilitySystem::getMaxCargoVolume(const std::string& entity_id) const {
    const auto* cap = getComponentFor(entity_id);
    if (!cap) return 0.0f;
    return cap->max_cargo_volume;
}

bool HullClassCapabilitySystem::setRoverBayCount(const std::string& entity_id, int count) {
    auto* cap = getComponentFor(entity_id);
    if (!cap) return false;
    cap->rover_bay_count = std::max(0, count);
    cap->has_rig_locker = cap->rover_bay_count > 0;
    return true;
}

bool HullClassCapabilitySystem::setGravBikeBayCount(const std::string& entity_id, int count) {
    auto* cap = getComponentFor(entity_id);
    if (!cap) return false;
    cap->grav_bike_bay_count = std::max(0, count);
    return true;
}

bool HullClassCapabilitySystem::setShipHangarCount(const std::string& entity_id, int count) {
    auto* cap = getComponentFor(entity_id);
    if (!cap) return false;
    cap->ship_hangar_count = std::max(0, count);
    return true;
}

} // namespace systems
} // namespace atlas
