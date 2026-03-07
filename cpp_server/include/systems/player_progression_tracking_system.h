#ifndef NOVAFORGE_SYSTEMS_PLAYER_PROGRESSION_TRACKING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_PROGRESSION_TRACKING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks player milestones, achievements, and progression stats
 *
 * Records gameplay milestones such as first dock, first kill, first trade,
 * mining totals, ISC earned, and systems visited. Provides vertical-slice
 * metrics that prove the core loop is playable end-to-end.
 */
class PlayerProgressionTrackingSystem : public ecs::SingleComponentSystem<components::PlayerProgressionTracking> {
public:
    explicit PlayerProgressionTrackingSystem(ecs::World* world);
    ~PlayerProgressionTrackingSystem() override = default;

    std::string getName() const override { return "PlayerProgressionTrackingSystem"; }

public:
    bool initialize(const std::string& entity_id);
    bool recordMilestone(const std::string& entity_id, const std::string& milestone_id,
                         const std::string& description);
    bool addIscEarned(const std::string& entity_id, float amount);
    bool addIscSpent(const std::string& entity_id, float amount);
    bool recordKill(const std::string& entity_id);
    bool recordDeath(const std::string& entity_id);
    bool recordDock(const std::string& entity_id);
    bool recordJump(const std::string& entity_id);
    bool addMiningYield(const std::string& entity_id, float ore_units);
    int getMilestoneCount(const std::string& entity_id) const;
    bool hasMilestone(const std::string& entity_id, const std::string& milestone_id) const;
    float getIscEarned(const std::string& entity_id) const;
    float getIscSpent(const std::string& entity_id) const;
    int getKills(const std::string& entity_id) const;
    int getDeaths(const std::string& entity_id) const;
    int getDocks(const std::string& entity_id) const;
    int getJumps(const std::string& entity_id) const;
    float getMiningYield(const std::string& entity_id) const;
    float getPlayTime(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PlayerProgressionTracking& ppt, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_PROGRESSION_TRACKING_SYSTEM_H
