#include "systems/trade_route_analytics_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using TRA = components::TradeRouteAnalytics;
using Route = components::TradeRouteAnalytics::TradeRoute;

Route* findRoute(TRA* tra, const std::string& route_id) {
    for (auto& r : tra->routes) {
        if (r.route_id == route_id) return &r;
    }
    return nullptr;
}

const Route* findRouteConst(const TRA* tra, const std::string& route_id) {
    for (const auto& r : tra->routes) {
        if (r.route_id == route_id) return &r;
    }
    return nullptr;
}

} // anonymous namespace

TradeRouteAnalyticsSystem::TradeRouteAnalyticsSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void TradeRouteAnalyticsSystem::updateComponent(ecs::Entity& entity,
    components::TradeRouteAnalytics& tra, float delta_time) {
    if (!tra.active) return;

    tra.snapshot_timer += delta_time;
    if (tra.snapshot_timer >= tra.snapshot_interval) {
        tra.snapshot_timer -= tra.snapshot_interval;
        tra.total_snapshots++;
    }
}

bool TradeRouteAnalyticsSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::TradeRouteAnalytics>();
    entity->addComponent(std::move(comp));
    return true;
}

bool TradeRouteAnalyticsSystem::registerRoute(const std::string& entity_id,
    const std::string& route_id, const std::string& origin,
    const std::string& dest, const std::string& commodity) {
    auto* tra = getComponentFor(entity_id);
    if (!tra) return false;
    if (static_cast<int>(tra->routes.size()) >= tra->max_routes) return false;
    if (findRoute(tra, route_id)) return false;

    Route r;
    r.route_id = route_id;
    r.origin_system = origin;
    r.dest_system = dest;
    r.commodity = commodity;
    tra->routes.push_back(r);
    return true;
}

bool TradeRouteAnalyticsSystem::recordTrip(const std::string& entity_id,
    const std::string& route_id, float volume, float revenue, float cost) {
    auto* tra = getComponentFor(entity_id);
    if (!tra) return false;

    auto* route = findRoute(tra, route_id);
    if (!route) return false;

    route->volume += volume;
    route->revenue += revenue;
    route->cost += cost;
    route->trips++;
    tra->total_volume += volume;
    tra->total_revenue += revenue;
    return true;
}

bool TradeRouteAnalyticsSystem::updateCongestion(const std::string& entity_id,
    const std::string& route_id, float congestion) {
    auto* tra = getComponentFor(entity_id);
    if (!tra) return false;

    auto* route = findRoute(tra, route_id);
    if (!route) return false;

    route->congestion = std::max(0.0f, std::min(1.0f, congestion));
    return true;
}

bool TradeRouteAnalyticsSystem::removeRoute(const std::string& entity_id,
    const std::string& route_id) {
    auto* tra = getComponentFor(entity_id);
    if (!tra) return false;

    auto it = std::find_if(tra->routes.begin(), tra->routes.end(),
        [&](const Route& r) { return r.route_id == route_id; });
    if (it == tra->routes.end()) return false;
    tra->routes.erase(it);
    return true;
}

int TradeRouteAnalyticsSystem::getRouteCount(const std::string& entity_id) const {
    auto* tra = getComponentFor(entity_id);
    return tra ? static_cast<int>(tra->routes.size()) : 0;
}

float TradeRouteAnalyticsSystem::getRouteVolume(const std::string& entity_id,
    const std::string& route_id) const {
    auto* tra = getComponentFor(entity_id);
    if (!tra) return 0.0f;
    const auto* route = findRouteConst(tra, route_id);
    return route ? route->volume : 0.0f;
}

float TradeRouteAnalyticsSystem::getRouteRevenue(const std::string& entity_id,
    const std::string& route_id) const {
    auto* tra = getComponentFor(entity_id);
    if (!tra) return 0.0f;
    const auto* route = findRouteConst(tra, route_id);
    return route ? route->revenue : 0.0f;
}

float TradeRouteAnalyticsSystem::getRouteProfitMargin(const std::string& entity_id,
    const std::string& route_id) const {
    auto* tra = getComponentFor(entity_id);
    if (!tra) return 0.0f;
    const auto* route = findRouteConst(tra, route_id);
    if (!route || route->revenue <= 0.0f) return 0.0f;
    return (route->revenue - route->cost) / route->revenue;
}

std::string TradeRouteAnalyticsSystem::getMostProfitableRoute(
    const std::string& entity_id) const {
    auto* tra = getComponentFor(entity_id);
    if (!tra || tra->routes.empty()) return "";

    const Route* best = nullptr;
    float best_profit = -1e30f;
    for (const auto& r : tra->routes) {
        float profit = r.revenue - r.cost;
        if (profit > best_profit) {
            best_profit = profit;
            best = &r;
        }
    }
    return best ? best->route_id : "";
}

float TradeRouteAnalyticsSystem::getTotalVolume(const std::string& entity_id) const {
    auto* tra = getComponentFor(entity_id);
    return tra ? tra->total_volume : 0.0f;
}

float TradeRouteAnalyticsSystem::getTotalRevenue(const std::string& entity_id) const {
    auto* tra = getComponentFor(entity_id);
    return tra ? tra->total_revenue : 0.0f;
}

int TradeRouteAnalyticsSystem::getTotalSnapshots(const std::string& entity_id) const {
    auto* tra = getComponentFor(entity_id);
    return tra ? tra->total_snapshots : 0;
}

} // namespace systems
} // namespace atlas
