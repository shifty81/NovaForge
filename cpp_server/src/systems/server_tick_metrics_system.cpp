#include "systems/server_tick_metrics_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ServerTickMetricsSystem::ServerTickMetricsSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ServerTickMetricsSystem::updateComponent(ecs::Entity& /*entity*/,
    components::ServerTickMetrics& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool ServerTickMetricsSystem::initialize(const std::string& entity_id,
    float target_tick_rate) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (target_tick_rate <= 0.0f) return false;

    auto comp = std::make_unique<components::ServerTickMetrics>();
    comp->target_tick_rate = target_tick_rate;
    comp->tick_budget_ms = 1000.0f / target_tick_rate;
    entity->addComponent(std::move(comp));
    return true;
}

bool ServerTickMetricsSystem::recordTick(const std::string& entity_id,
    float tick_duration_ms) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (tick_duration_ms < 0.0f) return false;

    comp->last_tick_ms = tick_duration_ms;
    comp->total_ticks++;

    // Running average
    comp->avg_tick_ms = comp->avg_tick_ms
        + (tick_duration_ms - comp->avg_tick_ms) / static_cast<float>(comp->total_ticks);

    comp->min_tick_ms = std::min(comp->min_tick_ms, tick_duration_ms);
    comp->max_tick_ms = std::max(comp->max_tick_ms, tick_duration_ms);

    if (tick_duration_ms > comp->tick_budget_ms) {
        comp->overtime_ticks++;
    }
    return true;
}

bool ServerTickMetricsSystem::recordPhase(const std::string& entity_id,
    const std::string& phase_name, float duration_ms) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (phase_name.empty()) return false;
    if (duration_ms < 0.0f) return false;

    // Update existing phase or add new one
    for (auto& phase : comp->phase_timers) {
        if (phase.phase_name == phase_name) {
            phase.duration_ms = duration_ms;
            return true;
        }
    }

    if (static_cast<int>(comp->phase_timers.size()) >= comp->max_phases) return false;

    components::ServerTickMetrics::PhaseTimer timer;
    timer.phase_name = phase_name;
    timer.duration_ms = duration_ms;
    comp->phase_timers.push_back(timer);
    return true;
}

bool ServerTickMetricsSystem::setEntityCount(const std::string& entity_id, int count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (count < 0) return false;

    comp->entity_count = count;
    comp->peak_entity_count = std::max(comp->peak_entity_count, count);
    return true;
}

bool ServerTickMetricsSystem::resetStats(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    comp->last_tick_ms = 0.0f;
    comp->avg_tick_ms = 0.0f;
    comp->min_tick_ms = 999.0f;
    comp->max_tick_ms = 0.0f;
    comp->total_ticks = 0;
    comp->overtime_ticks = 0;
    comp->phase_timers.clear();
    return true;
}

float ServerTickMetricsSystem::getLastTickMs(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->last_tick_ms : 0.0f;
}

float ServerTickMetricsSystem::getAvgTickMs(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->avg_tick_ms : 0.0f;
}

float ServerTickMetricsSystem::getMinTickMs(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->min_tick_ms : 0.0f;
}

float ServerTickMetricsSystem::getMaxTickMs(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_tick_ms : 0.0f;
}

int ServerTickMetricsSystem::getTotalTicks(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_ticks : 0;
}

int ServerTickMetricsSystem::getOvertimeTicks(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->overtime_ticks : 0;
}

float ServerTickMetricsSystem::getOvertimeRatio(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->total_ticks == 0) return 0.0f;
    return static_cast<float>(comp->overtime_ticks) / static_cast<float>(comp->total_ticks);
}

int ServerTickMetricsSystem::getEntityCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->entity_count : 0;
}

int ServerTickMetricsSystem::getPeakEntityCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->peak_entity_count : 0;
}

float ServerTickMetricsSystem::getTickBudgetMs(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->tick_budget_ms : 0.0f;
}

int ServerTickMetricsSystem::getPhaseCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->phase_timers.size()) : 0;
}

float ServerTickMetricsSystem::getPhaseTime(const std::string& entity_id,
    const std::string& phase_name) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& phase : comp->phase_timers) {
        if (phase.phase_name == phase_name) return phase.duration_ms;
    }
    return 0.0f;
}

} // namespace systems
} // namespace atlas
