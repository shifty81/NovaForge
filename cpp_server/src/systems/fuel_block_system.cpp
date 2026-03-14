#include "systems/fuel_block_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FuelBlockSystem::FuelBlockSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick — consume fuel, check low-fuel, go offline on empty
// ---------------------------------------------------------------------------

void FuelBlockSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FuelBlockState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (!comp.is_online) return;

    bool any_empty = false;
    bool any_low   = false;

    for (auto& res : comp.reserves) {
        if (res.consumption_rate <= 0.0f) continue;
        float drain = res.consumption_rate * delta_time;
        if (drain > res.quantity) drain = res.quantity;
        res.quantity -= drain;
        comp.total_fuel_consumed += drain;

        if (res.quantity <= 0.0f) {
            res.quantity = 0.0f;
            any_empty = true;
        }

        // Low-fuel check: time remaining < threshold
        if (res.consumption_rate > 0.0f) {
            float time_left = res.quantity / res.consumption_rate;
            if (time_left < comp.low_fuel_threshold) {
                any_low = true;
            }
        }
    }

    comp.low_fuel_warning = any_low;

    if (any_empty) {
        comp.is_online = false;
        comp.total_offline_events++;
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool FuelBlockSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FuelBlockState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Reserve management
// ---------------------------------------------------------------------------

bool FuelBlockSystem::addReserve(
        const std::string& entity_id,
        const std::string& reserve_id,
        components::FuelBlockState::FuelType fuel_type,
        float quantity,
        float max_quantity,
        float consumption_rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (reserve_id.empty()) return false;
    if (max_quantity <= 0.0f) return false;
    if (quantity < 0.0f) return false;
    if (quantity > max_quantity) return false;
    if (consumption_rate < 0.0f) return false;

    // Duplicate check
    for (const auto& r : comp->reserves) {
        if (r.reserve_id == reserve_id) return false;
    }

    // Capacity check
    if (static_cast<int>(comp->reserves.size()) >= comp->max_reserves)
        return false;

    components::FuelBlockState::FuelReserve r;
    r.reserve_id       = reserve_id;
    r.fuel_type        = fuel_type;
    r.quantity          = quantity;
    r.max_quantity      = max_quantity;
    r.consumption_rate  = consumption_rate;
    comp->reserves.push_back(r);
    return true;
}

bool FuelBlockSystem::removeReserve(const std::string& entity_id,
                                     const std::string& reserve_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->reserves.begin(), comp->reserves.end(),
        [&](const components::FuelBlockState::FuelReserve& r) {
            return r.reserve_id == reserve_id;
        });
    if (it == comp->reserves.end()) return false;
    comp->reserves.erase(it);
    return true;
}

bool FuelBlockSystem::clearReserves(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->reserves.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Operations
// ---------------------------------------------------------------------------

bool FuelBlockSystem::refuel(const std::string& entity_id,
                              const std::string& reserve_id,
                              float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount <= 0.0f) return false;

    for (auto& r : comp->reserves) {
        if (r.reserve_id == reserve_id) {
            r.quantity = std::min(r.quantity + amount, r.max_quantity);
            comp->total_refuels++;
            return true;
        }
    }
    return false;
}

bool FuelBlockSystem::setConsumptionRate(const std::string& entity_id,
                                          const std::string& reserve_id,
                                          float rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0f) return false;

    for (auto& r : comp->reserves) {
        if (r.reserve_id == reserve_id) {
            r.consumption_rate = rate;
            return true;
        }
    }
    return false;
}

bool FuelBlockSystem::bringOnline(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->is_online) return false; // already online

    // Require all reserves to have fuel > 0
    for (const auto& r : comp->reserves) {
        if (r.consumption_rate > 0.0f && r.quantity <= 0.0f) return false;
    }

    comp->is_online = true;
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool FuelBlockSystem::setStructureId(const std::string& entity_id,
                                      const std::string& structure_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->structure_id = structure_id;
    return true;
}

bool FuelBlockSystem::setLowFuelThreshold(const std::string& entity_id,
                                           float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold < 0.0f) return false;
    comp->low_fuel_threshold = threshold;
    return true;
}

bool FuelBlockSystem::setMaxReserves(const std::string& entity_id,
                                      int max_reserves) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_reserves <= 0) return false;
    comp->max_reserves = max_reserves;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int FuelBlockSystem::getReserveCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->reserves.size()) : 0;
}

bool FuelBlockSystem::hasReserve(const std::string& entity_id,
                                  const std::string& reserve_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& r : comp->reserves) {
        if (r.reserve_id == reserve_id) return true;
    }
    return false;
}

float FuelBlockSystem::getFuelQuantity(const std::string& entity_id,
                                        const std::string& reserve_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->reserves) {
        if (r.reserve_id == reserve_id) return r.quantity;
    }
    return 0.0f;
}

float FuelBlockSystem::getFuelCapacity(const std::string& entity_id,
                                        const std::string& reserve_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->reserves) {
        if (r.reserve_id == reserve_id) return r.max_quantity;
    }
    return 0.0f;
}

float FuelBlockSystem::getConsumptionRate(const std::string& entity_id,
                                           const std::string& reserve_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->reserves) {
        if (r.reserve_id == reserve_id) return r.consumption_rate;
    }
    return 0.0f;
}

bool FuelBlockSystem::isOnline(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->is_online;
}

bool FuelBlockSystem::isLowFuel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->low_fuel_warning;
}

std::string FuelBlockSystem::getStructureId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->structure_id : "";
}

int FuelBlockSystem::getTotalRefuels(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_refuels : 0;
}

float FuelBlockSystem::getTotalFuelConsumed(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_fuel_consumed : 0.0f;
}

int FuelBlockSystem::getTotalOfflineEvents(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_offline_events : 0;
}

float FuelBlockSystem::getTimeUntilEmpty(
        const std::string& entity_id,
        const std::string& reserve_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& r : comp->reserves) {
        if (r.reserve_id == reserve_id) {
            if (r.consumption_rate <= 0.0f) return -1.0f; // infinite
            return r.quantity / r.consumption_rate;
        }
    }
    return 0.0f;
}

} // namespace systems
} // namespace atlas
