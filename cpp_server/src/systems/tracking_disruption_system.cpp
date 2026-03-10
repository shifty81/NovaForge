#include "systems/tracking_disruption_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

TrackingDisruptionSystem::TrackingDisruptionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void TrackingDisruptionSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::TrackingDisruptionState& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& td : comp.tracking_disruptors) {
        if (!td.active) continue;
        td.cycle_elapsed += delta_time;
        if (td.cycle_elapsed >= td.cycle_time) td.cycle_elapsed -= td.cycle_time;
    }
    for (auto& gd : comp.guidance_disruptors) {
        if (!gd.active) continue;
        gd.cycle_elapsed += delta_time;
        if (gd.cycle_elapsed >= gd.cycle_time) gd.cycle_elapsed -= gd.cycle_time;
    }
}

// ---------------------------------------------------------------------------
// Effective-value helper
// ---------------------------------------------------------------------------

void TrackingDisruptionSystem::recalcEffectiveValues(
        components::TrackingDisruptionState& comp) const {
    // Tracking disruptors — additive reductions, clamped
    float track_red = 0.0f;
    float range_red = 0.0f;
    for (const auto& td : comp.tracking_disruptors) {
        if (td.active) {
            track_red += td.tracking_speed_reduction;
            range_red += td.optimal_range_reduction;
        }
    }
    track_red = std::min(track_red, 0.99f);
    range_red = std::min(range_red, 0.99f);
    comp.effective_tracking_speed = comp.base_tracking_speed * (1.0f - track_red);
    comp.effective_optimal_range  = comp.base_optimal_range  * (1.0f - range_red);

    // Guidance disruptors — additive
    float radius_inc = 0.0f;
    float vel_red    = 0.0f;
    for (const auto& gd : comp.guidance_disruptors) {
        if (gd.active) {
            radius_inc += gd.explosion_radius_increase;
            vel_red    += gd.explosion_velocity_reduction;
        }
    }
    vel_red = std::min(vel_red, 0.99f);
    comp.effective_explosion_radius   = comp.base_explosion_radius * (1.0f + radius_inc);
    comp.effective_explosion_velocity = comp.base_explosion_velocity * (1.0f - vel_red);
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool TrackingDisruptionSystem::initialize(const std::string& entity_id,
                                           float base_tracking_speed,
                                           float base_optimal_range,
                                           float base_explosion_radius,
                                           float base_explosion_velocity) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::TrackingDisruptionState>();
    comp->base_tracking_speed       = base_tracking_speed;
    comp->base_optimal_range        = base_optimal_range;
    comp->base_explosion_radius     = base_explosion_radius;
    comp->base_explosion_velocity   = base_explosion_velocity;
    comp->effective_tracking_speed  = base_tracking_speed;
    comp->effective_optimal_range   = base_optimal_range;
    comp->effective_explosion_radius   = base_explosion_radius;
    comp->effective_explosion_velocity = base_explosion_velocity;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Tracking disruptor management
// ---------------------------------------------------------------------------

bool TrackingDisruptionSystem::applyTrackingDisruptor(
        const std::string& entity_id,
        const std::string& source_id,
        float tracking_speed_reduction,
        float optimal_range_reduction,
        float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (source_id.empty()) return false;
    if (tracking_speed_reduction < 0.0f || optimal_range_reduction < 0.0f) return false;
    if (cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->tracking_disruptors.size()) +
        static_cast<int>(comp->guidance_disruptors.size()) >= comp->max_disruptors) return false;

    for (const auto& td : comp->tracking_disruptors) {
        if (td.source_id == source_id) return false;
    }

    components::TrackingDisruptionState::TrackingDisruptor td;
    td.source_id                  = source_id;
    td.tracking_speed_reduction   = tracking_speed_reduction;
    td.optimal_range_reduction    = optimal_range_reduction;
    td.cycle_time                 = cycle_time;
    td.cycle_elapsed              = 0.0f;
    td.active                     = true;
    comp->tracking_disruptors.push_back(td);
    comp->total_tracking_disruptors_applied++;
    recalcEffectiveValues(*comp);
    return true;
}

