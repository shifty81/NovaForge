#ifndef NOVAFORGE_SYSTEMS_NPC_PATROL_ROUTE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_PATROL_ROUTE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Waypoint-based patrol route system for NPC security and pirate AI
 *
 * Advances NPCs through patrol waypoints, handles idle pauses at each
 * waypoint, detects hostiles within alert radius, and optionally loops
 * the route.  Supports the vertical-slice pirate zone and AI traffic.
 */
class NPCPatrolRouteSystem : public ecs::SingleComponentSystem<components::NPCPatrolRoute> {
public:
    explicit NPCPatrolRouteSystem(ecs::World* world);
    ~NPCPatrolRouteSystem() override = default;

    std::string getName() const override { return "NPCPatrolRouteSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool addWaypoint(const std::string& entity_id, const std::string& waypoint_id,
                     float x, float y, float z, float idle_time, float alert_radius);
    bool removeWaypoint(const std::string& entity_id, const std::string& waypoint_id);
    bool startPatrol(const std::string& entity_id);
    bool stopPatrol(const std::string& entity_id);
    bool triggerAlert(const std::string& entity_id, const std::string& hostile_id);
    bool clearAlert(const std::string& entity_id);
    int getWaypointCount(const std::string& entity_id) const;
    int getCurrentWaypointIndex(const std::string& entity_id) const;
    std::string getState(const std::string& entity_id) const;
    bool isHostileDetected(const std::string& entity_id) const;
    int getTotalPatrolsCompleted(const std::string& entity_id) const;
    int getTotalAlertsTriggered(const std::string& entity_id) const;
    int getWaypointsVisited(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::NPCPatrolRoute& comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_PATROL_ROUTE_SYSTEM_H
