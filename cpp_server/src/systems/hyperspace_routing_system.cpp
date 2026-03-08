#include "systems/hyperspace_routing_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include <limits>

namespace atlas {
namespace systems {

HyperspaceRoutingSystem::HyperspaceRoutingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void HyperspaceRoutingSystem::updateComponent(ecs::Entity& entity,
    components::HyperspaceRoute& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.route_active) {
        comp.elapsed_travel_time += delta_time;
    }
}

bool HyperspaceRoutingSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::HyperspaceRoute>();
    entity->addComponent(std::move(comp));
    return true;
}

bool HyperspaceRoutingSystem::addGateConnection(const std::string& from_system,
    const std::string& to_system, const std::string& gate_id, float travel_time) {
    if (from_system.empty() || to_system.empty() || gate_id.empty()) return false;
    if (travel_time <= 0.0f) return false;
    if (from_system == to_system) return false;

    // Check for duplicate
    auto& edges = m_gateGraph[from_system];
    for (const auto& e : edges) {
        if (e.to_system == to_system) return false;
    }

    edges.push_back({to_system, gate_id, travel_time});
    return true;
}

bool HyperspaceRoutingSystem::removeGateConnection(const std::string& from_system,
    const std::string& to_system) {
    auto it = m_gateGraph.find(from_system);
    if (it == m_gateGraph.end()) return false;

    auto& edges = it->second;
    auto eit = std::find_if(edges.begin(), edges.end(),
        [&to_system](const GateEdge& e) { return e.to_system == to_system; });
    if (eit == edges.end()) return false;

    edges.erase(eit);
    if (edges.empty()) m_gateGraph.erase(it);
    return true;
}

int HyperspaceRoutingSystem::getConnectionCount() const {
    int count = 0;
    for (const auto& pair : m_gateGraph) {
        count += static_cast<int>(pair.second.size());
    }
    return count;
}

bool HyperspaceRoutingSystem::calculateRoute(const std::string& entity_id,
    const std::string& origin, const std::string& destination) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (origin.empty() || destination.empty()) return false;
    if (origin == destination) return false;

    // BFS shortest path (fewest jumps)
    std::unordered_map<std::string, std::string> came_from;
    std::unordered_map<std::string, GateEdge> edge_used;
    std::queue<std::string> frontier;
    std::unordered_set<std::string> visited;

    frontier.push(origin);
    visited.insert(origin);
    came_from[origin] = "";

    bool found = false;
    while (!frontier.empty()) {
        std::string current = frontier.front();
        frontier.pop();

        if (current == destination) {
            found = true;
            break;
        }

        auto git = m_gateGraph.find(current);
        if (git == m_gateGraph.end()) continue;

        for (const auto& edge : git->second) {
            if (visited.count(edge.to_system)) continue;
            visited.insert(edge.to_system);
            came_from[edge.to_system] = current;
            edge_used[edge.to_system] = edge;
            frontier.push(edge.to_system);
        }
    }

    if (!found) {
        comp->route_valid = false;
        return false;
    }

    // Reconstruct path
    std::vector<components::HyperspaceRoute::Waypoint> path;
    float total_time = 0.0f;
    std::string current = destination;

    while (current != origin) {
        auto& edge = edge_used[current];
        components::HyperspaceRoute::Waypoint wp;
        wp.system_id = current;
        wp.gate_id = edge.gate_id;
        wp.estimated_travel_time = edge.travel_time;
        total_time += edge.travel_time;
        path.push_back(wp);
        current = came_from[current];
    }

    std::reverse(path.begin(), path.end());

    comp->origin_system = origin;
    comp->destination_system = destination;
    comp->waypoints = path;
    comp->current_waypoint_index = 0;
    comp->total_estimated_time = total_time;
    comp->elapsed_travel_time = 0.0f;
    comp->route_active = true;
    comp->route_valid = true;
    comp->total_routes_calculated++;
    return true;
}

bool HyperspaceRoutingSystem::clearRoute(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->waypoints.clear();
    comp->current_waypoint_index = -1;
    comp->origin_system.clear();
    comp->destination_system.clear();
    comp->total_estimated_time = 0.0f;
    comp->elapsed_travel_time = 0.0f;
    comp->route_active = false;
    return true;
}

bool HyperspaceRoutingSystem::advanceWaypoint(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->route_active) return false;
    if (comp->current_waypoint_index < 0) return false;
    if (comp->current_waypoint_index >= static_cast<int>(comp->waypoints.size())) return false;

    comp->waypoints[comp->current_waypoint_index].visited = true;
    comp->total_jumps_completed++;
    comp->current_waypoint_index++;

    if (comp->current_waypoint_index >= static_cast<int>(comp->waypoints.size())) {
        comp->route_active = false;  // Route complete
    }
    return true;
}

bool HyperspaceRoutingSystem::isRouteComplete(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->waypoints.empty()) return false;
    return comp->current_waypoint_index >= static_cast<int>(comp->waypoints.size());
}

int HyperspaceRoutingSystem::getWaypointCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->waypoints.size()) : 0;
}

int HyperspaceRoutingSystem::getCurrentWaypointIndex(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_waypoint_index : -1;
}

std::string HyperspaceRoutingSystem::getCurrentSystem(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    if (comp->current_waypoint_index < 0) return comp->origin_system;
    if (comp->current_waypoint_index >= static_cast<int>(comp->waypoints.size()))
        return comp->destination_system;
    return comp->waypoints[comp->current_waypoint_index].system_id;
}

std::string HyperspaceRoutingSystem::getDestination(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->destination_system : "";
}

float HyperspaceRoutingSystem::getTotalEstimatedTime(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_estimated_time : 0.0f;
}

float HyperspaceRoutingSystem::getElapsedTravelTime(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->elapsed_travel_time : 0.0f;
}

int HyperspaceRoutingSystem::getTotalRoutesCalculated(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_routes_calculated : 0;
}

int HyperspaceRoutingSystem::getTotalJumpsCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_jumps_completed : 0;
}

int HyperspaceRoutingSystem::getRemainingJumps(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->remainingJumps() : 0;
}

bool HyperspaceRoutingSystem::isRouteActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->route_active : false;
}

} // namespace systems
} // namespace atlas
