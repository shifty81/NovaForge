#include "systems/autopilot_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/navigation_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

AutopilotSystem::AutopilotSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void AutopilotSystem::updateComponent(ecs::Entity& /*entity*/, components::Autopilot& ap, float delta_time) {
    if (!ap.active || !ap.engaged) return;
    if (ap.waypoints.empty()) return;
    if (ap.current_waypoint_index >= static_cast<int>(ap.waypoints.size())) {
        if (ap.loop) {
            ap.current_waypoint_index = 0;
            for (auto& wp : ap.waypoints) wp.reached = false;
        } else {
            return; // route complete
        }
    }

    auto& wp = ap.waypoints[ap.current_waypoint_index];
    // Calculate distance (simplified, assumes ship at origin moving toward waypoint)
    float travel = ap.speed * delta_time;
    ap.distance_to_next -= travel;
    ap.total_distance_traveled += travel;

    if (ap.distance_to_next <= ap.arrival_distance) {
        wp.reached = true;
        ap.waypoints_reached++;
        ap.current_waypoint_index++;
        // Calculate distance to next waypoint
        if (ap.current_waypoint_index < static_cast<int>(ap.waypoints.size())) {
            auto& next_wp = ap.waypoints[ap.current_waypoint_index];
            float dx = next_wp.x - wp.x;
            float dy = next_wp.y - wp.y;
            float dz = next_wp.z - wp.z;
            ap.distance_to_next = std::sqrt(dx*dx + dy*dy + dz*dz);
        } else if (ap.loop && !ap.waypoints.empty()) {
            ap.current_waypoint_index = 0;
            for (auto& w : ap.waypoints) w.reached = false;
            auto& next_wp = ap.waypoints[0];
            float dx = next_wp.x - wp.x;
            float dy = next_wp.y - wp.y;
            float dz = next_wp.z - wp.z;
            ap.distance_to_next = std::sqrt(dx*dx + dy*dy + dz*dz);
        }
    }
}

bool AutopilotSystem::initializeAutopilot(const std::string& entity_id,
    const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::Autopilot>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool AutopilotSystem::addWaypoint(const std::string& entity_id,
    const std::string& waypoint_id, const std::string& label,
    float x, float y, float z) {
    auto* ap = getComponentFor(entity_id);
    if (!ap) return false;
    if (static_cast<int>(ap->waypoints.size()) >= ap->max_waypoints) return false;
    // Check for duplicate
    for (const auto& wp : ap->waypoints) {
        if (wp.waypoint_id == waypoint_id) return false;
    }

    components::Autopilot::Waypoint wp;
    wp.waypoint_id = waypoint_id;
    wp.label = label;
    wp.x = x;
    wp.y = y;
    wp.z = z;
    ap->waypoints.push_back(wp);
    return true;
}

bool AutopilotSystem::removeWaypoint(const std::string& entity_id,
    const std::string& waypoint_id) {
    auto* ap = getComponentFor(entity_id);
    if (!ap) return false;

    auto it = std::remove_if(ap->waypoints.begin(), ap->waypoints.end(),
        [&](const components::Autopilot::Waypoint& w) {
            return w.waypoint_id == waypoint_id;
        });
    if (it == ap->waypoints.end()) return false;
    ap->waypoints.erase(it, ap->waypoints.end());
    return true;
}

bool AutopilotSystem::engage(const std::string& entity_id) {
    auto* ap = getComponentFor(entity_id);
    if (!ap) return false;
    if (ap->waypoints.empty()) return false;
    ap->engaged = true;
    // Set initial distance to first waypoint (from origin)
    if (ap->current_waypoint_index < static_cast<int>(ap->waypoints.size())) {
        auto& wp = ap->waypoints[ap->current_waypoint_index];
        ap->distance_to_next = std::sqrt(wp.x*wp.x + wp.y*wp.y + wp.z*wp.z);
    }
    return true;
}

bool AutopilotSystem::disengage(const std::string& entity_id) {
    auto* ap = getComponentFor(entity_id);
    if (!ap) return false;
    ap->engaged = false;
    return true;
}

bool AutopilotSystem::setLoop(const std::string& entity_id, bool loop) {
    auto* ap = getComponentFor(entity_id);
    if (!ap) return false;
    ap->loop = loop;
    return true;
}

bool AutopilotSystem::setSpeed(const std::string& entity_id, float speed) {
    auto* ap = getComponentFor(entity_id);
    if (!ap) return false;
    ap->speed = speed;
    return true;
}

int AutopilotSystem::getWaypointCount(const std::string& entity_id) const {
    const auto* ap = getComponentFor(entity_id);
    if (!ap) return 0;
    return static_cast<int>(ap->waypoints.size());
}

int AutopilotSystem::getCurrentWaypointIndex(const std::string& entity_id) const {
    const auto* ap = getComponentFor(entity_id);
    if (!ap) return 0;
    return ap->current_waypoint_index;
}

int AutopilotSystem::getWaypointsReached(const std::string& entity_id) const {
    const auto* ap = getComponentFor(entity_id);
    if (!ap) return 0;
    return ap->waypoints_reached;
}

float AutopilotSystem::getTotalDistanceTraveled(const std::string& entity_id) const {
    const auto* ap = getComponentFor(entity_id);
    if (!ap) return 0.0f;
    return ap->total_distance_traveled;
}

bool AutopilotSystem::isEngaged(const std::string& entity_id) const {
    const auto* ap = getComponentFor(entity_id);
    if (!ap) return false;
    return ap->engaged;
}

bool AutopilotSystem::isRouteComplete(const std::string& entity_id) const {
    const auto* ap = getComponentFor(entity_id);
    if (!ap) return false;
    if (ap->waypoints.empty()) return true;
    if (ap->loop) return false; // looping routes never complete
    return ap->current_waypoint_index >= static_cast<int>(ap->waypoints.size());
}

} // namespace systems
} // namespace atlas
