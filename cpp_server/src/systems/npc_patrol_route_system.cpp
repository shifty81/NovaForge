#include "systems/npc_patrol_route_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

NpcPatrolRouteSystem::NpcPatrolRouteSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void NpcPatrolRouteSystem::updateComponent(ecs::Entity& /*entity*/,
                                            components::NpcPatrolRoute& comp,
                                            float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.status == components::NpcPatrolRoute::Status::Idle) return;
    if (comp.waypoints.empty()) return;

    if (comp.status == components::NpcPatrolRoute::Status::Dwelling) {
        comp.dwell_timer -= delta_time;
        if (comp.dwell_timer <= 0.0f) {
            comp.dwell_timer = 0.0f;
            advanceToNext(comp);
        }
    }
    // Status::Traveling is resolved externally via advanceWaypoint();
    // the per-tick here handles dwell expiry only.
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool NpcPatrolRouteSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::NpcPatrolRoute>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Internal helper
// ---------------------------------------------------------------------------

void NpcPatrolRouteSystem::advanceToNext(components::NpcPatrolRoute& comp) {
    if (comp.waypoints.empty()) return;

    const int count = static_cast<int>(comp.waypoints.size());

    if (comp.patrol_mode == components::NpcPatrolRoute::PatrolMode::Loop) {
        comp.current_index = (comp.current_index + 1) % count;
        if (comp.current_index == 0) {
            comp.total_circuits++;
        }
    } else {
        // PingPong
        comp.current_index += comp.direction;
        if (comp.current_index >= count) {
            comp.current_index = count - 2;
            if (comp.current_index < 0) comp.current_index = 0;
            comp.direction = -1;
            comp.total_circuits++;
        } else if (comp.current_index < 0) {
            comp.current_index = 1;
            if (comp.current_index >= count) comp.current_index = 0;
            comp.direction = 1;
            comp.total_circuits++;
        }
    }

    comp.total_waypoints_visited++;

    float dwell = comp.waypoints[comp.current_index].dwell_time;
    if (dwell > 0.0f) {
        comp.dwell_timer = dwell;
        comp.status      = components::NpcPatrolRoute::Status::Dwelling;
    } else {
        comp.status = components::NpcPatrolRoute::Status::Traveling;
    }
}

// ---------------------------------------------------------------------------
// Waypoint management
// ---------------------------------------------------------------------------

bool NpcPatrolRouteSystem::addWaypoint(const std::string& entity_id,
                                        const std::string& waypoint_id,
                                        float x, float y, float z,
                                        float dwell_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (waypoint_id.empty()) return false;
    if (dwell_time < 0.0f) return false;
    if (static_cast<int>(comp->waypoints.size()) >= comp->max_waypoints) return false;

    for (const auto& wp : comp->waypoints) {
        if (wp.waypoint_id == waypoint_id) return false;
    }

    components::NpcPatrolRoute::Waypoint wp;
    wp.waypoint_id = waypoint_id;
    wp.x           = x;
    wp.y           = y;
    wp.z           = z;
    wp.dwell_time  = dwell_time;
    comp->waypoints.push_back(wp);
    return true;
}

bool NpcPatrolRouteSystem::removeWaypoint(const std::string& entity_id,
                                           const std::string& waypoint_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->waypoints.begin(), comp->waypoints.end(),
        [&](const components::NpcPatrolRoute::Waypoint& wp) {
            return wp.waypoint_id == waypoint_id;
        });
    if (it == comp->waypoints.end()) return false;
    comp->waypoints.erase(it);
    if (!comp->waypoints.empty() &&
        comp->current_index >= static_cast<int>(comp->waypoints.size())) {
        comp->current_index = static_cast<int>(comp->waypoints.size()) - 1;
    }
    return true;
}

bool NpcPatrolRouteSystem::clearWaypoints(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->waypoints.clear();
    comp->current_index = 0;
    comp->status        = components::NpcPatrolRoute::Status::Idle;
    return true;
}

// ---------------------------------------------------------------------------
// Patrol control
// ---------------------------------------------------------------------------

bool NpcPatrolRouteSystem::startPatrol(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->waypoints.empty()) return false;
    if (comp->status != components::NpcPatrolRoute::Status::Idle) return false;

    comp->current_index = 0;
    comp->direction     = 1;
    float dwell = comp->waypoints[0].dwell_time;
    if (dwell > 0.0f) {
        comp->dwell_timer = dwell;
        comp->status      = components::NpcPatrolRoute::Status::Dwelling;
    } else {
        comp->status = components::NpcPatrolRoute::Status::Traveling;
    }
    comp->total_waypoints_visited++;
    return true;
}

bool NpcPatrolRouteSystem::stopPatrol(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->status == components::NpcPatrolRoute::Status::Idle) return false;
    comp->status      = components::NpcPatrolRoute::Status::Idle;
    comp->dwell_timer = 0.0f;
    return true;
}

bool NpcPatrolRouteSystem::setPatrolMode(
        const std::string& entity_id,
        components::NpcPatrolRoute::PatrolMode mode) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->patrol_mode = mode;
    return true;
}

bool NpcPatrolRouteSystem::setSpeed(const std::string& entity_id, float speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (speed <= 0.0f) return false;
    comp->speed = speed;
    return true;
}

bool NpcPatrolRouteSystem::advanceWaypoint(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->status == components::NpcPatrolRoute::Status::Idle) return false;
    if (comp->waypoints.empty()) return false;
    advanceToNext(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int NpcPatrolRouteSystem::getWaypointCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->waypoints.size()) : 0;
}

std::string NpcPatrolRouteSystem::getCurrentWaypointId(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->waypoints.empty()) return "";
    return comp->waypoints[comp->current_index].waypoint_id;
}

int NpcPatrolRouteSystem::getCurrentWaypointIndex(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_index : 0;
}

components::NpcPatrolRoute::Status
NpcPatrolRouteSystem::getStatus(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->status
                : components::NpcPatrolRoute::Status::Idle;
}

components::NpcPatrolRoute::PatrolMode
NpcPatrolRouteSystem::getPatrolMode(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->patrol_mode
                : components::NpcPatrolRoute::PatrolMode::Loop;
}

bool NpcPatrolRouteSystem::isPatrolling(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->status != components::NpcPatrolRoute::Status::Idle;
}

float NpcPatrolRouteSystem::getSpeed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->speed : 0.0f;
}

float NpcPatrolRouteSystem::getDwellTimer(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->dwell_timer : 0.0f;
}

int NpcPatrolRouteSystem::getTotalCircuits(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_circuits : 0;
}

int NpcPatrolRouteSystem::getTotalWaypointsVisited(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_waypoints_visited : 0;
}

} // namespace systems
} // namespace atlas
