#ifndef NOVAFORGE_SYSTEMS_TRADE_ROUTE_OPTIMIZER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TRADE_ROUTE_OPTIMIZER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Multi-hop trade route calculation with profit optimization
 *
 * Stores station market snapshots, finds the most profitable
 * buy/sell pairs across the network, and orders them into an
 * optimized route.  Supports cargo capacity constraints and
 * travel-time weighting.  Essential for the trade loop in the
 * vertical slice.
 */
class TradeRouteOptimizerSystem : public ecs::SingleComponentSystem<components::TradeRouteOptimizerState> {
public:
    explicit TradeRouteOptimizerSystem(ecs::World* world);
    ~TradeRouteOptimizerSystem() override = default;

    std::string getName() const override { return "TradeRouteOptimizerSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, int cargo_capacity);

    // Market data management
    bool addMarketEntry(const std::string& entity_id, const std::string& station_id,
                        const std::string& commodity, float buy_price, float sell_price,
                        int supply, int demand);
    bool removeMarketEntry(const std::string& entity_id, const std::string& station_id,
                           const std::string& commodity);
    bool clearMarketData(const std::string& entity_id);

    // Route calculation
    bool calculateOptimalRoute(const std::string& entity_id, float travel_time_per_hop);
    bool clearRoute(const std::string& entity_id);

    // Queries
    int getMarketEntryCount(const std::string& entity_id) const;
    int getRouteHopCount(const std::string& entity_id) const;
    float getTotalEstimatedProfit(const std::string& entity_id) const;
    float getTotalTravelTime(const std::string& entity_id) const;
    int getCargoCapacity(const std::string& entity_id) const;
    int getRoutesCalculated(const std::string& entity_id) const;
    float getBestProfitPerUnit(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::TradeRouteOptimizerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TRADE_ROUTE_OPTIMIZER_SYSTEM_H
