#include "systems/outer_rim_logistics_distortion_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::OuterRimLogisticsDistortion::RouteDistortion* findRoute(
    components::OuterRimLogisticsDistortion* ord, const std::string& route_id) {
    for (auto& r : ord->routes) {
        if (r.route_id == route_id) return &r;
    }
    return nullptr;
}
} // anonymous namespace

OuterRimLogisticsDistortionSystem::OuterRimLogisticsDistortionSystem(ecs::World* world)
    : System(world) {
}

void OuterRimLogisticsDistortionSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::OuterRimLogisticsDistortion>();
    for (auto* entity : entities) {
        auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
        if (!ord || !ord->active) continue;

        int disrupted = 0;
        float total_price = 0.0f;

        for (auto& route : ord->routes) {
            float effective_threat = std::max(route.threat_level, ord->global_threat);
            float target_efficiency = 1.0f - (effective_threat * ord->distortion_factor);
            target_efficiency = std::max(0.0f, std::min(1.0f, target_efficiency));

            if (route.efficiency > target_efficiency) {
                // Distorting down — instant
                route.efficiency = target_efficiency;
            } else if (route.efficiency < target_efficiency) {
                // Recovering — gradual
                route.efficiency = std::min(target_efficiency,
                    route.efficiency + ord->recovery_rate * delta_time);
            }

            route.price_impact = (1.0f - route.efficiency) * 2.0f;

            if (route.efficiency < 0.8f) {
                disrupted++;
            }
            total_price += route.price_impact;
        }

        ord->disrupted_route_count = disrupted;
        ord->total_price_impact = total_price;
    }
}

bool OuterRimLogisticsDistortionSystem::initializeRegion(const std::string& entity_id,
    const std::string& region_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::OuterRimLogisticsDistortion>();
    comp->region_id = region_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool OuterRimLogisticsDistortionSystem::addRoute(const std::string& entity_id,
    const std::string& route_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return false;
    if (static_cast<int>(ord->routes.size()) >= ord->max_routes) return false;
    if (findRoute(ord, route_id)) return false;

    components::OuterRimLogisticsDistortion::RouteDistortion route;
    route.route_id = route_id;
    ord->routes.push_back(route);
    return true;
}

bool OuterRimLogisticsDistortionSystem::setGlobalThreat(const std::string& entity_id, float threat) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return false;
    ord->global_threat = std::max(0.0f, std::min(1.0f, threat));
    return true;
}

bool OuterRimLogisticsDistortionSystem::setRouteThreat(const std::string& entity_id,
    const std::string& route_id, float threat) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return false;
    auto* route = findRoute(ord, route_id);
    if (!route) return false;
    route->threat_level = std::max(0.0f, std::min(1.0f, threat));
    return true;
}

float OuterRimLogisticsDistortionSystem::getRouteEfficiency(const std::string& entity_id,
    const std::string& route_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return 0.0f;
    for (const auto& r : ord->routes) {
        if (r.route_id == route_id) return r.efficiency;
    }
    return 0.0f;
}

float OuterRimLogisticsDistortionSystem::getRoutePriceImpact(const std::string& entity_id,
    const std::string& route_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return 0.0f;
    for (const auto& r : ord->routes) {
        if (r.route_id == route_id) return r.price_impact;
    }
    return 0.0f;
}

float OuterRimLogisticsDistortionSystem::getGlobalThreat(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return 0.0f;
    return ord->global_threat;
}

int OuterRimLogisticsDistortionSystem::getDisruptedRouteCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return 0;
    return ord->disrupted_route_count;
}

float OuterRimLogisticsDistortionSystem::getTotalPriceImpact(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return 0.0f;
    return ord->total_price_impact;
}

int OuterRimLogisticsDistortionSystem::getRouteCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ord = entity->getComponent<components::OuterRimLogisticsDistortion>();
    if (!ord) return 0;
    return static_cast<int>(ord->routes.size());
}

} // namespace systems
} // namespace atlas
