#include "systems/server_performance_monitor_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ServerPerformanceMonitorSystem::ServerPerformanceMonitorSystem(ecs::World* world)
    : System(world) {
}

void ServerPerformanceMonitorSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::ServerPerformanceMetrics>();
    for (auto* entity : entities) {
        auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
        if (!metrics) continue;

        // Recompute budget utilization
        if (metrics->tick_budget_ms > 0.0f && metrics->total_ticks > 0) {
            metrics->budget_utilization = metrics->avg_tick_time_ms / metrics->tick_budget_ms;
        }

        // Alert check
        metrics->alert_active = (metrics->budget_utilization >= metrics->alert_threshold);

        // Find slowest system and hot paths
        float max_avg = 0.0f;
        metrics->hot_path_count = 0;
        metrics->slowest_system.clear();

        float hot_threshold = metrics->tick_budget_ms * 0.25f;
        for (const auto& timing : metrics->system_timings) {
            if (timing.avg_time_ms > max_avg) {
                max_avg = timing.avg_time_ms;
                metrics->slowest_system = timing.system_name;
            }
            if (timing.avg_time_ms > hot_threshold) {
                metrics->hot_path_count++;
            }
        }
    }
}

bool ServerPerformanceMonitorSystem::initializeMonitor(const std::string& entity_id,
                                                        const std::string& server_id,
                                                        float tick_budget_ms) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::ServerPerformanceMetrics>();
    if (existing) return false;

    auto comp = std::make_unique<components::ServerPerformanceMetrics>();
    comp->monitor_id = entity_id;
    comp->server_id = server_id;
    comp->tick_budget_ms = tick_budget_ms;
    entity->addComponent(std::move(comp));
    return true;
}

bool ServerPerformanceMonitorSystem::recordSystemTiming(const std::string& entity_id,
                                                         const std::string& system_name,
                                                         float time_ms) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
    if (!metrics) return false;

    auto* timing = metrics->findTiming(system_name);
    if (!timing) {
        components::ServerPerformanceMetrics::SystemTiming st;
        st.system_name = system_name;
        st.last_time_ms = time_ms;
        st.avg_time_ms = time_ms;
        st.max_time_ms = time_ms;
        st.sample_count = 1;
        metrics->system_timings.push_back(st);
    } else {
        timing->last_time_ms = time_ms;
        timing->max_time_ms = std::max(timing->max_time_ms, time_ms);
        timing->sample_count++;
        // Running average
        float n = static_cast<float>(timing->sample_count);
        timing->avg_time_ms = timing->avg_time_ms * ((n - 1.0f) / n) + time_ms / n;
    }
    return true;
}

bool ServerPerformanceMonitorSystem::recordTickComplete(const std::string& entity_id,
                                                         float total_time_ms,
                                                         int entity_count) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
    if (!metrics) return false;

    metrics->total_ticks++;
    metrics->total_tick_time_ms = total_time_ms;
    metrics->entity_count = entity_count;
    metrics->max_tick_time_ms = std::max(metrics->max_tick_time_ms, total_time_ms);

    // Running average
    float n = static_cast<float>(metrics->total_ticks);
    metrics->avg_tick_time_ms = metrics->avg_tick_time_ms * ((n - 1.0f) / n) + total_time_ms / n;

    if (total_time_ms > metrics->tick_budget_ms) {
        metrics->ticks_over_budget++;
    }

    // Update budget utilization
    if (metrics->tick_budget_ms > 0.0f) {
        metrics->budget_utilization = metrics->avg_tick_time_ms / metrics->tick_budget_ms;
    }

    return true;
}

float ServerPerformanceMonitorSystem::getAverageTickTime(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
    if (!metrics) return 0.0f;

    return metrics->avg_tick_time_ms;
}

float ServerPerformanceMonitorSystem::getBudgetUtilization(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
    if (!metrics) return 0.0f;

    return metrics->budget_utilization;
}

std::string ServerPerformanceMonitorSystem::getSlowestSystem(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";

    auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
    if (!metrics) return "";

    return metrics->slowest_system;
}

int ServerPerformanceMonitorSystem::getHotPathCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
    if (!metrics) return 0;

    return metrics->hot_path_count;
}

bool ServerPerformanceMonitorSystem::isAlertActive(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
    if (!metrics) return false;

    return metrics->alert_active;
}

bool ServerPerformanceMonitorSystem::resetMetrics(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* metrics = entity->getComponent<components::ServerPerformanceMetrics>();
    if (!metrics) return false;

    metrics->system_timings.clear();
    metrics->entity_count = 0;
    metrics->component_count = 0;
    metrics->total_tick_time_ms = 0.0f;
    metrics->avg_tick_time_ms = 0.0f;
    metrics->max_tick_time_ms = 0.0f;
    metrics->ticks_over_budget = 0;
    metrics->total_ticks = 0;
    metrics->budget_utilization = 0.0f;
    metrics->alert_active = false;
    metrics->hot_path_count = 0;
    metrics->slowest_system.clear();

    return true;
}

} // namespace systems
} // namespace atlas
