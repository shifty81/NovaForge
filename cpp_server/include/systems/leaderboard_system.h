#ifndef NOVAFORGE_SYSTEMS_LEADERBOARD_SYSTEM_H
#define NOVAFORGE_SYSTEMS_LEADERBOARD_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Tracks player rankings and achievements
 *
 * Aggregates stats across categories, maintains sorted leaderboards,
 * defines achievements with unlock conditions, and awards them when
 * thresholds are met.
 */
class LeaderboardSystem : public ecs::SingleComponentSystem<components::Leaderboard> {
public:
    explicit LeaderboardSystem(ecs::World* world);
    ~LeaderboardSystem() override = default;

    std::string getName() const override { return "LeaderboardSystem"; }

    /**
     * @brief Record a kill for a player
     */
    void recordKill(const std::string& entity_id,
                    const std::string& player_id,
                    const std::string& player_name);

    /**
     * @brief Record Credits earned by a player
     */
    void recordIscEarned(const std::string& entity_id,
                         const std::string& player_id,
                         const std::string& player_name,
                         double amount);

    /**
     * @brief Record a mission completion
     */
    void recordMissionComplete(const std::string& entity_id,
                               const std::string& player_id,
                               const std::string& player_name);

    /**
     * @brief Record a tournament win
     */
    void recordTournamentWin(const std::string& entity_id,
                             const std::string& player_id,
                             const std::string& player_name);

    /**
     * @brief Record damage dealt by a player
     */
    void recordDamageDealt(const std::string& entity_id,
                           const std::string& player_id,
                           const std::string& player_name,
                           double amount);

    /**
     * @brief Get total kills for a player
     */
    int getPlayerKills(const std::string& entity_id,
                       const std::string& player_id);

    /**
     * @brief Get total Credits earned by a player
     */
    double getPlayerIscEarned(const std::string& entity_id,
                              const std::string& player_id);

    /**
     * @brief Get missions completed by a player
     */
    int getPlayerMissions(const std::string& entity_id,
                          const std::string& player_id);

    /**
     * @brief Define an achievement on the leaderboard
     */
    void defineAchievement(const std::string& entity_id,
                           const std::string& achievement_id,
                           const std::string& name,
                           const std::string& description,
                           const std::string& category,
                           const std::string& stat_key,
                           int requirement);

    /**
     * @brief Check and award achievements for a player
     * @return number of newly unlocked achievements
     */
    int checkAchievements(const std::string& entity_id,
                          const std::string& player_id,
                          float current_time = 0.0f);

    /**
     * @brief Check if a player has unlocked a specific achievement
     */
    bool hasAchievement(const std::string& entity_id,
                        const std::string& player_id,
                        const std::string& achievement_id);

    /**
     * @brief Get total number of achievements unlocked by a player
     */
    int getPlayerAchievementCount(const std::string& entity_id,
                                  const std::string& player_id);

    /**
     * @brief Get the number of entries on the leaderboard
     */
    int getEntryCount(const std::string& entity_id);

    /**
     * @brief Get ranked player IDs sorted by total kills (descending)
     */
    std::vector<std::string> getRankingByKills(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::Leaderboard& board, float delta_time) override;

private:
    /**
     * @brief Find or create a player entry on the leaderboard
     */
    int findOrCreateEntry(const std::string& entity_id,
                          const std::string& player_id,
                          const std::string& player_name);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_LEADERBOARD_SYSTEM_H
