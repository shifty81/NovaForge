#include "systems/fps_enemy_ai_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

FPSEnemyAISystem::FPSEnemyAISystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FPSEnemyAISystem::updateComponent(ecs::Entity& /*entity*/,
    components::FPSEnemyAI& ai, float delta_time) {
    if (!ai.active) return;
    if (ai.state == components::FPSEnemyAI::Dead) return;

    // Attack cooldown
    if (ai.attack_timer > 0.0f) {
        ai.attack_timer -= delta_time;
        if (ai.attack_timer < 0.0f) ai.attack_timer = 0.0f;
    }

    // Alert timer countdown
    if (ai.state == components::FPSEnemyAI::Alert) {
        ai.alert_timer -= delta_time;
        if (ai.alert_timer <= 0.0f) {
            ai.alert_timer = 0.0f;
            ai.state = components::FPSEnemyAI::Patrol;
            ai.target_id.clear();
        }
    }

    // Patrol waypoint progression
    if (ai.state == components::FPSEnemyAI::Patrol && !ai.patrol_route.empty()) {
        ai.waypoint_wait_timer -= delta_time;
        if (ai.waypoint_wait_timer <= 0.0f) {
            ai.current_waypoint = (ai.current_waypoint + 1) %
                static_cast<int>(ai.patrol_route.size());
            if (ai.current_waypoint == 0) {
                ai.total_patrols_completed++;
            }
            auto& wp = ai.patrol_route[ai.current_waypoint];
            ai.pos_x = wp.x;
            ai.pos_y = wp.y;
            ai.pos_z = wp.z;
            ai.waypoint_wait_timer = wp.wait_time;
        }
    }

    // Attack tick
    if (ai.state == components::FPSEnemyAI::Attack && ai.attack_timer <= 0.0f) {
        ai.total_attacks++;
        ai.attack_timer = ai.attack_cooldown;
    }
}

bool FPSEnemyAISystem::setState(const std::string& entity_id, int state) {
    if (state < 0 || state > 6) return false;
    auto* ai = getComponentFor(entity_id);
    if (!ai) return false;
    if (ai->state == components::FPSEnemyAI::Dead) return false;
    ai->state = static_cast<components::FPSEnemyAI::AIState>(state);
    if (state == components::FPSEnemyAI::Alert) {
        ai->alert_timer = ai->alert_duration;
    }
    return true;
}

int FPSEnemyAISystem::getState(const std::string& entity_id) const {
    auto* ai = getComponentFor(entity_id);
    return ai ? static_cast<int>(ai->state) : 0;
}

bool FPSEnemyAISystem::setTarget(const std::string& entity_id, const std::string& target_id) {
    auto* ai = getComponentFor(entity_id);
    if (!ai) return false;
    if (ai->state == components::FPSEnemyAI::Dead) return false;
    ai->target_id = target_id;
    return true;
}

std::string FPSEnemyAISystem::getTarget(const std::string& entity_id) const {
    auto* ai = getComponentFor(entity_id);
    return ai ? ai->target_id : "";
}

bool FPSEnemyAISystem::addWaypoint(const std::string& entity_id,
    const std::string& wp_id, float x, float y, float z, float wait_time) {
    auto* ai = getComponentFor(entity_id);
    if (!ai) return false;
    for (const auto& wp : ai->patrol_route) {
        if (wp.waypoint_id == wp_id) return false;
    }
    components::FPSEnemyAI::Waypoint wp;
    wp.waypoint_id = wp_id;
    wp.x = x;
    wp.y = y;
    wp.z = z;
    wp.wait_time = wait_time;
    ai->patrol_route.push_back(wp);
    return true;
}

int FPSEnemyAISystem::getWaypointCount(const std::string& entity_id) const {
    auto* ai = getComponentFor(entity_id);
    return ai ? static_cast<int>(ai->patrol_route.size()) : 0;
}

bool FPSEnemyAISystem::applyDamage(const std::string& entity_id, float amount) {
    if (amount <= 0.0f) return false;
    auto* ai = getComponentFor(entity_id);
    if (!ai) return false;
    if (ai->state == components::FPSEnemyAI::Dead) return false;
    ai->health -= amount;
    if (ai->health <= 0.0f) {
        ai->health = 0.0f;
        ai->state = components::FPSEnemyAI::Dead;
    }
    return true;
}

float FPSEnemyAISystem::getHealth(const std::string& entity_id) const {
    auto* ai = getComponentFor(entity_id);
    return ai ? ai->health : 0.0f;
}

bool FPSEnemyAISystem::isDead(const std::string& entity_id) const {
    auto* ai = getComponentFor(entity_id);
    return ai ? ai->state == components::FPSEnemyAI::Dead : false;
}

bool FPSEnemyAISystem::isHostile(const std::string& entity_id) const {
    auto* ai = getComponentFor(entity_id);
    if (!ai) return false;
    return ai->state != components::FPSEnemyAI::Dead &&
           ai->state != components::FPSEnemyAI::Idle;
}

float FPSEnemyAISystem::getDistanceToTarget(const std::string& entity_id,
    float target_x, float target_y, float target_z) const {
    auto* ai = getComponentFor(entity_id);
    if (!ai) return 0.0f;
    float dx = ai->pos_x - target_x;
    float dy = ai->pos_y - target_y;
    float dz = ai->pos_z - target_z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool FPSEnemyAISystem::canDetect(const std::string& entity_id,
    float target_x, float target_y, float target_z) const {
    auto* ai = getComponentFor(entity_id);
    if (!ai) return false;
    if (ai->state == components::FPSEnemyAI::Dead) return false;
    float dist = getDistanceToTarget(entity_id, target_x, target_y, target_z);
    return dist <= ai->detection_range;
}

int FPSEnemyAISystem::getTotalAttacks(const std::string& entity_id) const {
    auto* ai = getComponentFor(entity_id);
    return ai ? ai->total_attacks : 0;
}

int FPSEnemyAISystem::getTotalPatrolsCompleted(const std::string& entity_id) const {
    auto* ai = getComponentFor(entity_id);
    return ai ? ai->total_patrols_completed : 0;
}

bool FPSEnemyAISystem::setPosition(const std::string& entity_id, float x, float y, float z) {
    auto* ai = getComponentFor(entity_id);
    if (!ai) return false;
    ai->pos_x = x;
    ai->pos_y = y;
    ai->pos_z = z;
    return true;
}

} // namespace systems
} // namespace atlas
