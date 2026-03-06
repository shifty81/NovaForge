#include "systems/fps_companion_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FPSCompanionSystem::FPSCompanionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FPSCompanionSystem::updateComponent(ecs::Entity& /*entity*/,
    components::FPSCompanion& companion, float delta_time) {
    if (!companion.active) return;
    if (companion.state == components::FPSCompanion::Dead ||
        companion.state == components::FPSCompanion::Rescued) return;

    // Morale recovery when not under fire (following state)
    if (companion.state == components::FPSCompanion::Following) {
        companion.morale = std::min(100.0f,
            companion.morale + companion.morale_recovery_rate * delta_time);
    }

    // Natural health regen
    if (companion.heal_rate > 0.0f && companion.health < companion.max_health) {
        companion.health = std::min(companion.max_health,
            companion.health + companion.heal_rate * delta_time);
    }

    // Auto-hide when health is critical
    if (companion.state == components::FPSCompanion::Following &&
        companion.health <= companion.max_health * companion.panic_threshold / 100.0f) {
        companion.state = components::FPSCompanion::Hiding;
    }
}

bool FPSCompanionSystem::startFollowing(const std::string& entity_id,
    const std::string& player_id) {
    auto* companion = getComponentFor(entity_id);
    if (!companion) return false;
    if (companion->state == components::FPSCompanion::Dead ||
        companion->state == components::FPSCompanion::Rescued) return false;
    companion->state = components::FPSCompanion::Following;
    companion->follower_of = player_id;
    return true;
}

bool FPSCompanionSystem::stopFollowing(const std::string& entity_id) {
    auto* companion = getComponentFor(entity_id);
    if (!companion) return false;
    if (companion->state != components::FPSCompanion::Following) return false;
    companion->state = components::FPSCompanion::Waiting;
    return true;
}

bool FPSCompanionSystem::commandHide(const std::string& entity_id) {
    auto* companion = getComponentFor(entity_id);
    if (!companion) return false;
    if (companion->state == components::FPSCompanion::Dead ||
        companion->state == components::FPSCompanion::Rescued) return false;
    companion->state = components::FPSCompanion::Hiding;
    return true;
}

bool FPSCompanionSystem::applyDamage(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* companion = getComponentFor(entity_id);
    if (!companion) return false;
    if (companion->state == components::FPSCompanion::Dead) return false;
    companion->health -= amount;
    companion->morale = std::max(0.0f,
        companion->morale - companion->morale_decay_rate);
    companion->times_injured++;
    if (companion->health <= 0.0f) {
        companion->health = 0.0f;
        companion->state = components::FPSCompanion::Dead;
    } else if (companion->health <= companion->max_health * companion->panic_threshold / 100.0f) {
        companion->state = components::FPSCompanion::Injured;
    }
    return true;
}

bool FPSCompanionSystem::heal(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* companion = getComponentFor(entity_id);
    if (!companion) return false;
    if (companion->state == components::FPSCompanion::Dead) return false;
    companion->health = std::min(companion->max_health, companion->health + amount);
    companion->times_healed++;
    // If healed above panic threshold, can follow again
    if (companion->state == components::FPSCompanion::Injured &&
        companion->health > companion->max_health * companion->panic_threshold / 100.0f) {
        companion->state = components::FPSCompanion::Following;
    }
    return true;
}

bool FPSCompanionSystem::rescue(const std::string& entity_id) {
    auto* companion = getComponentFor(entity_id);
    if (!companion) return false;
    if (companion->state == components::FPSCompanion::Dead) return false;
    companion->state = components::FPSCompanion::Rescued;
    return true;
}

int FPSCompanionSystem::getState(const std::string& entity_id) const {
    auto* companion = getComponentFor(entity_id);
    return companion ? static_cast<int>(companion->state) : 0;
}

float FPSCompanionSystem::getHealth(const std::string& entity_id) const {
    auto* companion = getComponentFor(entity_id);
    return companion ? companion->health : 0.0f;
}

float FPSCompanionSystem::getMorale(const std::string& entity_id) const {
    auto* companion = getComponentFor(entity_id);
    return companion ? companion->morale : 0.0f;
}

bool FPSCompanionSystem::isDead(const std::string& entity_id) const {
    auto* companion = getComponentFor(entity_id);
    return companion ? companion->state == components::FPSCompanion::Dead : false;
}

bool FPSCompanionSystem::isRescued(const std::string& entity_id) const {
    auto* companion = getComponentFor(entity_id);
    return companion ? companion->state == components::FPSCompanion::Rescued : false;
}

float FPSCompanionSystem::getDistanceFollowed(const std::string& entity_id) const {
    auto* companion = getComponentFor(entity_id);
    return companion ? companion->total_distance_followed : 0.0f;
}

bool FPSCompanionSystem::setPosition(const std::string& entity_id, float x, float y, float z) {
    auto* companion = getComponentFor(entity_id);
    if (!companion) return false;
    companion->pos_x = x;
    companion->pos_y = y;
    companion->pos_z = z;
    return true;
}

} // namespace systems
} // namespace atlas
