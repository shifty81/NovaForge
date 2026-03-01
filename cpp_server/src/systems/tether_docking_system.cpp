#include "systems/tether_docking_system.h"
#include "ecs/world.h"

#include <algorithm>

namespace atlas {
namespace systems {

TetherDockingSystem::TetherDockingSystem(ecs::World* world)
    : System(world) {
}

void TetherDockingSystem::update(float delta_time) {
    for (auto* entity : world_->getEntities<components::TetherDockingArm>()) {
        auto* arm = entity->getComponent<components::TetherDockingArm>();
        if (!arm) continue;

        switch (arm->state) {
            case components::TetherDockingArm::ArmState::Extending:
                arm->extend_progress += arm->extend_speed * delta_time;
                if (arm->extend_progress >= 1.0f) {
                    arm->extend_progress = 1.0f;
                    arm->state = components::TetherDockingArm::ArmState::Locked;
                    arm->crew_transfer_enabled = true;
                }
                break;

            case components::TetherDockingArm::ArmState::Retracting:
                arm->extend_progress -= arm->extend_speed * delta_time;
                if (arm->extend_progress <= 0.0f) {
                    arm->extend_progress = 0.0f;
                    arm->state = components::TetherDockingArm::ArmState::Retracted;
                    arm->tethered_ship_id.clear();
                    arm->crew_transfer_enabled = false;
                    arm->station_shield_active = true;
                }
                break;

            default:
                break;
        }
    }
}

// ---------------------------------------------------------------------------
// Arm creation
// ---------------------------------------------------------------------------

bool TetherDockingSystem::createArm(const std::string& arm_id,
                                     const std::string& station_id,
                                     float min_ship_mass) {
    if (world_->getEntity(arm_id)) return false;

    auto* entity = world_->createEntity(arm_id);
    if (!entity) return false;

    auto arm = std::make_unique<components::TetherDockingArm>();
    arm->arm_id       = arm_id;
    arm->station_id   = station_id;
    arm->min_ship_mass = min_ship_mass;
    entity->addComponent(std::move(arm));
    return true;
}

// ---------------------------------------------------------------------------
// Tether lifecycle
// ---------------------------------------------------------------------------

bool TetherDockingSystem::beginTether(const std::string& arm_id,
                                       const std::string& ship_id) {
    auto* entity = world_->getEntity(arm_id);
    if (!entity) return false;

    auto* arm = entity->getComponent<components::TetherDockingArm>();
    if (!arm) return false;
    if (arm->isOccupied()) return false;
    if (arm->state != components::TetherDockingArm::ArmState::Retracted) return false;

    arm->tethered_ship_id = ship_id;
    arm->state = components::TetherDockingArm::ArmState::Extending;
    arm->extend_progress = 0.0f;
    arm->crew_transfer_enabled = false;
    return true;
}

bool TetherDockingSystem::beginUndock(const std::string& arm_id) {
    auto* entity = world_->getEntity(arm_id);
    if (!entity) return false;

    auto* arm = entity->getComponent<components::TetherDockingArm>();
    if (!arm) return false;
    if (!arm->isOccupied()) return false;
    if (arm->state != components::TetherDockingArm::ArmState::Locked) return false;

    arm->state = components::TetherDockingArm::ArmState::Retracting;
    arm->crew_transfer_enabled = false;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool TetherDockingSystem::isCrewTransferEnabled(const std::string& arm_id) const {
    auto* entity = world_->getEntity(arm_id);
    if (!entity) return false;

    auto* arm = entity->getComponent<components::TetherDockingArm>();
    if (!arm) return false;

    return arm->crew_transfer_enabled;
}

components::TetherDockingArm::ArmState
TetherDockingSystem::getArmState(const std::string& arm_id) const {
    auto* entity = world_->getEntity(arm_id);
    if (!entity) return components::TetherDockingArm::ArmState::Retracted;

    auto* arm = entity->getComponent<components::TetherDockingArm>();
    if (!arm) return components::TetherDockingArm::ArmState::Retracted;

    return arm->state;
}

std::string TetherDockingSystem::getTetheredShip(const std::string& arm_id) const {
    auto* entity = world_->getEntity(arm_id);
    if (!entity) return "";

    auto* arm = entity->getComponent<components::TetherDockingArm>();
    if (!arm) return "";

    return arm->tethered_ship_id;
}

bool TetherDockingSystem::isOccupied(const std::string& arm_id) const {
    auto* entity = world_->getEntity(arm_id);
    if (!entity) return false;

    auto* arm = entity->getComponent<components::TetherDockingArm>();
    if (!arm) return false;

    return arm->isOccupied();
}

float TetherDockingSystem::getExtendProgress(const std::string& arm_id) const {
    auto* entity = world_->getEntity(arm_id);
    if (!entity) return 0.0f;

    auto* arm = entity->getComponent<components::TetherDockingArm>();
    if (!arm) return 0.0f;

    return arm->extend_progress;
}

} // namespace systems
} // namespace atlas
