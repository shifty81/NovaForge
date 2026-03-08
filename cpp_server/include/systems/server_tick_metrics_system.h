#ifndef NOVAFORGE_SYSTEMS_SERVER_TICK_METRICS_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SERVER_TICK_METRICS_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Server tick performance monitoring system
 *
 * Tracks per-tick timing, entity counts, and performance budgets to
 * maintain the 20 Hz target tick rate.  Records min/max/avg tick
 * durations, overtime warnings, and per-system phase timings.
 * Essential for the performance profiling milestone.
 */
class ServerTickMetricsSystem : public ecs::SingleComponentSystem<components::ServerTickMetrics> {
public:
    explicit ServerTickMetricsSystem(ecs::World* world);
    ~ServerTickMetricsSystem() override = default;

    std::string getName() const override { return "ServerTickMetricsSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, float target_tick_rate);

    // Tick recording
    bool recordTick(const std::string& entity_id, float tick_duration_ms);
    bool recordPhase(const std::string& entity_id, const std::string& phase_name,
                     float duration_ms);
    bool setEntityCount(const std::string& entity_id, int count);
    bool resetStats(const std::string& entity_id);

    // Queries
    float getLastTickMs(const std::string& entity_id) const;
    float getAvgTickMs(const std::string& entity_id) const;
    float getMinTickMs(const std::string& entity_id) const;
    float getMaxTickMs(const std::string& entity_id) const;
    int getTotalTicks(const std::string& entity_id) const;
    int getOvertimeTicks(const std::string& entity_id) const;
    float getOvertimeRatio(const std::string& entity_id) const;
    int getEntityCount(const std::string& entity_id) const;
    int getPeakEntityCount(const std::string& entity_id) const;
    float getTickBudgetMs(const std::string& entity_id) const;
    int getPhaseCount(const std::string& entity_id) const;
    float getPhaseTime(const std::string& entity_id, const std::string& phase_name) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ServerTickMetrics& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SERVER_TICK_METRICS_SYSTEM_H
