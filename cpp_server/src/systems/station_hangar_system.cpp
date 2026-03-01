#include "systems/station_hangar_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

StationHangarSystem::StationHangarSystem(ecs::World* world)
    : System(world) {
}

void StationHangarSystem::update(float delta_time) {
    // Accrue rental costs for leased hangars.
    // delta_time is in seconds; convert to fractional days.
    float days = delta_time / 86400.0f;

    for (auto* entity : world_->getEntities<components::StationHangar>()) {
        auto* hangar = entity->getComponent<components::StationHangar>();
        if (!hangar) continue;
        if (hangar->type != components::StationHangar::HangarType::Leased) continue;

        hangar->days_rented += days;
        hangar->accumulated_rental += hangar->daily_rental_cost * static_cast<double>(days);
    }
}

// ---------------------------------------------------------------------------
// Hangar creation
// ---------------------------------------------------------------------------

bool StationHangarSystem::createHangar(
        const std::string& hangar_id,
        const std::string& station_id,
        const std::string& owner_id,
        components::StationHangar::HangarType type) {

    if (world_->getEntity(hangar_id)) return false; // already exists

    auto* entity = world_->createEntity(hangar_id);
    if (!entity) return false;

    auto h = std::make_unique<components::StationHangar>();
    h->hangar_id  = hangar_id;
    h->station_id = station_id;
    h->owner_id   = owner_id;
    h->type       = type;

    // Set upgrade level and capacity based on type.
    auto lvl = components::StationHangar::UpgradeLevel::Basic;
    h->upgrade_level   = lvl;
    h->max_ship_slots  = components::StationHangar::maxSlotsForLevel(lvl);
    h->capacity_volume = components::StationHangar::capacityForLevel(lvl);

    // Default FPS spawn inside the hangar (slightly offset from centre).
    h->spawn_x = 5.0f;
    h->spawn_y = 0.0f;
    h->spawn_z = 2.0f;

    entity->addComponent(std::move(h));
    return true;
}

// ---------------------------------------------------------------------------
// Ship storage
// ---------------------------------------------------------------------------

bool StationHangarSystem::storeShip(const std::string& hangar_id,
                                     const std::string& ship_id) {
    auto* entity = world_->getEntity(hangar_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::StationHangar>();
    if (!hangar) return false;
    if (!hangar->hasRoom()) return false;

    // Check the ship isn't already stored.
    for (const auto& id : hangar->stored_ship_ids) {
        if (id == ship_id) return false;
    }

    hangar->stored_ship_ids.push_back(ship_id);
    hangar->occupied_ship_slots++;
    return true;
}

bool StationHangarSystem::retrieveShip(const std::string& hangar_id,
                                        const std::string& ship_id) {
    auto* entity = world_->getEntity(hangar_id);
    if (!entity) return false;

    auto* hangar = entity->getComponent<components::StationHangar>();
    if (!hangar) return false;

    auto& ships = hangar->stored_ship_ids;
    auto it = std::find(ships.begin(), ships.end(), ship_id);
    if (it == ships.end()) return false;

    ships.erase(it);
    hangar->occupied_ship_slots--;
    return true;
}

// ---------------------------------------------------------------------------
// Upgrades
// ---------------------------------------------------------------------------

double StationHangarSystem::upgradeHangar(const std::string& hangar_id) {
    auto* entity = world_->getEntity(hangar_id);
    if (!entity) return 0.0;

    auto* hangar = entity->getComponent<components::StationHangar>();
    if (!hangar) return 0.0;

    double cost = 0.0;
    auto next = hangar->upgrade_level;
    switch (hangar->upgrade_level) {
        case components::StationHangar::UpgradeLevel::Basic:
            next = components::StationHangar::UpgradeLevel::Standard;
            cost = UPGRADE_COST_STANDARD;
            break;
        case components::StationHangar::UpgradeLevel::Standard:
            next = components::StationHangar::UpgradeLevel::Advanced;
            cost = UPGRADE_COST_ADVANCED;
            break;
        case components::StationHangar::UpgradeLevel::Advanced:
            next = components::StationHangar::UpgradeLevel::Premium;
            cost = UPGRADE_COST_PREMIUM;
            break;
        case components::StationHangar::UpgradeLevel::Premium:
            return 0.0; // already max
    }

    hangar->upgrade_level   = next;
    hangar->max_ship_slots  = components::StationHangar::maxSlotsForLevel(next);
    hangar->capacity_volume = components::StationHangar::capacityForLevel(next);
    return cost;
}

// ---------------------------------------------------------------------------
// Ship size routing
// ---------------------------------------------------------------------------

bool StationHangarSystem::shouldUseHangar(const std::string& ship_id) const {
    auto* ship_entity = world_->getEntity(ship_id);
    if (!ship_entity) return true; // default to hangar

    auto* ship = ship_entity->getComponent<components::Ship>();
    if (ship) {
        // Capital-class and larger ships must use tether docking.
        const std::string& cls = ship->ship_class;
        if (cls == "Capital" || cls == "Carrier" || cls == "Dreadnought" || cls == "Titan") {
            return false;
        }
    }
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

std::tuple<float, float, float> StationHangarSystem::getSpawnPosition(
        const std::string& hangar_id) const {
    auto* entity = world_->getEntity(hangar_id);
    if (!entity) return {0.0f, 0.0f, 0.0f};

    auto* hangar = entity->getComponent<components::StationHangar>();
    if (!hangar) return {0.0f, 0.0f, 0.0f};

    return {hangar->spawn_x, hangar->spawn_y, hangar->spawn_z};
}

int StationHangarSystem::getStoredShipCount(const std::string& hangar_id) const {
    auto* entity = world_->getEntity(hangar_id);
    if (!entity) return 0;

    auto* hangar = entity->getComponent<components::StationHangar>();
    if (!hangar) return 0;

    return hangar->occupied_ship_slots;
}

double StationHangarSystem::getRentalBalance(const std::string& hangar_id) const {
    auto* entity = world_->getEntity(hangar_id);
    if (!entity) return 0.0;

    auto* hangar = entity->getComponent<components::StationHangar>();
    if (!hangar) return 0.0;

    return hangar->accumulated_rental;
}

} // namespace systems
} // namespace atlas
