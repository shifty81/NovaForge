#include "systems/trade_route_optimizer_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

TradeRouteOptimizerSystem::TradeRouteOptimizerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void TradeRouteOptimizerSystem::updateComponent(ecs::Entity& /*entity*/,
    components::TradeRouteOptimizerState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool TradeRouteOptimizerSystem::initialize(const std::string& entity_id,
    int cargo_capacity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (cargo_capacity <= 0) return false;

    auto comp = std::make_unique<components::TradeRouteOptimizerState>();
    comp->cargo_capacity = cargo_capacity;
    entity->addComponent(std::move(comp));
    return true;
}

bool TradeRouteOptimizerSystem::addMarketEntry(const std::string& entity_id,
    const std::string& station_id, const std::string& commodity,
    float buy_price, float sell_price, int supply, int demand) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (station_id.empty() || commodity.empty()) return false;
    if (buy_price < 0.0f || sell_price < 0.0f) return false;
    if (supply < 0 || demand < 0) return false;

    // Check for duplicate (same station + commodity)
    for (const auto& e : comp->market_data) {
        if (e.station_id == station_id && e.commodity == commodity) return false;
    }

    if (static_cast<int>(comp->market_data.size()) >= comp->max_market_entries) return false;

    components::TradeRouteOptimizerState::MarketEntry entry;
    entry.station_id = station_id;
    entry.commodity = commodity;
    entry.buy_price = buy_price;
    entry.sell_price = sell_price;
    entry.supply = supply;
    entry.demand = demand;
    comp->market_data.push_back(entry);
    return true;
}

bool TradeRouteOptimizerSystem::removeMarketEntry(const std::string& entity_id,
    const std::string& station_id, const std::string& commodity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->market_data.begin(), comp->market_data.end(),
        [&](const components::TradeRouteOptimizerState::MarketEntry& e) {
            return e.station_id == station_id && e.commodity == commodity;
        });
    if (it == comp->market_data.end()) return false;

    comp->market_data.erase(it);
    return true;
}

bool TradeRouteOptimizerSystem::clearMarketData(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->market_data.clear();
    return true;
}

bool TradeRouteOptimizerSystem::calculateOptimalRoute(const std::string& entity_id,
    float travel_time_per_hop) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (travel_time_per_hop <= 0.0f) return false;
    if (comp->market_data.empty()) return false;

    // Find all profitable trade pairs: buy at station A, sell at station B
    struct TradePair {
        std::string from_station;
        std::string to_station;
        std::string commodity;
        float buy_price;
        float sell_price;
        float profit_per_unit;
    };

    std::vector<TradePair> pairs;

    for (size_t i = 0; i < comp->market_data.size(); ++i) {
        for (size_t j = 0; j < comp->market_data.size(); ++j) {
            if (i == j) continue;
            const auto& buy_entry = comp->market_data[i];
            const auto& sell_entry = comp->market_data[j];

            // Same commodity, different stations, profitable
            if (buy_entry.commodity == sell_entry.commodity &&
                buy_entry.station_id != sell_entry.station_id &&
                sell_entry.sell_price > buy_entry.buy_price &&
                buy_entry.supply > 0 && sell_entry.demand > 0) {

                TradePair pair;
                pair.from_station = buy_entry.station_id;
                pair.to_station = sell_entry.station_id;
                pair.commodity = buy_entry.commodity;
                pair.buy_price = buy_entry.buy_price;
                pair.sell_price = sell_entry.sell_price;
                pair.profit_per_unit = sell_entry.sell_price - buy_entry.buy_price;
                pairs.push_back(pair);
            }
        }
    }

    if (pairs.empty()) return false;

    // Sort by profit per unit descending
    std::sort(pairs.begin(), pairs.end(),
        [](const TradePair& a, const TradePair& b) {
            return a.profit_per_unit > b.profit_per_unit;
        });

    // Build route from top profitable pairs (up to max_hops)
    comp->optimized_route.clear();
    comp->total_estimated_profit = 0.0f;
    comp->total_travel_time = 0.0f;

    int hops = std::min(static_cast<int>(pairs.size()), comp->max_hops);
    for (int i = 0; i < hops; ++i) {
        components::TradeRouteOptimizerState::TradeHop hop;
        hop.from_station = pairs[i].from_station;
        hop.to_station = pairs[i].to_station;
        hop.commodity = pairs[i].commodity;
        hop.buy_price = pairs[i].buy_price;
        hop.sell_price = pairs[i].sell_price;
        hop.profit_per_unit = pairs[i].profit_per_unit;
        hop.travel_time = travel_time_per_hop;
        comp->optimized_route.push_back(hop);

        comp->total_estimated_profit += hop.profit_per_unit * comp->cargo_capacity;
        comp->total_travel_time += travel_time_per_hop;
    }

    comp->routes_calculated++;
    return true;
}

bool TradeRouteOptimizerSystem::clearRoute(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->optimized_route.clear();
    comp->total_estimated_profit = 0.0f;
    comp->total_travel_time = 0.0f;
    return true;
}

int TradeRouteOptimizerSystem::getMarketEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->market_data.size()) : 0;
}

int TradeRouteOptimizerSystem::getRouteHopCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->optimized_route.size()) : 0;
}

float TradeRouteOptimizerSystem::getTotalEstimatedProfit(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_estimated_profit : 0.0f;
}

float TradeRouteOptimizerSystem::getTotalTravelTime(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_travel_time : 0.0f;
}

int TradeRouteOptimizerSystem::getCargoCapacity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cargo_capacity : 0;
}

int TradeRouteOptimizerSystem::getRoutesCalculated(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->routes_calculated : 0;
}

float TradeRouteOptimizerSystem::getBestProfitPerUnit(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->optimized_route.empty()) return 0.0f;
    return comp->optimized_route[0].profit_per_unit;
}

} // namespace systems
} // namespace atlas
