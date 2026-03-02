#include "systems/fleet_hangar_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

FleetHangarSystem::FleetHangarSystem(ecs::World* world)
    : System(world) {
}

void FleetHangarSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::FleetHangar>();
    for (auto* entity : entities) {
        auto* hangar = entity->getComponent<components::FleetHangar>();
        if (!hangar) continue;

        // Accrue maintenance cost when powered
        if (hangar->is_powered) {
            hangar->total_maintenance_paid += hangar->maintenance_cost_per_tick * delta_time;
        }
    }
}

bool FleetHangarSystem::initializeHangar(const std::string& entity_id,
                                          const std::string& owner_id,
                                          const std::string& name, int tier) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::FleetHangar>();
    if (existing) return false;

    int clamped_tier = std::max(1, std::min(5, tier));

    auto comp = std::make_unique<components::FleetHangar>();
    comp->owner_entity_id = owner_id;
    comp->hangar_name = name;
    comp->tier = clamped_tier;
    comp->max_ship_slots = components::FleetHangar::getMaxSlotsForTier(clamped_tier);
    comp->upgrade_cost = components::FleetHangar::getUpgradeCostForTier(clamped_tier);
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetHangarSystem::dockShip(const std::string& entity_id, const std::string& ship_id,
                                  const std::string& ship_class, float hull_integrity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return false;

    if (!hangar->is_powered) return false;
    if (static_cast<int>(hangar->current_ships.size()) >= hangar->max_ship_slots) return false;

    // Check not already docked
    for (const auto& s : hangar->current_ships) {
        if (s.ship_id == ship_id) return false;
    }

    components::FleetHangar::StoredShip ship;
    ship.ship_id = ship_id;
    ship.ship_class = ship_class;
    ship.hull_integrity = hull_integrity;
    ship.is_locked = false;
    hangar->current_ships.push_back(ship);
    hangar->total_ships_docked++;
    return true;
}

bool FleetHangarSystem::undockShip(const std::string& entity_id, const std::string& ship_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return false;

    if (!hangar->is_powered) return false;

    for (auto it = hangar->current_ships.begin(); it != hangar->current_ships.end(); ++it) {
        if (it->ship_id == ship_id) {
            if (it->is_locked) return false;
            hangar->current_ships.erase(it);
            return true;
        }
    }
    return false;
}

bool FleetHangarSystem::lockShip(const std::string& entity_id, const std::string& ship_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return false;

    for (auto& s : hangar->current_ships) {
        if (s.ship_id == ship_id) {
            s.is_locked = true;
            return true;
        }
    }
    return false;
}

bool FleetHangarSystem::unlockShip(const std::string& entity_id, const std::string& ship_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return false;

    for (auto& s : hangar->current_ships) {
        if (s.ship_id == ship_id) {
            s.is_locked = false;
            return true;
        }
    }
    return false;
}

bool FleetHangarSystem::repairShip(const std::string& entity_id, const std::string& ship_id,
                                    float amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return false;

    for (auto& s : hangar->current_ships) {
        if (s.ship_id == ship_id) {
            s.hull_integrity = std::min(100.0f, s.hull_integrity + amount);
            return true;
        }
    }
    return false;
}

bool FleetHangarSystem::upgradeHangar(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return false;

    if (hangar->tier >= 5) return false;

    hangar->tier++;
    hangar->max_ship_slots = components::FleetHangar::getMaxSlotsForTier(hangar->tier);
    hangar->upgrade_cost = components::FleetHangar::getUpgradeCostForTier(hangar->tier);
    return true;
}

int FleetHangarSystem::getShipCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return 0;

    return static_cast<int>(hangar->current_ships.size());
}

int FleetHangarSystem::getMaxSlots(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return 0;

    return hangar->max_ship_slots;
}

int FleetHangarSystem::getTier(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return 0;

    return hangar->tier;
}

bool FleetHangarSystem::setPowerEnabled(const std::string& entity_id, bool enabled) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::FleetHangar>();
    if (!hangar) return false;

    hangar->is_powered = enabled;
    return true;
}

} // namespace systems
} // namespace atlas
