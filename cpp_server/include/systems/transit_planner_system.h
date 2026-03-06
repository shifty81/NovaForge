#ifndef NOVAFORGE_SYSTEMS_TRANSIT_PLANNER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TRANSIT_PLANNER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Multi-leg journey planning with waypoints, ETAs, and progress tracking
 *
 * Manages transit routes composed of ordered waypoints. Tracks current leg,
 * calculates ETAs and total travel time, and advances through waypoints as
 * each leg is completed. Supports route modification (add/remove waypoints)
 * and fuel consumption forecasting.
 */
class TransitPlannerSystem : public ecs::SingleComponentSystem<components::TransitPlannerState> {
public:
    explicit TransitPlannerSystem(ecs::World* world);
    ~TransitPlannerSystem() override = default;

    std::string getName() const override { return "TransitPlannerSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Waypoint management
    bool addWaypoint(const std::string& entity_id, const std::string& waypoint_id,
                     const std::string& waypoint_name, float travel_time, float fuel_cost);
    bool removeWaypoint(const std::string& entity_id, const std::string& waypoint_id);
    int getWaypointCount(const std::string& entity_id) const;

    // Route progress
    bool advanceToNextWaypoint(const std::string& entity_id);
    int getCurrentWaypointIndex(const std::string& entity_id) const;
    std::string getCurrentWaypointName(const std::string& entity_id) const;
    bool isRouteComplete(const std::string& entity_id) const;

    // Time calculations
    float getTotalTravelTime(const std::string& entity_id) const;
    float getRemainingTravelTime(const std::string& entity_id) const;
    float getElapsedTravelTime(const std::string& entity_id) const;

    // Fuel calculations
    float getTotalFuelCost(const std::string& entity_id) const;
    float getRemainingFuelCost(const std::string& entity_id) const;

    // Route status
    int getCompletedLegs(const std::string& entity_id) const;
    int getRemainingLegs(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::TransitPlannerState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TRANSIT_PLANNER_SYSTEM_H
