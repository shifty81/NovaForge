#ifndef NOVAFORGE_SYSTEMS_SESSION_SUMMARY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SESSION_SUMMARY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Produces an end-of-session after-action report with performance metrics
 *
 * Aggregates combat, economy, exploration, and mission data into a structured
 * summary: ISK/hour, kills, damage dealt/received, objectives completed, and
 * letter-grade performance rating. Triggers when the session is finalized.
 */
class SessionSummarySystem : public ecs::SingleComponentSystem<components::SessionSummaryState> {
public:
    explicit SessionSummarySystem(ecs::World* world);
    ~SessionSummarySystem() override = default;

    std::string getName() const override { return "SessionSummarySystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& player_id);

    // Stat accumulation
    bool recordStat(const std::string& entity_id, const std::string& stat_key, double value);
    double getStat(const std::string& entity_id, const std::string& stat_key) const;
    int getStatCount(const std::string& entity_id) const;

    // Category-based tracking
    bool addCategoryStat(const std::string& entity_id, const std::string& category,
                         const std::string& stat_key, double value);
    double getCategoryStat(const std::string& entity_id, const std::string& category,
                           const std::string& stat_key) const;
    int getCategoryCount(const std::string& entity_id) const;

    // Finalization
    bool finalizeReport(const std::string& entity_id, float end_time);
    bool isFinalized(const std::string& entity_id) const;
    float getSessionDuration(const std::string& entity_id) const;

    // Derived metrics
    double getIskPerHour(const std::string& entity_id) const;
    std::string getPerformanceGrade(const std::string& entity_id) const;
    double getTotalDamageDealt(const std::string& entity_id) const;
    double getTotalDamageReceived(const std::string& entity_id) const;
    int getObjectivesCompleted(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SessionSummaryState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SESSION_SUMMARY_SYSTEM_H
