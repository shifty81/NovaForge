#include "systems/fleet_progression_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

FleetProgressionSystem::FleetProgressionSystem(ecs::World* world)
    : System(world) {
}

void FleetProgressionSystem::update(float /*delta_time*/) {
    auto entities = world_->getEntities<components::FleetProgression>();
    for (auto* entity : entities) {
        auto* prog = entity->getComponent<components::FleetProgression>();
        if (!prog) continue;

        auto prev_stage = prog->stage;
        prog->updateStage();

        // Auto-unlock wing roles on stage transition
        if (prog->stage >= components::FleetProgression::Stage::Mid) {
            prog->mining_wing_unlocked = true;
            prog->combat_wing_unlocked = true;
            prog->logistics_wing_unlocked = true;
        }
        if (prog->stage >= components::FleetProgression::Stage::End) {
            prog->salvage_wing_unlocked = true;
            prog->construction_wing_unlocked = true;
        }
    }
}

components::FleetProgression::Stage
FleetProgressionSystem::getStage(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return components::FleetProgression::Stage::Early;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) return components::FleetProgression::Stage::Early;

    return prog->stage;
}

void FleetProgressionSystem::addExperience(const std::string& entity_id, float xp) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) {
        entity->addComponent(std::make_unique<components::FleetProgression>());
        prog = entity->getComponent<components::FleetProgression>();
    }

    prog->fleet_experience += xp;
    prog->updateStage();
}

int FleetProgressionSystem::getMaxShips(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 5;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) return 5;

    return prog->max_ships;
}

int FleetProgressionSystem::getMaxWings(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 1;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) return 1;

    return prog->max_wings;
}

bool FleetProgressionSystem::canAddShip(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) return false;

    return prog->current_ship_count < prog->max_ships;
}

bool FleetProgressionSystem::addShipToFleet(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) return false;

    if (prog->current_ship_count >= prog->max_ships) return false;

    prog->current_ship_count++;
    return true;
}

void FleetProgressionSystem::removeShipFromFleet(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) return;

    if (prog->current_ship_count > 0) {
        prog->current_ship_count--;
    }
}

void FleetProgressionSystem::unlockWingRole(const std::string& entity_id, const std::string& role) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) return;

    if (role == "mining") prog->mining_wing_unlocked = true;
    else if (role == "combat") prog->combat_wing_unlocked = true;
    else if (role == "logistics") prog->logistics_wing_unlocked = true;
    else if (role == "salvage") prog->salvage_wing_unlocked = true;
    else if (role == "construction") prog->construction_wing_unlocked = true;
}

bool FleetProgressionSystem::isWingRoleUnlocked(const std::string& entity_id, const std::string& role) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* prog = entity->getComponent<components::FleetProgression>();
    if (!prog) return false;

    if (role == "mining") return prog->mining_wing_unlocked;
    if (role == "combat") return prog->combat_wing_unlocked;
    if (role == "logistics") return prog->logistics_wing_unlocked;
    if (role == "salvage") return prog->salvage_wing_unlocked;
    if (role == "construction") return prog->construction_wing_unlocked;

    return false;
}

} // namespace systems
} // namespace atlas
