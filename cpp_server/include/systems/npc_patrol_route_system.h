#ifndef NOVAFORGE_SYSTEMS_NPC_PATROL_ROUTE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_PATROL_ROUTE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief NPC patrol route management with ordered waypoints and patrol modes.
 *
 * An NPC entity can have a list of Waypoints.  startPatrol() begins travel
 * toward waypoints in order.  When a waypoint is reached the NPC dwells for
 * dwell_time seconds, then advances to the next one.
 *
 * PatrolMode::Loop  — wraps from last waypoint back to index 0.
 * PatrolMode::PingPong — reverses direction at each end; a full circuit
 *   (0→last→0) counts as two increments of total_circuits.
 *
 * advanceWaypoint() immediately moves to the next waypoint (used in tests
 * and by the per-tick update when travel is instantaneous or skipped).
 */
class NpcPatrolRouteSystem
    : public ecs::SingleComponentSystem<components::NpcPatrolRoute> {
public:
    explicit NpcPatrolRouteSystem(ecs::World* world);
    ~NpcPatrolRouteSystem() override = default;

    std::string getName() const override { return "NpcPatrolRouteSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Waypoint management ---
    bool addWaypoint(const std::string& entity_id,
                     const std::string& waypoint_id,
                     float x, float y, float z,
                     float dwell_time = 0.0f);
    bool removeWaypoint(const std::string& entity_id,
                        const std::string& waypoint_id);
    bool clearWaypoints(const std::string& entity_id);

    // --- Patrol control ---
    bool startPatrol(const std::string& entity_id);
    bool stopPatrol(const std::string& entity_id);
    bool setPatrolMode(const std::string& entity_id,
                       components::NpcPatrolRoute::PatrolMode mode);
    bool setSpeed(const std::string& entity_id, float speed);
    bool advanceWaypoint(const std::string& entity_id);

    // --- Queries ---
    int   getWaypointCount(const std::string& entity_id) const;
    std::string getCurrentWaypointId(const std::string& entity_id) const;
    int   getCurrentWaypointIndex(const std::string& entity_id) const;
    components::NpcPatrolRoute::Status     getStatus(const std::string& entity_id) const;
    components::NpcPatrolRoute::PatrolMode getPatrolMode(const std::string& entity_id) const;
    bool  isPatrolling(const std::string& entity_id) const;
    float getSpeed(const std::string& entity_id) const;
    float getDwellTimer(const std::string& entity_id) const;
    int   getTotalCircuits(const std::string& entity_id) const;
    int   getTotalWaypointsVisited(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::NpcPatrolRoute& comp,
                         float delta_time) override;

private:
    void advanceToNext(components::NpcPatrolRoute& comp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_PATROL_ROUTE_SYSTEM_H
