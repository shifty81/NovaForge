#include "systems/turret_ai_system.h"
#include "ecs/world.h"

#include <cmath>

namespace atlas {
namespace systems {

TurretAISystem::TurretAISystem(ecs::World* world)
    : ecs::System(world) {}

// ── Static helpers ──────────────────────────────────────────────────

bool TurretAISystem::isWithinArc(float bearing_deg,
                                  float turret_direction_deg,
                                  float arc_degrees) {
    float half_arc = arc_degrees * 0.5f;
    float diff = bearing_deg - turret_direction_deg;

    // Normalize to [-180, 180]
    diff = std::fmod(diff + 180.0f, 360.0f);
    if (diff < 0.0f) diff += 360.0f;
    diff -= 180.0f;

    return std::fabs(diff) <= half_arc;
}

float TurretAISystem::computeTrackingPenalty(float angular_velocity,
                                              float tracking_speed) {
    if (tracking_speed <= 0.0f) return 0.0f;
    float ratio = angular_velocity / tracking_speed;
    // Sigmoid-style falloff: penalty = 1 / (1 + ratio^2)
    return 1.0f / (1.0f + ratio * ratio);
}

// ── Update ──────────────────────────────────────────────────────────

void TurretAISystem::update(float delta_time) {
    auto entities = world_->getEntities<components::TurretAIState>();
    for (auto* entity : entities) {
        auto* turret = entity->getComponent<components::TurretAIState>();
        if (!turret) continue;

        // Count down cooldown
        if (turret->cooldown_remaining > 0.0f) {
            turret->cooldown_remaining -= delta_time;
            if (turret->cooldown_remaining < 0.0f)
                turret->cooldown_remaining = 0.0f;
        }

        // If engaged and cooldown expired, fire
        if (turret->engaged && turret->cooldown_remaining <= 0.0f
            && !turret->target_entity_id.empty()) {

            float tracking_hit = computeTrackingPenalty(
                turret->angular_velocity, turret->tracking_speed);
            float damage = turret->base_damage * tracking_hit;

            turret->total_damage_dealt += damage;
            turret->shots_fired++;
            turret->cooldown_remaining = (turret->rate_of_fire > 0.0f)
                ? 1.0f / turret->rate_of_fire
                : 1.0f;
        }
    }
}

// ── Queries ─────────────────────────────────────────────────────────

int TurretAISystem::getEngagedTurretCount(const std::string& entity_id) const {
    // Count all TurretAIState entities whose id starts with the ship id
    int count = 0;
    auto entities = world_->getEntities<components::TurretAIState>();
    for (auto* entity : entities) {
        if (entity->getId().find(entity_id) == 0) {
            auto* turret = entity->getComponent<components::TurretAIState>();
            if (turret && turret->engaged) count++;
        }
    }
    return count;
}

float TurretAISystem::getActiveDPS(const std::string& entity_id) const {
    float dps = 0.0f;
    auto entities = world_->getEntities<components::TurretAIState>();
    for (auto* entity : entities) {
        if (entity->getId().find(entity_id) == 0) {
            auto* turret = entity->getComponent<components::TurretAIState>();
            if (turret && turret->engaged) {
                float tracking_hit = computeTrackingPenalty(
                    turret->angular_velocity, turret->tracking_speed);
                dps += turret->base_damage * turret->rate_of_fire * tracking_hit;
            }
        }
    }
    return dps;
}

} // namespace systems
} // namespace atlas
