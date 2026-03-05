#ifndef NOVAFORGE_SYSTEMS_PLAYER_PROGRESSION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_PROGRESSION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks player XP, levels, and milestone achievements
 *
 * Players earn XP across 5 categories (combat, mining, exploration, industry, social).
 * Levels follow a scaling curve: each level requires 100 × level^1.5 XP.
 * Milestones are unlocked when category XP thresholds are met.
 */
class PlayerProgressionSystem : public ecs::SingleComponentSystem<components::PlayerProgression> {
public:
    explicit PlayerProgressionSystem(ecs::World* world);
    ~PlayerProgressionSystem() override = default;

    std::string getName() const override { return "PlayerProgressionSystem"; }

    // XP earning
    bool awardXP(const std::string& player_id, const std::string& category, float amount);

    // Initialization
    bool initProgression(const std::string& player_id);
    bool addMilestone(const std::string& player_id, const std::string& name,
                      const std::string& category, float xp_required);

    // Prestige
    bool prestige(const std::string& player_id);

    // Query API
    int getLevel(const std::string& player_id) const;
    float getTotalXP(const std::string& player_id) const;
    float getCategoryXP(const std::string& player_id, const std::string& category) const;
    float getLevelProgress(const std::string& player_id) const;
    int getMilestonesAchieved(const std::string& player_id) const;
    int getPrestigeLevel(const std::string& player_id) const;
    float getPrestigeMultiplier(const std::string& player_id) const;

    // Level calculation helper
    static float xpForLevel(int level);

protected:
    void updateComponent(ecs::Entity& entity, components::PlayerProgression& prog,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_PROGRESSION_SYSTEM_H
