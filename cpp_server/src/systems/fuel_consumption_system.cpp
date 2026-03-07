#include "systems/fuel_consumption_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using FT = components::FuelTank;

const char* fuelTypeToString(FT::FuelType t) {
    switch (t) {
        case FT::FuelType::Standard:     return "Standard";
        case FT::FuelType::HighGrade:    return "HighGrade";
        case FT::FuelType::Experimental: return "Experimental";
    }
    return "Unknown";
}

FT::FuelType stringToFuelType(const std::string& s) {
    if (s == "HighGrade") return FT::FuelType::HighGrade;
    if (s == "Experimental") return FT::FuelType::Experimental;
    return FT::FuelType::Standard;
}

} // anonymous namespace

FuelConsumptionSystem::FuelConsumptionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FuelConsumptionSystem::updateComponent(ecs::Entity& entity,
    components::FuelTank& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    double consumption = comp.idle_consumption_rate * delta_time;
    if (comp.warping) {
        consumption += comp.warp_consumption_rate * delta_time;
    }
    if (comp.thrusting) {
        consumption += comp.thrust_consumption_rate * delta_time;
    }

    double actual = std::min(consumption, comp.current_fuel);
    comp.current_fuel -= actual;
    comp.total_fuel_consumed += actual;

    // Clamp to zero
    if (comp.current_fuel < 0.0) comp.current_fuel = 0.0;
}

bool FuelConsumptionSystem::initialize(const std::string& entity_id,
    const std::string& ship_id, double max_fuel) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FuelTank>();
    comp->ship_id = ship_id;
    comp->max_fuel = max_fuel;
    comp->current_fuel = max_fuel;
    entity->addComponent(std::move(comp));
    return true;
}

bool FuelConsumptionSystem::setFuelType(const std::string& entity_id,
    const std::string& fuel_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->fuel_type = stringToFuelType(fuel_type);
    return true;
}

bool FuelConsumptionSystem::setWarpState(const std::string& entity_id,
    bool warping) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->warping = warping;
    return true;
}

bool FuelConsumptionSystem::setThrustState(const std::string& entity_id,
    bool thrusting) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->thrusting = thrusting;
    return true;
}

bool FuelConsumptionSystem::refuel(const std::string& entity_id,
    double amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0.0) return false;
    double added = std::min(amount, comp->max_fuel - comp->current_fuel);
    comp->current_fuel += added;
    comp->total_fuel_purchased += added;
    comp->refuel_count++;
    return true;
}

bool FuelConsumptionSystem::setConsumptionRates(const std::string& entity_id,
    double warp_rate, double thrust_rate, double idle_rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->warp_consumption_rate = warp_rate;
    comp->thrust_consumption_rate = thrust_rate;
    comp->idle_consumption_rate = idle_rate;
    return true;
}

double FuelConsumptionSystem::getCurrentFuel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_fuel : 0.0;
}

double FuelConsumptionSystem::getMaxFuel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_fuel : 0.0;
}

double FuelConsumptionSystem::getFuelPercentage(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->max_fuel <= 0.0) return 0.0;
    return (comp->current_fuel / comp->max_fuel) * 100.0;
}

std::string FuelConsumptionSystem::getFuelType(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    return fuelTypeToString(comp->fuel_type);
}

bool FuelConsumptionSystem::canWarp(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->current_fuel >= comp->warp_consumption_rate;
}

double FuelConsumptionSystem::getTotalFuelConsumed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_fuel_consumed : 0.0;
}

double FuelConsumptionSystem::getTotalFuelPurchased(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_fuel_purchased : 0.0;
}

int FuelConsumptionSystem::getRefuelCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->refuel_count : 0;
}

} // namespace systems
} // namespace atlas
