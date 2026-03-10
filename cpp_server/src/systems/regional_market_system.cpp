#include "systems/regional_market_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

RegionalMarketSystem::RegionalMarketSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void RegionalMarketSystem::updateComponent(ecs::Entity& entity,
    components::RegionalMarketState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    comp.price_update_timer += delta_time;

    if (comp.price_update_timer >= comp.price_update_interval) {
        comp.price_update_timer = 0.0f;
        comp.total_updates++;

        // Adjust prices based on supply/demand ratio
        for (auto& p : comp.prices) {
            float ratio = 1.0f;
            if (p.supply > 0.0f) {
                ratio = p.demand / p.supply;
            }
            // Price moves toward equilibrium
            float adjustment = (ratio - 1.0f) * comp.volatility_factor;
            p.average_price *= (1.0f + adjustment);
            // Apply hub multiplier
            float effective = p.average_price * p.hub_multiplier;
            p.min_price = effective * 0.9f;
            p.max_price = effective * 1.1f;
        }
    }
}

bool RegionalMarketSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::RegionalMarketState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool RegionalMarketSystem::addRegionPrice(const std::string& entity_id,
    const std::string& region_id, const std::string& item_type,
    float average_price, float supply, float demand, float hub_multiplier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->prices.size()) >= comp->max_tracked_items) return false;

    components::RegionalMarketState::RegionPrice rp;
    rp.region_id = region_id;
    rp.item_type = item_type;
    rp.average_price = average_price;
    rp.supply = supply;
    rp.demand = demand;
    rp.hub_multiplier = hub_multiplier;
    rp.min_price = average_price * hub_multiplier * 0.9f;
    rp.max_price = average_price * hub_multiplier * 1.1f;
    comp->prices.push_back(rp);
    return true;
}

bool RegionalMarketSystem::recordTrade(const std::string& entity_id,
    const std::string& region_id, const std::string& item_type,
    int volume, float price) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (auto& p : comp->prices) {
        if (p.region_id == region_id && p.item_type == item_type) {
            p.trade_volume += volume;
            // Weighted average with new trade
            float total = static_cast<float>(p.trade_volume);
            if (total > 0.0f) {
                p.average_price = ((p.average_price * (total - volume)) + (price * volume)) / total;
            }
            return true;
        }
    }
    return false;
}

bool RegionalMarketSystem::setVolatilityFactor(const std::string& entity_id,
    float factor) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->volatility_factor = factor;
    return true;
}

int RegionalMarketSystem::getTrackedItemCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->prices.size()) : 0;
}

int RegionalMarketSystem::getTotalUpdates(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_updates : 0;
}

float RegionalMarketSystem::getAveragePrice(const std::string& entity_id,
    const std::string& region_id, const std::string& item_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& p : comp->prices) {
        if (p.region_id == region_id && p.item_type == item_type) return p.average_price;
    }
    return 0.0f;
}

float RegionalMarketSystem::getHubMultiplier(const std::string& entity_id,
    const std::string& region_id, const std::string& item_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& p : comp->prices) {
        if (p.region_id == region_id && p.item_type == item_type) return p.hub_multiplier;
    }
    return 0.0f;
}

int RegionalMarketSystem::getTradeVolume(const std::string& entity_id,
    const std::string& region_id, const std::string& item_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& p : comp->prices) {
        if (p.region_id == region_id && p.item_type == item_type) return p.trade_volume;
    }
    return 0;
}

float RegionalMarketSystem::getVolatilityFactor(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->volatility_factor : 0.0f;
}

} // namespace systems
} // namespace atlas
