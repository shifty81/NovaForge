#ifndef NOVAFORGE_SYSTEMS_PLAYER_SESSION_STATS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLAYER_SESSION_STATS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks per-session player performance statistics
 *
 * Records cumulative statistics for the active play session including combat
 * performance (kills, losses, damage), economy activity (ISK earned/spent,
 * loot, trades), and travel metrics (distance, jumps, warps).
 *
 * startSession() resets per-session counters and increments total_sessions.
 * endSession() deactivates the timer without clearing data so the summary
 * remains available for display.
 * resetSession() wipes all per-session counters and re-activates the timer.
 *
 * getKDRatio() returns kills / losses (0 if no losses, returns kills as float).
 * getNetISK()  returns isk_earned - isk_spent.
 */
class PlayerSessionStatsSystem
    : public ecs::SingleComponentSystem<components::PlayerSessionStats> {
public:
    explicit PlayerSessionStatsSystem(ecs::World* world);
    ~PlayerSessionStatsSystem() override = default;

    std::string getName() const override { return "PlayerSessionStatsSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);
    bool startSession(const std::string& entity_id);
    bool endSession(const std::string& entity_id);
    bool resetSession(const std::string& entity_id);

    // --- Combat recording ---
    bool recordKill(const std::string& entity_id);
    bool recordLoss(const std::string& entity_id);
    bool recordDamageDealt(const std::string& entity_id, float amount);
    bool recordDamageReceived(const std::string& entity_id, float amount);
    bool recordAssist(const std::string& entity_id);

    // --- Economy recording ---
    bool recordIskEarned(const std::string& entity_id, float amount);
    bool recordIskSpent(const std::string& entity_id, float amount);
    bool recordTradeCompleted(const std::string& entity_id);
    bool recordItemLooted(const std::string& entity_id, int count = 1);

    // --- Travel recording ---
    bool recordDistanceTraveled(const std::string& entity_id, float au);
    bool recordJump(const std::string& entity_id);
    bool recordWarp(const std::string& entity_id);

    // --- Queries ---
    int   getKills(const std::string& entity_id) const;
    int   getLosses(const std::string& entity_id) const;
    float getDamageDealt(const std::string& entity_id) const;
    float getDamageReceived(const std::string& entity_id) const;
    int   getAssists(const std::string& entity_id) const;
    float getIskEarned(const std::string& entity_id) const;
    float getIskSpent(const std::string& entity_id) const;
    float getNetISK(const std::string& entity_id) const;
    int   getTradesCompleted(const std::string& entity_id) const;
    int   getItemsLooted(const std::string& entity_id) const;
    float getDistanceTraveled(const std::string& entity_id) const;
    int   getJumpsMade(const std::string& entity_id) const;
    int   getWarpsMade(const std::string& entity_id) const;
    float getElapsedTime(const std::string& entity_id) const;
    float getKDRatio(const std::string& entity_id) const;
    int   getTotalSessions(const std::string& entity_id) const;
    bool  isActive(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::PlayerSessionStats& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLAYER_SESSION_STATS_SYSTEM_H
