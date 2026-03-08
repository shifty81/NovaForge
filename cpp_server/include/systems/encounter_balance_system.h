#ifndef NOVAFORGE_SYSTEMS_ENCOUNTER_BALANCE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ENCOUNTER_BALANCE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Dynamic encounter difficulty and reward scaling
 *
 * Adjusts difficulty_multiplier and reward_multiplier each tick based on
 * player fleet size, average ship class, and cumulative kill count.
 * Ensures the vertical-slice gameplay loop stays challenging for solo
 * players and scales appropriately for co-op groups.
 *
 * Scaling rules:
 *   difficulty = base_difficulty * (1 + 0.25*(player_count-1)) * (1 + 0.1*avg_ship_class)
 *   reward     = base_reward * difficulty_multiplier * (1 + 0.01*total_kills)
 */
class EncounterBalanceSystem : public ecs::SingleComponentSystem<components::EncounterBalanceState> {
public:
    explicit EncounterBalanceSystem(ecs::World* world);
    ~EncounterBalanceSystem() override = default;

    std::string getName() const override { return "EncounterBalanceSystem"; }

    bool initialize(const std::string& entity_id, const std::string& encounter_id,
                    float base_difficulty, float base_reward);
    bool setPlayerCount(const std::string& entity_id, int count);
    bool setAvgShipClass(const std::string& entity_id, int ship_class);
    bool addKills(const std::string& entity_id, int kills);
    bool recalculate(const std::string& entity_id);

    float getDifficultyMultiplier(const std::string& entity_id) const;
    float getRewardMultiplier(const std::string& entity_id) const;
    float getEffectiveReward(const std::string& entity_id) const;
    int   getRecalcCount(const std::string& entity_id) const;
    int   getPlayerCount(const std::string& entity_id) const;
    int   getAvgShipClass(const std::string& entity_id) const;
    int   getTotalKills(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::EncounterBalanceState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ENCOUNTER_BALANCE_SYSTEM_H
