#include "systems/sensor_dampening_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SensorDampeningSystem::SensorDampeningSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void SensorDampeningSystem::updateComponent(ecs::Entity& /*entity*/,
                                             components::SensorDampeningState& comp,
                                             float delta_time) {
    if (!comp.active_flag) return;
    comp.elapsed += delta_time;

    for (auto& d : comp.dampeners) {
        if (!d.active) continue;
        d.cycle_elapsed += delta_time;
        if (d.cycle_elapsed >= d.cycle_time) {
            d.cycle_elapsed -= d.cycle_time;
            comp.total_dampener_cycles++;
        }
    }
}

// ---------------------------------------------------------------------------
// Effective-value helper
// ---------------------------------------------------------------------------

void SensorDampeningSystem::recalcEffectiveValues(
        components::SensorDampeningState& comp) const {
    float range_factor = 1.0f;
    float res_factor   = 1.0f;
    for (const auto& d : comp.dampeners) {
        if (!d.active) continue;
        range_factor *= (1.0f - d.range_reduction);
        res_factor   *= (1.0f - d.scan_res_reduction);
    }
    // Clamp so values never go below 1% of base
    comp.effective_lock_range = comp.base_lock_range * std::max(range_factor, 0.01f);
    comp.effective_scan_res   = comp.base_scan_resolution * std::max(res_factor, 0.01f);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool SensorDampeningSystem::initialize(const std::string& entity_id,
                                        float base_lock_range,
                                        float base_scan_resolution) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SensorDampeningState>();
    comp->base_lock_range      = base_lock_range;
    comp->base_scan_resolution = base_scan_resolution;
    comp->effective_lock_range = base_lock_range;
    comp->effective_scan_res   = base_scan_resolution;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Dampener management
// ---------------------------------------------------------------------------

bool SensorDampeningSystem::applyDampener(const std::string& entity_id,
                                           const std::string& source_id,
                                           float range_reduction,
                                           float scan_res_reduction,
                                           float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (source_id.empty()) return false;
    if (range_reduction < 0.0f || range_reduction >= 1.0f) return false;
    if (scan_res_reduction < 0.0f || scan_res_reduction >= 1.0f) return false;
    if (cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->dampeners.size()) >= comp->max_dampeners) return false;

    for (const auto& d : comp->dampeners) {
        if (d.source_id == source_id) return false; // duplicate
    }

    components::SensorDampeningState::Dampener d;
    d.source_id           = source_id;
    d.range_reduction     = range_reduction;
    d.scan_res_reduction  = scan_res_reduction;
    d.cycle_time          = cycle_time;
    d.cycle_elapsed       = 0.0f;
    d.active              = true;
    comp->dampeners.push_back(d);
    comp->total_dampeners_applied++;
    recalcEffectiveValues(*comp);
    return true;
}

bool SensorDampeningSystem::removeDampener(const std::string& entity_id,
                                            const std::string& source_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->dampeners.begin(), comp->dampeners.end(),
        [&](const components::SensorDampeningState::Dampener& d) {
            return d.source_id == source_id;
        });
    if (it == comp->dampeners.end()) return false;
    comp->dampeners.erase(it);
    recalcEffectiveValues(*comp);
    return true;
}

bool SensorDampeningSystem::clearDampeners(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->dampeners.clear();
    comp->effective_lock_range = comp->base_lock_range;
    comp->effective_scan_res   = comp->base_scan_resolution;
    return true;
}

// ---------------------------------------------------------------------------
// Base value configuration
// ---------------------------------------------------------------------------

bool SensorDampeningSystem::setBaseLockRange(const std::string& entity_id,
                                              float range) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (range <= 0.0f) return false;
    comp->base_lock_range = range;
    recalcEffectiveValues(*comp);
    return true;
}

bool SensorDampeningSystem::setBaseScanResolution(const std::string& entity_id,
                                                   float res) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (res <= 0.0f) return false;
    comp->base_scan_resolution = res;
    recalcEffectiveValues(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int SensorDampeningSystem::getDampenerCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->dampeners.size()) : 0;
}

float SensorDampeningSystem::getEffectiveLockRange(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_lock_range : 0.0f;
}

float SensorDampeningSystem::getEffectiveScanResolution(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_scan_res : 0.0f;
}

float SensorDampeningSystem::getBaseLockRange(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->base_lock_range : 0.0f;
}

float SensorDampeningSystem::getBaseScanResolution(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->base_scan_resolution : 0.0f;
}

int SensorDampeningSystem::getTotalDampenersApplied(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_dampeners_applied : 0;
}

int SensorDampeningSystem::getTotalDampenerCycles(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_dampener_cycles : 0;
}

bool SensorDampeningSystem::isDampened(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& d : comp->dampeners) {
        if (d.active) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
