#include "systems/transit_planner_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using TP = components::TransitPlannerState;
}

TransitPlannerSystem::TransitPlannerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void TransitPlannerSystem::updateComponent(ecs::Entity& entity,
    components::TransitPlannerState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;
}

bool TransitPlannerSystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::TransitPlannerState>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool TransitPlannerSystem::addWaypoint(const std::string& entity_id,
    const std::string& waypoint_id, const std::string& waypoint_name,
    float travel_time, float fuel_cost) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // Duplicate check
    for (const auto& wp : state->waypoints) {
        if (wp.waypoint_id == waypoint_id) return false;
    }
    if (static_cast<int>(state->waypoints.size()) >= state->max_waypoints) return false;
    TP::Waypoint wp;
    wp.waypoint_id = waypoint_id;
    wp.waypoint_name = waypoint_name;
    wp.travel_time = travel_time;
    wp.fuel_cost = fuel_cost;
    state->waypoints.push_back(wp);
    return true;
}

bool TransitPlannerSystem::removeWaypoint(const std::string& entity_id,
    const std::string& waypoint_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto it = state->waypoints.begin(); it != state->waypoints.end(); ++it) {
        if (it->waypoint_id == waypoint_id) {
            int removed_index = static_cast<int>(std::distance(state->waypoints.begin(), it));
            state->waypoints.erase(it);
            // Adjust current index if the removed waypoint was before or at current
            if (removed_index < state->current_waypoint_index) {
                state->current_waypoint_index--;
            } else if (removed_index == state->current_waypoint_index) {
                // If we removed the current waypoint, clamp
                if (state->current_waypoint_index >= static_cast<int>(state->waypoints.size())) {
                    state->current_waypoint_index = static_cast<int>(state->waypoints.size());
                }
            }
            return true;
        }
    }
    return false;
}

int TransitPlannerSystem::getWaypointCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->waypoints.size()) : 0;
}

bool TransitPlannerSystem::advanceToNextWaypoint(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->current_waypoint_index >= static_cast<int>(state->waypoints.size())) return false;
    state->current_waypoint_index++;
    return true;
}

int TransitPlannerSystem::getCurrentWaypointIndex(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->current_waypoint_index : -1;
}

std::string TransitPlannerSystem::getCurrentWaypointName(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return "";
    if (state->current_waypoint_index < 0 ||
        state->current_waypoint_index >= static_cast<int>(state->waypoints.size())) return "";
    return state->waypoints[state->current_waypoint_index].waypoint_name;
}

bool TransitPlannerSystem::isRouteComplete(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    return state->current_waypoint_index >= static_cast<int>(state->waypoints.size());
}

float TransitPlannerSystem::getTotalTravelTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float total = 0.0f;
    for (const auto& wp : state->waypoints) {
        total += wp.travel_time;
    }
    return total;
}

float TransitPlannerSystem::getRemainingTravelTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float remaining = 0.0f;
    for (int i = state->current_waypoint_index;
         i < static_cast<int>(state->waypoints.size()); ++i) {
        remaining += state->waypoints[i].travel_time;
    }
    return remaining;
}

float TransitPlannerSystem::getElapsedTravelTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float elapsed = 0.0f;
    for (int i = 0; i < state->current_waypoint_index &&
         i < static_cast<int>(state->waypoints.size()); ++i) {
        elapsed += state->waypoints[i].travel_time;
    }
    return elapsed;
}

float TransitPlannerSystem::getTotalFuelCost(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float total = 0.0f;
    for (const auto& wp : state->waypoints) {
        total += wp.fuel_cost;
    }
    return total;
}

float TransitPlannerSystem::getRemainingFuelCost(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float remaining = 0.0f;
    for (int i = state->current_waypoint_index;
         i < static_cast<int>(state->waypoints.size()); ++i) {
        remaining += state->waypoints[i].fuel_cost;
    }
    return remaining;
}

int TransitPlannerSystem::getCompletedLegs(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->current_waypoint_index : 0;
}

int TransitPlannerSystem::getRemainingLegs(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int remaining = static_cast<int>(state->waypoints.size()) - state->current_waypoint_index;
    return remaining > 0 ? remaining : 0;
}

} // namespace systems
} // namespace atlas
