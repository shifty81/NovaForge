#ifndef NOVAFORGE_SYSTEMS_OBJECTIVE_TRACKER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_OBJECTIVE_TRACKER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Converts missions into concrete, waypointed objectives for the player HUD
 *
 * Bridges the gap between abstract mission acceptance and real-time player
 * guidance: tracks active objectives, waypoint distances, completion
 * percentages, and provides the data the HUD needs to render objective markers.
 */
class ObjectiveTrackerSystem : public ecs::SingleComponentSystem<components::ObjectiveTrackerState> {
public:
    explicit ObjectiveTrackerSystem(ecs::World* world);
    ~ObjectiveTrackerSystem() override = default;

    std::string getName() const override { return "ObjectiveTrackerSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Objective management
    bool addObjective(const std::string& entity_id, const std::string& objective_id,
                      const std::string& description, const std::string& category,
                      float target_x, float target_y);
    bool removeObjective(const std::string& entity_id, const std::string& objective_id);
    int getObjectiveCount(const std::string& entity_id) const;
    bool hasObjective(const std::string& entity_id, const std::string& objective_id) const;

    // Objective progress
    bool updateProgress(const std::string& entity_id, const std::string& objective_id,
                        float progress);
    float getProgress(const std::string& entity_id, const std::string& objective_id) const;
    bool completeObjective(const std::string& entity_id, const std::string& objective_id,
                           float timestamp);
    bool isObjectiveComplete(const std::string& entity_id, const std::string& objective_id) const;

    // Active objective selection
    bool setActiveObjective(const std::string& entity_id, const std::string& objective_id);
    std::string getActiveObjectiveId(const std::string& entity_id) const;

    // Waypoint / distance
    bool updatePlayerPosition(const std::string& entity_id, float px, float py);
    float getDistanceToObjective(const std::string& entity_id, const std::string& objective_id) const;
    float getDistanceToActive(const std::string& entity_id) const;

    // Summary queries
    int getCompletedCount(const std::string& entity_id) const;
    float getOverallCompletion(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ObjectiveTrackerState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_OBJECTIVE_TRACKER_SYSTEM_H
