#include "systems/npc_patrol_route_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using NPR = components::NPCPatrolRoute;
using Waypoint = components::NPCPatrolRoute::PatrolWaypoint;

Waypoint* findWaypoint(NPR* npr, const std::string& waypoint_id) {
    for (auto& w : npr->waypoints) {
        if (w.waypoint_id == waypoint_id) return &w;
    }
    return nullptr;
}

const Waypoint* findWaypointConst(const NPR* npr, const std::string& waypoint_id) {
    for (const auto& w : npr->waypoints) {
        if (w.waypoint_id == waypoint_id) return &w;
    }
    return nullptr;
}

const char* stateToString(NPR::PatrolState s) {
    switch (s) {
        case NPR::PatrolState::Idle: return "Idle";
        case NPR::PatrolState::Travelling: return "Travelling";
        case NPR::PatrolState::Waiting: return "Waiting";
        case NPR::PatrolState::Alert: return "Alert";
        case NPR::PatrolState::Complete: return "Complete";
    }
    return "Unknown";
}

} // anonymous namespace

NPCPatrolRouteSystem::NPCPatrolRouteSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void NPCPatrolRouteSystem::updateComponent(ecs::Entity& entity,
    components::NPCPatrolRoute& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.state == NPR::PatrolState::Alert) return; // paused during alert
    if (comp.state == NPR::PatrolState::Idle) return;
    if (comp.state == NPR::PatrolState::Complete) return;
    if (comp.waypoints.empty()) return;

    if (comp.state == NPR::PatrolState::Waiting) {
        comp.idle_timer -= delta_time;
        if (comp.idle_timer <= 0.0f) {
            comp.idle_timer = 0.0f;
            comp.current_waypoint_index++;
            if (comp.current_waypoint_index >= static_cast<int>(comp.waypoints.size())) {
                comp.total_patrols_completed++;
                if (comp.loop) {
                    comp.current_waypoint_index = 0;
                    comp.state = NPR::PatrolState::Travelling;
                } else {
                    comp.state = NPR::PatrolState::Complete;
                }
            } else {
                comp.state = NPR::PatrolState::Travelling;
            }
        }
    } else if (comp.state == NPR::PatrolState::Travelling) {
        float dist = comp.travel_speed * delta_time;
        comp.distance_to_next -= dist;
        if (comp.distance_to_next <= 0.0f) {
            comp.distance_to_next = 0.0f;
            comp.waypoints_visited++;
            const auto& wp = comp.waypoints[comp.current_waypoint_index];
            comp.idle_timer = wp.idle_time;
            if (comp.idle_timer > 0.0f) {
                comp.state = NPR::PatrolState::Waiting;
            } else {
                comp.current_waypoint_index++;
                if (comp.current_waypoint_index >= static_cast<int>(comp.waypoints.size())) {
                    comp.total_patrols_completed++;
                    if (comp.loop) {
                        comp.current_waypoint_index = 0;
                        comp.state = NPR::PatrolState::Travelling;
                    } else {
                        comp.state = NPR::PatrolState::Complete;
                    }
                }
            }
        }
    }
}

bool NPCPatrolRouteSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::NPCPatrolRoute>();
    entity->addComponent(std::move(comp));
    return true;
}

bool NPCPatrolRouteSystem::addWaypoint(const std::string& entity_id,
    const std::string& waypoint_id, float x, float y, float z,
    float idle_time, float alert_radius) {
    auto* npr = getComponentFor(entity_id);
    if (!npr) return false;
    if (static_cast<int>(npr->waypoints.size()) >= npr->max_waypoints) return false;
    if (findWaypoint(npr, waypoint_id)) return false; // duplicate

    Waypoint w;
    w.waypoint_id = waypoint_id;
    w.x = x;
    w.y = y;
    w.z = z;
    w.idle_time = idle_time;
    w.alert_radius = alert_radius;
    npr->waypoints.push_back(w);
    return true;
}

bool NPCPatrolRouteSystem::removeWaypoint(const std::string& entity_id,
    const std::string& waypoint_id) {
    auto* npr = getComponentFor(entity_id);
    if (!npr) return false;
    auto it = std::find_if(npr->waypoints.begin(), npr->waypoints.end(),
        [&](const Waypoint& w) { return w.waypoint_id == waypoint_id; });
    if (it == npr->waypoints.end()) return false;
    npr->waypoints.erase(it);
    return true;
}

bool NPCPatrolRouteSystem::startPatrol(const std::string& entity_id) {
    auto* npr = getComponentFor(entity_id);
    if (!npr || npr->waypoints.empty()) return false;
    npr->state = NPR::PatrolState::Travelling;
    npr->current_waypoint_index = 0;
    npr->distance_to_next = 500.0f; // default distance to first waypoint
    npr->hostile_detected = false;
    npr->detected_hostile_id.clear();
    return true;
}

bool NPCPatrolRouteSystem::stopPatrol(const std::string& entity_id) {
    auto* npr = getComponentFor(entity_id);
    if (!npr) return false;
    npr->state = NPR::PatrolState::Idle;
    return true;
}

bool NPCPatrolRouteSystem::triggerAlert(const std::string& entity_id,
    const std::string& hostile_id) {
    auto* npr = getComponentFor(entity_id);
    if (!npr) return false;
    npr->state = NPR::PatrolState::Alert;
    npr->hostile_detected = true;
    npr->detected_hostile_id = hostile_id;
    npr->total_alerts_triggered++;
    return true;
}

bool NPCPatrolRouteSystem::clearAlert(const std::string& entity_id) {
    auto* npr = getComponentFor(entity_id);
    if (!npr) return false;
    if (npr->state != NPR::PatrolState::Alert) return false;
    npr->hostile_detected = false;
    npr->detected_hostile_id.clear();
    npr->state = NPR::PatrolState::Travelling;
    return true;
}

int NPCPatrolRouteSystem::getWaypointCount(const std::string& entity_id) const {
    auto* npr = getComponentFor(entity_id);
    return npr ? static_cast<int>(npr->waypoints.size()) : 0;
}

int NPCPatrolRouteSystem::getCurrentWaypointIndex(const std::string& entity_id) const {
    auto* npr = getComponentFor(entity_id);
    return npr ? npr->current_waypoint_index : -1;
}

std::string NPCPatrolRouteSystem::getState(const std::string& entity_id) const {
    auto* npr = getComponentFor(entity_id);
    if (!npr) return "Unknown";
    return stateToString(npr->state);
}

bool NPCPatrolRouteSystem::isHostileDetected(const std::string& entity_id) const {
    auto* npr = getComponentFor(entity_id);
    return npr ? npr->hostile_detected : false;
}

int NPCPatrolRouteSystem::getTotalPatrolsCompleted(const std::string& entity_id) const {
    auto* npr = getComponentFor(entity_id);
    return npr ? npr->total_patrols_completed : 0;
}

int NPCPatrolRouteSystem::getTotalAlertsTriggered(const std::string& entity_id) const {
    auto* npr = getComponentFor(entity_id);
    return npr ? npr->total_alerts_triggered : 0;
}

int NPCPatrolRouteSystem::getWaypointsVisited(const std::string& entity_id) const {
    auto* npr = getComponentFor(entity_id);
    return npr ? npr->waypoints_visited : 0;
}

} // namespace systems
} // namespace atlas
