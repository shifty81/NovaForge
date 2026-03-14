#include "systems/hangar_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

HangarSystem::HangarSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void HangarSystem::updateComponent(ecs::Entity& /*entity*/,
                                    components::HangarState& comp,
                                    float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool HangarSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::HangarState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Ship management
// ---------------------------------------------------------------------------

bool HangarSystem::storeShip(const std::string& entity_id,
                              const std::string& ship_id,
                              const std::string& ship_type,
                              const std::string& ship_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (ship_id.empty()) return false;
    if (ship_type.empty()) return false;
    if (ship_name.empty()) return false;

    // Duplicate check
    for (const auto& s : comp->ships) {
        if (s.ship_id == ship_id) return false;
    }

    // Capacity check
    if (static_cast<int>(comp->ships.size()) >= comp->max_ships) {
        return false;
    }

    components::HangarState::HangarShip ship;
    ship.ship_id   = ship_id;
    ship.ship_type = ship_type;
    ship.ship_name = ship_name;
    comp->ships.push_back(ship);

    comp->total_ships_stored++;
    return true;
}

bool HangarSystem::retrieveShip(const std::string& entity_id,
                                 const std::string& ship_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->ships.begin(), comp->ships.end(),
        [&](const components::HangarState::HangarShip& s) {
            return s.ship_id == ship_id;
        });
    if (it == comp->ships.end()) return false;
    comp->ships.erase(it);
    comp->total_ships_retrieved++;
    return true;
}

bool HangarSystem::renameShip(const std::string& entity_id,
                               const std::string& ship_id,
                               const std::string& new_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (new_name.empty()) return false;
    for (auto& s : comp->ships) {
        if (s.ship_id == ship_id) {
            s.ship_name = new_name;
            return true;
        }
    }
    return false;
}

bool HangarSystem::setActiveShip(const std::string& entity_id,
                                  const std::string& ship_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    bool found = false;
    for (auto& s : comp->ships) {
        if (s.ship_id == ship_id) {
            found = true;
        }
    }
    if (!found) return false;

    // Deactivate all, then activate the target
    for (auto& s : comp->ships) {
        s.is_active = (s.ship_id == ship_id);
    }
    return true;
}

bool HangarSystem::setInsurance(const std::string& entity_id,
                                 const std::string& ship_id,
                                 float insurance_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (insurance_value < 0.0f) return false;
    for (auto& s : comp->ships) {
        if (s.ship_id == ship_id) {
            s.insurance = insurance_value;
            return true;
        }
    }
    return false;
}

bool HangarSystem::clearHangar(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->ships.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool HangarSystem::setStationId(const std::string& entity_id,
                                 const std::string& station_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (station_id.empty()) return false;
    comp->station_id = station_id;
    return true;
}

bool HangarSystem::setMaxShips(const std::string& entity_id, int max_ships) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_ships <= 0) return false;
    comp->max_ships = max_ships;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int HangarSystem::getShipCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->ships.size()) : 0;
}

bool HangarSystem::hasShip(const std::string& entity_id,
                            const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->ships) {
        if (s.ship_id == ship_id) return true;
    }
    return false;
}

std::string HangarSystem::getShipName(const std::string& entity_id,
                                       const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& s : comp->ships) {
        if (s.ship_id == ship_id) return s.ship_name;
    }
    return "";
}

std::string HangarSystem::getShipType(const std::string& entity_id,
                                       const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& s : comp->ships) {
        if (s.ship_id == ship_id) return s.ship_type;
    }
    return "";
}

std::string HangarSystem::getActiveShipId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& s : comp->ships) {
        if (s.is_active) return s.ship_id;
    }
    return "";
}

std::string HangarSystem::getStationId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->station_id : "";
}

float HangarSystem::getInsurance(const std::string& entity_id,
                                  const std::string& ship_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& s : comp->ships) {
        if (s.ship_id == ship_id) return s.insurance;
    }
    return 0.0f;
}

int HangarSystem::getTotalShipsStored(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_ships_stored : 0;
}

int HangarSystem::getTotalShipsRetrieved(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_ships_retrieved : 0;
}

int HangarSystem::getShipCountByType(const std::string& entity_id,
                                      const std::string& ship_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->ships) {
        if (s.ship_type == ship_type) count++;
    }
    return count;
}

} // namespace systems
} // namespace atlas
