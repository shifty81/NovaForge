#include "systems/jump_drive_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

JumpDriveSystem::JumpDriveSystem(ecs::World* world)
    : System(world) {
}

void JumpDriveSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::JumpDriveState>();
    for (auto* entity : entities) {
        auto* jd = entity->getComponent<components::JumpDriveState>();
        if (!jd) continue;

        // Always decay fatigue
        if (jd->fatigue_hours > 0.0f) {
            jd->fatigue_hours = std::max(0.0f, jd->fatigue_hours - jd->fatigue_decay_rate * delta_time);
        }

        switch (jd->phase) {
            case components::JumpDriveState::JumpPhase::SpoolingUp: {
                jd->phase_timer += delta_time;
                if (jd->phase_timer >= jd->spool_time) {
                    // Consume fuel
                    jd->current_fuel -= jd->fuel_per_ly * jd->jump_distance_ly;
                    if (jd->current_fuel < 0.0f) jd->current_fuel = 0.0f;
                    // Increment jump stats
                    jd->total_jumps++;
                    jd->fatigue_hours += jd->fatigue_per_jump;
                    if (jd->fatigue_hours > jd->max_fatigue) jd->fatigue_hours = jd->max_fatigue;
                    // Transition to Jumping
                    jd->phase = components::JumpDriveState::JumpPhase::Jumping;
                    jd->phase_timer = 0.0f;
                }
                break;
            }
            case components::JumpDriveState::JumpPhase::Jumping: {
                // Jump is instant in simulation — transition to Cooldown
                jd->phase = components::JumpDriveState::JumpPhase::Cooldown;
                jd->phase_timer = 0.0f;
                break;
            }
            case components::JumpDriveState::JumpPhase::Cooldown: {
                jd->phase_timer += delta_time;
                if (jd->phase_timer >= jd->cooldown_time) {
                    jd->phase = components::JumpDriveState::JumpPhase::Idle;
                    jd->phase_timer = 0.0f;
                }
                break;
            }
            default:
                break;
        }
    }
}

bool JumpDriveSystem::initiateJump(const std::string& entity_id, const std::string& destination, float distance_ly, const std::string& cyno_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return false;

    if (jd->phase != components::JumpDriveState::JumpPhase::Idle) return false;
    if (jd->fatigue_hours >= jd->max_fatigue) return false;

    float fuel_needed = jd->fuel_per_ly * distance_ly;
    if (jd->current_fuel < fuel_needed) return false;

    float effective_range = (jd->max_fatigue > 0.0f)
        ? jd->max_range_ly * (1.0f - jd->fatigue_hours / jd->max_fatigue)
        : jd->max_range_ly;
    if (distance_ly > effective_range) return false;

    if (jd->requires_cyno && cyno_id.empty()) return false;

    jd->phase = components::JumpDriveState::JumpPhase::SpoolingUp;
    jd->phase_timer = 0.0f;
    jd->destination_system = destination;
    jd->jump_distance_ly = distance_ly;
    jd->cyno_target_id = cyno_id;
    return true;
}

bool JumpDriveSystem::cancelJump(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return false;

    if (jd->phase != components::JumpDriveState::JumpPhase::SpoolingUp) return false;

    jd->phase = components::JumpDriveState::JumpPhase::Idle;
    jd->phase_timer = 0.0f;
    return true;
}

bool JumpDriveSystem::refuel(const std::string& entity_id, float amount) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return false;

    jd->current_fuel = std::min(jd->current_fuel + amount, jd->max_fuel);
    return true;
}

bool JumpDriveSystem::setCynoTarget(const std::string& entity_id, const std::string& cyno_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return false;

    jd->cyno_target_id = cyno_id;
    return true;
}

std::string JumpDriveSystem::getPhase(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "unknown";

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return "unknown";

    return components::JumpDriveState::phaseToString(jd->phase);
}

float JumpDriveSystem::getFuel(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return 0.0f;

    return jd->current_fuel;
}

float JumpDriveSystem::getMaxFuel(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return 0.0f;

    return jd->max_fuel;
}

float JumpDriveSystem::getFatigue(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return 0.0f;

    return jd->fatigue_hours;
}

float JumpDriveSystem::getMaxRange(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return 0.0f;

    return jd->max_range_ly;
}

float JumpDriveSystem::getEffectiveRange(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return 0.0f;

    if (jd->max_fatigue <= 0.0f) return jd->max_range_ly;
    return jd->max_range_ly * (1.0f - jd->fatigue_hours / jd->max_fatigue);
}

bool JumpDriveSystem::canJump(const std::string& entity_id, float distance_ly) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return false;

    if (jd->phase != components::JumpDriveState::JumpPhase::Idle) return false;
    if (jd->fatigue_hours >= jd->max_fatigue) return false;

    float fuel_needed = jd->fuel_per_ly * distance_ly;
    if (jd->current_fuel < fuel_needed) return false;

    float effective_range = (jd->max_fatigue > 0.0f)
        ? jd->max_range_ly * (1.0f - jd->fatigue_hours / jd->max_fatigue)
        : jd->max_range_ly;
    if (distance_ly > effective_range) return false;

    return true;
}

int JumpDriveSystem::getTotalJumps(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return 0;

    return jd->total_jumps;
}

float JumpDriveSystem::getCooldownRemaining(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* jd = entity->getComponent<components::JumpDriveState>();
    if (!jd) return 0.0f;

    if (jd->phase == components::JumpDriveState::JumpPhase::Cooldown) {
        return jd->cooldown_time - jd->phase_timer;
    }
    return 0.0f;
}

} // namespace systems
} // namespace atlas
