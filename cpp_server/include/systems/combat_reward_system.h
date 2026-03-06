#ifndef NOVAFORGE_SYSTEMS_COMBAT_REWARD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_COMBAT_REWARD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Awards XP and credits from NPC kills
 *
 * Bridges the fight → reward loop in the vertical slice.  When an NPC
 * is destroyed, kill data is recorded as a pending reward.  The system
 * periodically flushes pending rewards, crediting the player's
 * combat XP tally and wallet balance.
 */
class CombatRewardSystem : public ecs::SingleComponentSystem<components::CombatReward> {
public:
    explicit CombatRewardSystem(ecs::World* world);
    ~CombatRewardSystem() override = default;

    std::string getName() const override { return "CombatRewardSystem"; }

    /**
     * @brief Initialize reward tracking for an entity
     */
    bool initializeRewards(const std::string& entity_id,
                           float flush_interval = 2.0f);

    /**
     * @brief Record a kill and queue reward
     * @return true if reward was queued
     */
    bool recordKill(const std::string& entity_id,
                    const std::string& target_id,
                    const std::string& target_name,
                    float xp,
                    double credits,
                    const std::string& loot_table = "");

    /**
     * @brief Get total credits awarded since init
     */
    double getTotalCredits(const std::string& entity_id) const;

    /**
     * @brief Get total XP awarded since init
     */
    float getTotalXP(const std::string& entity_id) const;

    /**
     * @brief Get total kills recorded
     */
    int getTotalKills(const std::string& entity_id) const;

    /**
     * @brief Get number of pending (unflushed) rewards
     */
    int getPendingCount(const std::string& entity_id) const;

    /**
     * @brief Force flush all pending rewards immediately
     * @return number of rewards flushed
     */
    int flushRewards(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::CombatReward& reward,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_COMBAT_REWARD_SYSTEM_H
