#include "systems/turret_tracking_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/combat_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

TurretTrackingSystem::TurretTrackingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

float TurretTrackingSystem::computeAccuracy(const components::TurretTrackingState& state) const {
    if (!state.locked) return 0.0f;
    // Accuracy = tracking_speed / (tracking_speed + angular_velocity)
    // This gives 1.0 when target is stationary and decreases as target angular velocity increases
    float denom = state.tracking_speed + state.angular_velocity;
    if (denom <= 0.0f) return 0.0f;
    return std::min(1.0f, state.tracking_speed / denom);
}

void TurretTrackingSystem::updateComponent(ecs::Entity& entity,
    components::TurretTrackingState& state, float delta_time) {
    if (!state.active) return;

    if (state.locked) {
        // Rotate turret toward target angle
        float angle_diff = state.target_angle - state.current_angle;
        float max_rotation = state.tracking_speed * delta_time;
        if (std::abs(angle_diff) <= max_rotation) {
            state.current_angle = state.target_angle;
        } else if (angle_diff > 0) {
            state.current_angle += max_rotation;
        } else {
            state.current_angle -= max_rotation;
        }

        state.accuracy = computeAccuracy(state);
        state.damage_multiplier = state.accuracy;
    }

    state.elapsed += delta_time;
}

bool TurretTrackingSystem::initialize(const std::string& entity_id,
    const std::string& turret_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (turret_id.empty()) return false;
    auto comp = std::make_unique<components::TurretTrackingState>();
    comp->turret_id = turret_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool TurretTrackingSystem::lockTarget(const std::string& entity_id,
    const std::string& target_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (target_id.empty()) return false;
    state->target_id = target_id;
    state->locked = true;
    return true;
}

bool TurretTrackingSystem::unlockTarget(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->locked) return false;
    state->target_id.clear();
    state->locked = false;
    state->accuracy = 0.0f;
    state->damage_multiplier = 0.0f;
    return true;
}

bool TurretTrackingSystem::setTrackingSpeed(const std::string& entity_id, float speed) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->tracking_speed = std::max(0.01f, std::min(10.0f, speed));
    return true;
}

bool TurretTrackingSystem::setOptimalRange(const std::string& entity_id, float range) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->optimal_range = std::max(100.0f, std::min(100000.0f, range));
    return true;
}

bool TurretTrackingSystem::setFalloffRange(const std::string& entity_id, float range) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->falloff_range = std::max(0.0f, std::min(50000.0f, range));
    return true;
}

bool TurretTrackingSystem::setTargetAngularVelocity(const std::string& entity_id, float angular_vel) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->angular_velocity = std::max(0.0f, angular_vel);
    return true;
}

bool TurretTrackingSystem::fireShot(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state || !state->locked) return false;
    state->total_shots_fired++;
    // Determine hit based on accuracy
    if (state->accuracy >= 0.5f) {
        state->total_hits++;
    }
    return true;
}

float TurretTrackingSystem::getAccuracy(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->accuracy : 0.0f;
}

float TurretTrackingSystem::getDamageMultiplier(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->damage_multiplier : 0.0f;
}

float TurretTrackingSystem::getHitRate(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->hitRate() : 0.0f;
}

bool TurretTrackingSystem::isLocked(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->locked : false;
}

int TurretTrackingSystem::getTotalShots(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_shots_fired : 0;
}

int TurretTrackingSystem::getTotalHits(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_hits : 0;
}

std::string TurretTrackingSystem::getTargetId(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->target_id : "";
}

} // namespace systems
} // namespace atlas
