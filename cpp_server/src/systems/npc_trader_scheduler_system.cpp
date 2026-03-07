#include "systems/npc_trader_scheduler_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using NTS = components::NpcTraderSchedule;

const char* routeStateToString(NTS::RouteState s) {
    switch (s) {
        case NTS::RouteState::Waiting:   return "Waiting";
        case NTS::RouteState::Loading:   return "Loading";
        case NTS::RouteState::InTransit: return "InTransit";
        case NTS::RouteState::Unloading: return "Unloading";
        case NTS::RouteState::Complete:  return "Complete";
    }
    return "Unknown";
}

} // anonymous namespace

NpcTraderSchedulerSystem::NpcTraderSchedulerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void NpcTraderSchedulerSystem::updateComponent(ecs::Entity& entity,
    components::NpcTraderSchedule& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Spawn timer – activate waiting routes when timer fires
    comp.spawn_timer += delta_time;
    if (comp.spawn_timer >= comp.spawn_interval &&
        comp.active_haulers < comp.max_haulers) {
        for (auto& route : comp.routes) {
            if (route.state == NTS::RouteState::Waiting &&
                comp.active_haulers < comp.max_haulers) {
                route.state = NTS::RouteState::Loading;
                route.load_progress = 0.0f;
                comp.active_haulers++;
            }
        }
        comp.spawn_timer = 0.0f;
    }

    // Progress each active route through phases
    for (auto& route : comp.routes) {
        switch (route.state) {
            case NTS::RouteState::Loading:
                route.load_progress += delta_time / route.load_time;
                if (route.load_progress >= 1.0f) {
                    route.load_progress = 1.0f;
                    route.state = NTS::RouteState::InTransit;
                    route.progress = 0.0f;
                }
                break;

            case NTS::RouteState::InTransit:
                route.progress += delta_time / route.travel_time;
                if (route.progress >= 1.0f) {
                    route.progress = 1.0f;
                    route.state = NTS::RouteState::Unloading;
                    route.load_progress = 0.0f;
                }
                break;

            case NTS::RouteState::Unloading:
                route.load_progress += delta_time / route.load_time;
                if (route.load_progress >= 1.0f) {
                    route.load_progress = 1.0f;
                    route.state = NTS::RouteState::Complete;
                    comp.total_deliveries++;
                    comp.total_cargo_value_delivered += route.cargo_value;
                    comp.active_haulers = std::max(0, comp.active_haulers - 1);
                }
                break;

            case NTS::RouteState::Complete:
                // Reset for next cycle
                route.state = NTS::RouteState::Waiting;
                route.progress = 0.0f;
                route.load_progress = 0.0f;
                break;

            default:
                break;
        }
    }
}

bool NpcTraderSchedulerSystem::initialize(const std::string& entity_id,
    const std::string& scheduler_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::NpcTraderSchedule>();
    comp->scheduler_id = scheduler_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool NpcTraderSchedulerSystem::addRoute(const std::string& entity_id,
    const std::string& route_id, const std::string& origin,
    const std::string& destination, const std::string& cargo_type,
    float cargo_volume, double cargo_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    NTS::TradeRoute route;
    route.route_id = route_id;
    route.origin_station = origin;
    route.destination_station = destination;
    route.cargo_type = cargo_type;
    route.cargo_volume = cargo_volume;
    route.cargo_value = cargo_value;
    comp->routes.push_back(route);
    return true;
}

bool NpcTraderSchedulerSystem::setMaxHaulers(const std::string& entity_id,
    int max_haulers) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->max_haulers = max_haulers;
    return true;
}

bool NpcTraderSchedulerSystem::setSpawnInterval(const std::string& entity_id,
    float interval) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->spawn_interval = interval;
    return true;
}

int NpcTraderSchedulerSystem::getActiveHaulers(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active_haulers : 0;
}

int NpcTraderSchedulerSystem::getRouteCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->routes.size()) : 0;
}

int NpcTraderSchedulerSystem::getTotalDeliveries(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_deliveries : 0;
}

double NpcTraderSchedulerSystem::getTotalCargoValueDelivered(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_cargo_value_delivered : 0.0;
}

std::string NpcTraderSchedulerSystem::getRouteState(const std::string& entity_id,
    const std::string& route_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    for (const auto& r : comp->routes) {
        if (r.route_id == route_id) return routeStateToString(r.state);
    }
    return "Unknown";
}

} // namespace systems
} // namespace atlas
