#include "systems/cloaking_system.h"
#include "components/core_components.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CloakingSystem::CloakingSystem(ecs::World* world)
    : StateMachineSystem(world) {
}

void CloakingSystem::updateComponent(ecs::Entity& entity, components::CloakingState& cloak, float delta_time) {
    // Tick targeting lockout
    if (cloak.targeting_lockout_remaining > 0.0f) {
        cloak.targeting_lockout_remaining = std::max(0.0f, cloak.targeting_lockout_remaining - delta_time);
    }

    switch (cloak.phase) {
        case components::CloakingState::CloakPhase::Activating: {
            cloak.phase_timer += delta_time;
            if (cloak.phase_timer >= cloak.activation_time) {
                cloak.phase = components::CloakingState::CloakPhase::Cloaked;
                cloak.phase_timer = 0.0f;
            }
            break;
        }
        case components::CloakingState::CloakPhase::Cloaked: {
            auto* cap = entity.getComponent<components::Capacitor>();
            if (cap) {
                cap->capacitor -= cloak.fuel_per_second * delta_time;
                if (cap->capacitor <= 0.0f) {
                    cap->capacitor = 0.0f;
                    // Force decloak due to capacitor empty
                    cloak.phase = components::CloakingState::CloakPhase::Deactivating;
                    cloak.phase_timer = 0.0f;
                    cloak.decloak_count++;
                    cloak.targeting_lockout_remaining = cloak.targeting_delay;
                }
            }
            break;
        }
        case components::CloakingState::CloakPhase::Deactivating: {
            cloak.phase_timer += delta_time;
            if (cloak.phase_timer >= cloak.deactivation_time) {
                cloak.phase = components::CloakingState::CloakPhase::Inactive;
                cloak.phase_timer = 0.0f;
            }
            break;
        }
        default:
            break;
    }
}

bool CloakingSystem::activateCloak(const std::string& entity_id) {
    auto* cloak = getComponentFor(entity_id);
    if (!cloak) return false;

    if (cloak->phase != components::CloakingState::CloakPhase::Inactive) return false;

    cloak->phase = components::CloakingState::CloakPhase::Activating;
    cloak->phase_timer = 0.0f;
    return true;
}

bool CloakingSystem::deactivateCloak(const std::string& entity_id) {
    auto* cloak = getComponentFor(entity_id);
    if (!cloak) return false;

    if (cloak->phase != components::CloakingState::CloakPhase::Cloaked &&
        cloak->phase != components::CloakingState::CloakPhase::Activating) {
        return false;
    }

    cloak->phase = components::CloakingState::CloakPhase::Deactivating;
    cloak->phase_timer = 0.0f;
    cloak->decloak_count++;
    cloak->targeting_lockout_remaining = cloak->targeting_delay;
    return true;
}

bool CloakingSystem::setProximityRange(const std::string& entity_id, float range) {
    auto* cloak = getComponentFor(entity_id);
    if (!cloak) return false;

    cloak->proximity_decloak_range = range;
    return true;
}

bool CloakingSystem::proximityDecloak(const std::string& entity_id, float nearest_distance) {
    auto* cloak = getComponentFor(entity_id);
    if (!cloak) return false;

    if (cloak->phase != components::CloakingState::CloakPhase::Cloaked) return false;
    if (nearest_distance >= cloak->proximity_decloak_range) return false;

    cloak->phase = components::CloakingState::CloakPhase::Deactivating;
    cloak->phase_timer = 0.0f;
    cloak->decloak_count++;
    cloak->targeting_lockout_remaining = cloak->targeting_delay;
    return true;
}

std::string CloakingSystem::getPhase(const std::string& entity_id) const {
    const auto* cloak = getComponentFor(entity_id);
    if (!cloak) return "unknown";

    switch (cloak->phase) {
        case components::CloakingState::CloakPhase::Inactive: return "Inactive";
        case components::CloakingState::CloakPhase::Activating: return "Activating";
        case components::CloakingState::CloakPhase::Cloaked: return "Cloaked";
        case components::CloakingState::CloakPhase::Deactivating: return "Deactivating";
        default: return "unknown";
    }
}

bool CloakingSystem::isCloaked(const std::string& entity_id) const {
    const auto* cloak = getComponentFor(entity_id);
    if (!cloak) return false;

    return cloak->phase == components::CloakingState::CloakPhase::Cloaked;
}

bool CloakingSystem::isTargetingLocked(const std::string& entity_id) const {
    const auto* cloak = getComponentFor(entity_id);
    if (!cloak) return false;

    return cloak->targeting_lockout_remaining > 0.0f;
}

float CloakingSystem::getTargetingLockoutRemaining(const std::string& entity_id) const {
    const auto* cloak = getComponentFor(entity_id);
    if (!cloak) return 0.0f;

    return cloak->targeting_lockout_remaining;
}

float CloakingSystem::getFuelRate(const std::string& entity_id) const {
    const auto* cloak = getComponentFor(entity_id);
    if (!cloak) return 0.0f;

    return cloak->fuel_per_second;
}

bool CloakingSystem::canWarpCloaked(const std::string& entity_id) const {
    const auto* cloak = getComponentFor(entity_id);
    if (!cloak) return false;

    return cloak->can_warp_while_cloaked;
}

int CloakingSystem::getDecloakCount(const std::string& entity_id) const {
    const auto* cloak = getComponentFor(entity_id);
    if (!cloak) return 0;

    return cloak->decloak_count;
}

} // namespace systems
} // namespace atlas
