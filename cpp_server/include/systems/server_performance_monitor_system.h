#ifndef NOVAFORGE_SYSTEMS_SERVER_PERFORMANCE_MONITOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SERVER_PERFORMANCE_MONITOR_SYSTEM_H

#include "ecs/system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Server performance monitor system (Phase 15)
 *
 * Server tick performance profiling with per-system timing, entity count
 * tracking, hot-path identification, and performance alerting.
 */
class ServerPerformanceMonitorSystem : public ecs::System {
public:
    explicit ServerPerformanceMonitorSystem(ecs::World* world);
    ~ServerPerformanceMonitorSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ServerPerformanceMonitorSystem"; }

    // Initialization
    bool initializeMonitor(const std::string& entity_id, const std::string& server_id,
                           float tick_budget_ms);

    // Recording
    bool recordSystemTiming(const std::string& entity_id, const std::string& system_name,
                            float time_ms);
    bool recordTickComplete(const std::string& entity_id, float total_time_ms,
                            int entity_count);

    // Query
    float getAverageTickTime(const std::string& entity_id) const;
    float getBudgetUtilization(const std::string& entity_id) const;
    std::string getSlowestSystem(const std::string& entity_id) const;
    int getHotPathCount(const std::string& entity_id) const;
    bool isAlertActive(const std::string& entity_id) const;

    // Reset
    bool resetMetrics(const std::string& entity_id);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SERVER_PERFORMANCE_MONITOR_SYSTEM_H
