#ifndef NOVAFORGE_SYSTEMS_DYNAMIC_DIFFICULTY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DYNAMIC_DIFFICULTY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

/**
 * DynamicDifficultySystem — scales PvE encounter difficulty based on
 * player performance and ship power level.
 *
 * Reads/Writes DynamicDifficultyState component.
 *
 * Design:
 *   - Tracks encounter wins/losses and consecutive streaks.
 *   - Computes encounter_multiplier from combat rating, ship power,
 *     and win/loss streaks.
 *   - Clamps difficulty within [min_difficulty, max_difficulty].
 *   - Gradual adjustment: difficulty moves by adjustment_rate per event.
 */
class DynamicDifficultySystem : public ecs::SingleComponentSystem<components::DynamicDifficultyState> {
public:
    explicit DynamicDifficultySystem(ecs::World* world);
    ~DynamicDifficultySystem() override = default;

    std::string getName() const override { return "DynamicDifficultySystem"; }

    /// Record an encounter win for a player entity
    bool recordWin(const std::string& entity_id);

    /// Record an encounter loss for a player entity
    bool recordLoss(const std::string& entity_id);

    /// Recompute the encounter multiplier from current state
    float computeDifficulty(const std::string& entity_id) const;

    /// Query helpers
    float getEncounterMultiplier(const std::string& entity_id) const;
    int getConsecutiveWins(const std::string& entity_id) const;
    int getConsecutiveLosses(const std::string& entity_id) const;
    int getTotalEncounters(const std::string& entity_id) const;

    /// Static helper: compute multiplier from inputs
    static float calculateMultiplier(float combat_rating, float ship_power,
                                     int consecutive_wins, int consecutive_losses,
                                     float base_difficulty, float adjustment_rate,
                                     float min_diff, float max_diff);

protected:
    void updateComponent(ecs::Entity& entity, components::DynamicDifficultyState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DYNAMIC_DIFFICULTY_SYSTEM_H
