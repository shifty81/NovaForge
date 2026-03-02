#ifndef NOVAFORGE_SYSTEMS_AUTOPILOT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_AUTOPILOT_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Waypoint-based navigation autopilot for ships
 *
 * Manages waypoint routes with automatic progression, arrival detection,
 * distance tracking, and optional looping. Ships follow waypoints in
 * sequence at configured speed.
 */
class AutopilotSystem : public ecs::System {
public:
    explicit AutopilotSystem(ecs::World* world);
    ~AutopilotSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "AutopilotSystem"; }

    bool initializeAutopilot(const std::string& entity_id, const std::string& owner_id);
    bool addWaypoint(const std::string& entity_id, const std::string& waypoint_id,
                     const std::string& label, float x, float y, float z);
    bool removeWaypoint(const std::string& entity_id, const std::string& waypoint_id);
    bool engage(const std::string& entity_id);
    bool disengage(const std::string& entity_id);
    bool setLoop(const std::string& entity_id, bool loop);
    bool setSpeed(const std::string& entity_id, float speed);
    int getWaypointCount(const std::string& entity_id) const;
    int getCurrentWaypointIndex(const std::string& entity_id) const;
    int getWaypointsReached(const std::string& entity_id) const;
    float getTotalDistanceTraveled(const std::string& entity_id) const;
    bool isEngaged(const std::string& entity_id) const;
    bool isRouteComplete(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_AUTOPILOT_SYSTEM_H