bool TrackingDisruptionSystem::removeTrackingDisruptor(
        const std::string& entity_id,
        const std::string& source_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->tracking_disruptors.begin(),
                            comp->tracking_disruptors.end(),
        [&](const components::TrackingDisruptionState::TrackingDisruptor& td) {
            return td.source_id == source_id;
        });
    if (it == comp->tracking_disruptors.end()) return false;
    comp->tracking_disruptors.erase(it);
    recalcEffectiveValues(*comp);
    return true;
}

bool TrackingDisruptionSystem::clearTrackingDisruptors(
        const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->tracking_disruptors.clear();
    recalcEffectiveValues(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Guidance disruptor management
// ---------------------------------------------------------------------------

bool TrackingDisruptionSystem::applyGuidanceDisruptor(
        const std::string& entity_id,
        const std::string& source_id,
        float explosion_radius_increase,
        float explosion_velocity_reduction,
        float cycle_time) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (source_id.empty()) return false;
    if (explosion_radius_increase < 0.0f || explosion_velocity_reduction < 0.0f) return false;
    if (cycle_time <= 0.0f) return false;
    if (static_cast<int>(comp->tracking_disruptors.size()) +
        static_cast<int>(comp->guidance_disruptors.size()) >= comp->max_disruptors) return false;

    for (const auto& gd : comp->guidance_disruptors) {
        if (gd.source_id == source_id) return false;
    }

    components::TrackingDisruptionState::GuidanceDisruptor gd;
    gd.source_id                    = source_id;
    gd.explosion_radius_increase    = explosion_radius_increase;
    gd.explosion_velocity_reduction = explosion_velocity_reduction;
    gd.cycle_time                   = cycle_time;
    gd.cycle_elapsed                = 0.0f;
    gd.active                       = true;
    comp->guidance_disruptors.push_back(gd);
    comp->total_guidance_disruptors_applied++;
    recalcEffectiveValues(*comp);
    return true;
}

bool TrackingDisruptionSystem::removeGuidanceDisruptor(
        const std::string& entity_id,
        const std::string& source_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->guidance_disruptors.begin(),
                            comp->guidance_disruptors.end(),
        [&](const components::TrackingDisruptionState::GuidanceDisruptor& gd) {
            return gd.source_id == source_id;
        });
    if (it == comp->guidance_disruptors.end()) return false;
    comp->guidance_disruptors.erase(it);
    recalcEffectiveValues(*comp);
    return true;
}

bool TrackingDisruptionSystem::clearGuidanceDisruptors(
        const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->guidance_disruptors.clear();
    recalcEffectiveValues(*comp);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

float TrackingDisruptionSystem::getEffectiveTrackingSpeed(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_tracking_speed : 0.0f;
}

float TrackingDisruptionSystem::getEffectiveOptimalRange(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_optimal_range : 0.0f;
}

int TrackingDisruptionSystem::getTrackingDisruptorCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->tracking_disruptors.size()) : 0;
}

float TrackingDisruptionSystem::getEffectiveExplosionRadius(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_explosion_radius : 0.0f;
}

float TrackingDisruptionSystem::getEffectiveExplosionVelocity(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effective_explosion_velocity : 0.0f;
}

int TrackingDisruptionSystem::getGuidanceDisruptorCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->guidance_disruptors.size()) : 0;
}

int TrackingDisruptionSystem::getTotalTrackingDisruptorsApplied(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_tracking_disruptors_applied : 0;
}

int TrackingDisruptionSystem::getTotalGuidanceDisruptorsApplied(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_guidance_disruptors_applied : 0;
}

bool TrackingDisruptionSystem::isDisrupted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& td : comp->tracking_disruptors) {
        if (td.active) return true;
    }
    for (const auto& gd : comp->guidance_disruptors) {
        if (gd.active) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
