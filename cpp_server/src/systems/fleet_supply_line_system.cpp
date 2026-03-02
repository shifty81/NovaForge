#include "systems/fleet_supply_line_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/fleet_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::FleetSupplyLine::SupplyDepot* findDepot(
    components::FleetSupplyLine* fsl, const std::string& depot_id) {
    for (auto& d : fsl->depots) {
        if (d.depot_id == depot_id) return &d;
    }
    return nullptr;
}
} // anonymous namespace

FleetSupplyLineSystem::FleetSupplyLineSystem(ecs::World* world)
    : System(world) {
}

void FleetSupplyLineSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::FleetSupplyLine>();
    for (auto* entity : entities) {
        auto* fsl = entity->getComponent<components::FleetSupplyLine>();
        if (!fsl || !fsl->active) continue;

        for (auto& depot : fsl->depots) {
            float drain = fsl->consumption_rate * delta_time;

            float actual_fuel = std::min(drain, depot.fuel_level);
            float actual_ammo = std::min(drain, depot.ammo_level);

            depot.fuel_level -= actual_fuel;
            depot.ammo_level -= actual_ammo;

            fsl->total_fuel_consumed += actual_fuel;
            fsl->total_ammo_consumed += actual_ammo;
        }
    }
}

bool FleetSupplyLineSystem::initializeSupplyLine(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetSupplyLine>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetSupplyLineSystem::addDepot(const std::string& entity_id,
    const std::string& depot_id, const std::string& system_id, float capacity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return false;
    if (static_cast<int>(fsl->depots.size()) >= fsl->max_depots) return false;
    if (findDepot(fsl, depot_id)) return false; // duplicate

    components::FleetSupplyLine::SupplyDepot depot;
    depot.depot_id = depot_id;
    depot.system_id = system_id;
    depot.capacity = capacity;
    depot.fuel_level = 0.0f;
    depot.ammo_level = 0.0f;
    fsl->depots.push_back(depot);
    return true;
}

bool FleetSupplyLineSystem::removeDepot(const std::string& entity_id,
    const std::string& depot_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return false;

    auto it = std::remove_if(fsl->depots.begin(), fsl->depots.end(),
        [&](const components::FleetSupplyLine::SupplyDepot& d) {
            return d.depot_id == depot_id;
        });
    if (it == fsl->depots.end()) return false;
    fsl->depots.erase(it, fsl->depots.end());
    return true;
}

bool FleetSupplyLineSystem::resupplyDepot(const std::string& entity_id,
    const std::string& depot_id, float fuel, float ammo) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return false;
    auto* depot = findDepot(fsl, depot_id);
    if (!depot) return false;

    float cap = std::min(100.0f, depot->capacity);
    depot->fuel_level = std::min(cap, depot->fuel_level + fuel);
    depot->ammo_level = std::min(cap, depot->ammo_level + ammo);
    fsl->total_resupplies++;
    return true;
}

bool FleetSupplyLineSystem::consumeSupplies(const std::string& entity_id,
    const std::string& depot_id, float fuel, float ammo) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return false;
    auto* depot = findDepot(fsl, depot_id);
    if (!depot) return false;

    depot->fuel_level = std::max(0.0f, depot->fuel_level - fuel);
    depot->ammo_level = std::max(0.0f, depot->ammo_level - ammo);
    return true;
}

int FleetSupplyLineSystem::getDepotCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return 0;
    return static_cast<int>(fsl->depots.size());
}

float FleetSupplyLineSystem::getFuelLevel(const std::string& entity_id,
    const std::string& depot_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return 0.0f;
    for (const auto& d : fsl->depots) {
        if (d.depot_id == depot_id) return d.fuel_level;
    }
    return 0.0f;
}

float FleetSupplyLineSystem::getAmmoLevel(const std::string& entity_id,
    const std::string& depot_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return 0.0f;
    for (const auto& d : fsl->depots) {
        if (d.depot_id == depot_id) return d.ammo_level;
    }
    return 0.0f;
}

int FleetSupplyLineSystem::getTotalResupplies(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return 0;
    return fsl->total_resupplies;
}

bool FleetSupplyLineSystem::isDepotCritical(const std::string& entity_id,
    const std::string& depot_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* fsl = entity->getComponent<components::FleetSupplyLine>();
    if (!fsl) return false;
    for (const auto& d : fsl->depots) {
        if (d.depot_id == depot_id) {
            return d.fuel_level < 20.0f || d.ammo_level < 20.0f;
        }
    }
    return false;
}

} // namespace systems
} // namespace atlas
