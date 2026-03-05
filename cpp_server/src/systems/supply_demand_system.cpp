#include "systems/supply_demand_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

SupplyDemandSystem::SupplyDemandSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void SupplyDemandSystem::updateComponent(ecs::Entity& /*entity*/, components::SupplyDemand& sd, float delta_time) {
    for (auto& c : sd.commodities) {
        // Natural supply decay
        c.supply -= sd.supply_decay_rate * delta_time;

        // NPC supply production
        c.supply += c.supply_rate * sd.npc_activity_modifier * delta_time;

        // Drift demand toward baseline (100)
        c.demand += (100.0f - c.demand) * sd.demand_drift_rate * delta_time;

        // Compute price from supply/demand ratio
        float ratio = c.demand / std::max(c.supply, 0.01f);
        c.current_price = c.base_price * (1.0f + (ratio - 1.0f) * sd.price_elasticity);

        // Clamp price
        float floor = c.base_price * sd.price_floor_multiplier;
        float ceiling = c.base_price * sd.price_ceiling_multiplier;
        c.current_price = std::clamp(c.current_price, floor, ceiling);

        // Clamp supply
        c.supply = std::max(c.supply, 0.0f);
    }
}

float SupplyDemandSystem::getPrice(const std::string& system_id, const std::string& commodity_id) const {
    const auto* sd = getComponentFor(system_id);
    if (!sd) return 0.0f;
    const auto* c = sd->getCommodity(commodity_id);
    if (c) return c->current_price;
    return 0.0f;
}

float SupplyDemandSystem::getSupply(const std::string& system_id, const std::string& commodity_id) const {
    const auto* sd = getComponentFor(system_id);
    if (!sd) return 0.0f;
    const auto* c = sd->getCommodity(commodity_id);
    if (c) return c->supply;
    return 0.0f;
}

float SupplyDemandSystem::getDemand(const std::string& system_id, const std::string& commodity_id) const {
    const auto* sd = getComponentFor(system_id);
    if (!sd) return 0.0f;
    const auto* c = sd->getCommodity(commodity_id);
    if (c) return c->demand;
    return 0.0f;
}

void SupplyDemandSystem::addSupply(const std::string& system_id, const std::string& commodity_id, float amount) {
    auto* sd = getComponentFor(system_id);
    if (!sd) return;
    auto* c = sd->getCommodity(commodity_id);
    if (c) c->supply += amount;
}

void SupplyDemandSystem::addDemand(const std::string& system_id, const std::string& commodity_id, float amount) {
    auto* sd = getComponentFor(system_id);
    if (!sd) return;
    auto* c = sd->getCommodity(commodity_id);
    if (c) c->demand += amount;
}

void SupplyDemandSystem::setNPCActivityModifier(const std::string& system_id, float modifier) {
    auto* sd = getComponentFor(system_id);
    if (sd) sd->npc_activity_modifier = modifier;
}

} // namespace systems
} // namespace atlas
