#ifndef NOVAFORGE_SYSTEMS_SESSION_PROGRESSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SESSION_PROGRESSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks cumulative player progress metrics across the vertical slice
 *
 * Records gameplay milestones and statistics during a session: ISK earned,
 * asteroids mined, NPCs destroyed, items traded, jumps completed, etc.
 * Integrates with the OnboardingSystem for phase-completion detection.
 */
class SessionProgressionSystem : public ecs::SingleComponentSystem<components::SessionProgression> {
public:
    explicit SessionProgressionSystem(ecs::World* world);
    ~SessionProgressionSystem() override = default;

    std::string getName() const override { return "SessionProgressionSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Milestone tracking
    bool recordMilestone(const std::string& entity_id, const std::string& milestone_id,
                         const std::string& description, float timestamp);
    bool isMilestoneReached(const std::string& entity_id, const std::string& milestone_id) const;
    int getMilestoneCount(const std::string& entity_id) const;

    // Statistic accumulation
    bool addStatistic(const std::string& entity_id, const std::string& stat_key, double value);
    double getStatistic(const std::string& entity_id, const std::string& stat_key) const;
    int getStatisticCount(const std::string& entity_id) const;

    // Activity logging
    bool logActivity(const std::string& entity_id, const std::string& activity_type,
                     const std::string& detail, float timestamp);
    int getActivityCount(const std::string& entity_id) const;
    std::string getLastActivityType(const std::string& entity_id) const;

    // Session summary
    float getSessionDuration(const std::string& entity_id) const;
    bool finalizeSession(const std::string& entity_id, float end_time);
    bool isSessionFinalized(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SessionProgression& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SESSION_PROGRESSION_SYSTEM_H
