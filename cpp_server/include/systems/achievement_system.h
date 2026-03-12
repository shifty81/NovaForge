#ifndef NOVAFORGE_SYSTEMS_ACHIEVEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ACHIEVEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Player achievement / milestone tracking system
 *
 * Manages a set of achievements that belong to categories (Combat, Economy,
 * Exploration, Social, Progression).  Each achievement has a required_count;
 * progressAchievement increments the current_count until the achievement is
 * unlocked.  Unlocking accumulates reward_points.  The list is capped at
 * max_achievements (default 50).
 */
class AchievementSystem
    : public ecs::SingleComponentSystem<components::AchievementState> {
public:
    explicit AchievementSystem(ecs::World* world);
    ~AchievementSystem() override = default;

    std::string getName() const override { return "AchievementSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Achievement management ---
    bool addAchievement(const std::string& entity_id,
                        const std::string& ach_id,
                        const std::string& name,
                        components::AchievementState::Category category,
                        int required_count = 1,
                        int reward_points  = 0);
    bool removeAchievement(const std::string& entity_id,
                           const std::string& ach_id);
    bool clearAchievements(const std::string& entity_id);

    // --- Progression ---
    bool progressAchievement(const std::string& entity_id,
                             const std::string& ach_id,
                             int amount = 1);

    // --- Configuration ---
    bool setRewardPoints(const std::string& entity_id,
                         const std::string& ach_id,
                         int points);

    // --- Queries ---
    int  getAchievementCount(const std::string& entity_id) const;
    int  getUnlockedCount(const std::string& entity_id) const;
    bool isUnlocked(const std::string& entity_id,
                    const std::string& ach_id) const;
    int  getProgress(const std::string& entity_id,
                     const std::string& ach_id) const;
    int  getRequiredCount(const std::string& entity_id,
                          const std::string& ach_id) const;
    int  getTotalRewardPoints(const std::string& entity_id) const;
    int  getTotalProgressCalls(const std::string& entity_id) const;
    int  getCountByCategory(const std::string& entity_id,
                            components::AchievementState::Category cat) const;
    bool hasAchievement(const std::string& entity_id,
                        const std::string& ach_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AchievementState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ACHIEVEMENT_SYSTEM_H
