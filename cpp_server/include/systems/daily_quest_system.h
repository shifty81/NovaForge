#ifndef NOVAFORGE_SYSTEMS_DAILY_QUEST_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DAILY_QUEST_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Daily repeatable quest tracking with 24-hour reset timer
 *
 * Manages a set of daily objectives that reset every 24 hours (configurable).
 * Objectives track a required count; each call to progressObjective increments
 * the current count.  When current_count reaches required_count the objective
 * is marked completed.  When all objectives are completed all_complete is set
 * and claimBonus() awards the bonus_reward.
 *
 * The reset_timer counts down each tick.  On expiry all objectives and flags
 * are reset, total_resets is incremented, and the timer is restarted.
 * If all objectives were completed that day, days_completed is also incremented.
 *
 * addObjective() is blocked while the timer is running (quest is active) to
 * prevent modification mid-day.  Use resetSession() to force an early reset.
 */
class DailyQuestSystem
    : public ecs::SingleComponentSystem<components::DailyQuestState> {
public:
    explicit DailyQuestSystem(ecs::World* world);
    ~DailyQuestSystem() override = default;

    std::string getName() const override { return "DailyQuestSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Objective management ---
    bool addObjective(const std::string& entity_id,
                      const std::string& obj_id,
                      const std::string& description,
                      int required_count = 1);
    bool removeObjective(const std::string& entity_id, const std::string& obj_id);

    // --- Progression ---
    bool progressObjective(const std::string& entity_id,
                           const std::string& obj_id,
                           int amount = 1);
    bool claimBonus(const std::string& entity_id);

    // --- Configuration ---
    bool setBonusReward(const std::string& entity_id, float reward);
    bool setResetDuration(const std::string& entity_id, float seconds);
    bool forceReset(const std::string& entity_id);

    // --- Queries ---
    int   getObjectiveCount(const std::string& entity_id) const;
    int   getCompletedObjectiveCount(const std::string& entity_id) const;
    bool  isObjectiveComplete(const std::string& entity_id,
                              const std::string& obj_id) const;
    int   getObjectiveProgress(const std::string& entity_id,
                               const std::string& obj_id) const;
    bool  isAllComplete(const std::string& entity_id) const;
    bool  isBonusClaimed(const std::string& entity_id) const;
    float getResetTimer(const std::string& entity_id) const;
    float getBonusReward(const std::string& entity_id) const;
    int   getDaysCompleted(const std::string& entity_id) const;
    int   getTotalResets(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::DailyQuestState& comp,
                         float delta_time) override;

private:
    static void performReset(components::DailyQuestState& comp, bool count_day);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DAILY_QUEST_SYSTEM_H
