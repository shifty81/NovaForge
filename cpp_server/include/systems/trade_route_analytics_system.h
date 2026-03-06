#ifndef NOVAFORGE_SYSTEMS_TRADE_ROUTE_ANALYTICS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TRADE_ROUTE_ANALYTICS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Trade route performance analytics system
 *
 * Tracks trade route metrics including volume, revenue, profit margins,
 * congestion, and periodic snapshots for supply/demand trend analysis.
 * Enables visible AI economic cycles by aggregating NPC trade activity.
 */
class TradeRouteAnalyticsSystem : public ecs::SingleComponentSystem<components::TradeRouteAnalytics> {
public:
    explicit TradeRouteAnalyticsSystem(ecs::World* world);
    ~TradeRouteAnalyticsSystem() override = default;

    std::string getName() const override { return "TradeRouteAnalyticsSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool registerRoute(const std::string& entity_id, const std::string& route_id,
                       const std::string& origin, const std::string& dest,
                       const std::string& commodity);
    bool recordTrip(const std::string& entity_id, const std::string& route_id,
                    float volume, float revenue, float cost);
    bool updateCongestion(const std::string& entity_id, const std::string& route_id,
                          float congestion);
    bool removeRoute(const std::string& entity_id, const std::string& route_id);
    int getRouteCount(const std::string& entity_id) const;
    float getRouteVolume(const std::string& entity_id, const std::string& route_id) const;
    float getRouteRevenue(const std::string& entity_id, const std::string& route_id) const;
    float getRouteProfitMargin(const std::string& entity_id, const std::string& route_id) const;
    std::string getMostProfitableRoute(const std::string& entity_id) const;
    float getTotalVolume(const std::string& entity_id) const;
    float getTotalRevenue(const std::string& entity_id) const;
    int getTotalSnapshots(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::TradeRouteAnalytics& tra, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TRADE_ROUTE_ANALYTICS_SYSTEM_H
