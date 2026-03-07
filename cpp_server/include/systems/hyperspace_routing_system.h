#ifndef NOVAFORGE_SYSTEMS_HYPERSPACE_ROUTING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_HYPERSPACE_ROUTING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Multi-system route calculation via jump gates
 *
 * Computes optimal paths between star systems using a graph of jump gate
 * connections.  Supports adding/removing gate connections, calculating
 * shortest routes, advancing through waypoints, and tracking route
 * analytics (total routes calculated, jumps completed).
 */
class HyperspaceRoutingSystem : public ecs::SingleComponentSystem<components::HyperspaceRoute> {
public:
    explicit HyperspaceRoutingSystem(ecs::World* world);
    ~HyperspaceRoutingSystem() override = default;

    std::string getName() const override { return "HyperspaceRoutingSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id);

    // Gate graph management
    bool addGateConnection(const std::string& from_system, const std::string& to_system,
                           const std::string& gate_id, float travel_time);
    bool removeGateConnection(const std::string& from_system, const std::string& to_system);
    int getConnectionCount() const;

    // Route calculation
    bool calculateRoute(const std::string& entity_id, const std::string& origin,
                        const std::string& destination);
    bool clearRoute(const std::string& entity_id);

    // Navigation
    bool advanceWaypoint(const std::string& entity_id);
    bool isRouteComplete(const std::string& entity_id) const;

    // Queries
    int getWaypointCount(const std::string& entity_id) const;
    int getCurrentWaypointIndex(const std::string& entity_id) const;
    std::string getCurrentSystem(const std::string& entity_id) const;
    std::string getDestination(const std::string& entity_id) const;
    float getTotalEstimatedTime(const std::string& entity_id) const;
    float getElapsedTravelTime(const std::string& entity_id) const;
    int getTotalRoutesCalculated(const std::string& entity_id) const;
    int getTotalJumpsCompleted(const std::string& entity_id) const;
    int getRemainingJumps(const std::string& entity_id) const;
    bool isRouteActive(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::HyperspaceRoute& comp,
                         float delta_time) override;

private:
    struct GateEdge {
        std::string to_system;
        std::string gate_id;
        float travel_time = 0.0f;
    };
    std::map<std::string, std::vector<GateEdge>> m_gateGraph;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_HYPERSPACE_ROUTING_SYSTEM_H
