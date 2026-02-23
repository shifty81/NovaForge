#include "systems/npc_rerouting_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

NPCReroutingSystem::NPCReroutingSystem(ecs::World* world)
    : System(world) {
}

void NPCReroutingSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::NPCRouteState>();
    for (auto* entity : entities) {
        auto* route = entity->getComponent<components::NPCRouteState>();
        if (!route) continue;

        route->reroute_cooldown -= delta_time;

        if (route->reroute_cooldown <= 0.0f && !route->rerouting) {
            // Check for dangerous systems in route
            for (const auto& sys_id : route->planned_route) {
                auto* sys_entity = world_->getEntity(sys_id);
                if (!sys_entity) continue;
                auto* sys_state = sys_entity->getComponent<components::SimStarSystemState>();
                if (sys_state && sys_state->threat_level >= route->danger_threshold) {
                    route->rerouting = true;
                    // Remove dangerous systems from route
                    route->planned_route.erase(
                        std::remove(route->planned_route.begin(),
                                    route->planned_route.end(), sys_id),
                        route->planned_route.end());
                    break;
                }
            }
            route->reroute_cooldown = reroute_interval;
        }
    }
}

bool NPCReroutingSystem::isRerouting(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* route = entity->getComponent<components::NPCRouteState>();
    if (!route) return false;
    return route->rerouting;
}

void NPCReroutingSystem::setRoute(const std::string& entity_id,
                                   const std::vector<std::string>& route) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;
    auto* rs = entity->getComponent<components::NPCRouteState>();
    if (!rs) return;
    rs->planned_route = route;
    rs->rerouting = false;
}

std::vector<std::string>
NPCReroutingSystem::getRoute(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return {};
    auto* route = entity->getComponent<components::NPCRouteState>();
    if (!route) return {};
    return route->planned_route;
}

} // namespace systems
} // namespace atlas
