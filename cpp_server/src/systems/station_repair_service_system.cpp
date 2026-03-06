#include "systems/station_repair_service_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

StationRepairServiceSystem::StationRepairServiceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

bool StationRepairServiceSystem::initializeRepair(const std::string& entity_id,
                                                  const std::string& station_id,
                                                  float repair_rate,
                                                  float cost_per_hp) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->hasComponent<components::StationRepairService>()) return false;

    auto repair = std::make_unique<components::StationRepairService>();
    repair->station_id = station_id;
    repair->repair_rate = repair_rate;
    repair->cost_per_hp = cost_per_hp;
    entity->addComponent(std::move(repair));
    return true;
}

bool StationRepairServiceSystem::startRepair(const std::string& entity_id,
                                             float shield_missing,
                                             float armor_missing,
                                             float hull_missing) {
    auto* repair = getComponentFor(entity_id);
    if (!repair || !repair->active) return false;
    if (repair->phase != components::StationRepairService::RepairPhase::Idle &&
        repair->phase != components::StationRepairService::RepairPhase::Complete) {
        return false;  // already repairing
    }

    repair->shield_to_repair = shield_missing;
    repair->armor_to_repair = armor_missing;
    repair->hull_to_repair = hull_missing;
    repair->total_cost = 0.0f;
    repair->total_hp_repaired = 0.0f;
    repair->elapsed = 0.0f;

    // Start with shield phase (or skip if no shield damage)
    if (shield_missing > 0.0f) {
        repair->phase = components::StationRepairService::RepairPhase::RepairingShield;
    } else if (armor_missing > 0.0f) {
        repair->phase = components::StationRepairService::RepairPhase::RepairingArmor;
    } else if (hull_missing > 0.0f) {
        repair->phase = components::StationRepairService::RepairPhase::RepairingHull;
    } else {
        repair->phase = components::StationRepairService::RepairPhase::Complete;
    }
    return true;
}

bool StationRepairServiceSystem::isRepairing(const std::string& entity_id) const {
    auto* repair = getComponentFor(entity_id);
    if (!repair) return false;
    return repair->phase == components::StationRepairService::RepairPhase::RepairingShield ||
           repair->phase == components::StationRepairService::RepairPhase::RepairingArmor ||
           repair->phase == components::StationRepairService::RepairPhase::RepairingHull;
}

bool StationRepairServiceSystem::isRepairComplete(const std::string& entity_id) const {
    auto* repair = getComponentFor(entity_id);
    if (!repair) return false;
    return repair->phase == components::StationRepairService::RepairPhase::Complete;
}

int StationRepairServiceSystem::getRepairPhase(const std::string& entity_id) const {
    auto* repair = getComponentFor(entity_id);
    if (!repair) return -1;
    return static_cast<int>(repair->phase);
}

float StationRepairServiceSystem::getTotalCost(const std::string& entity_id) const {
    auto* repair = getComponentFor(entity_id);
    return repair ? repair->total_cost : 0.0f;
}

float StationRepairServiceSystem::getTotalHPRepaired(const std::string& entity_id) const {
    auto* repair = getComponentFor(entity_id);
    return repair ? repair->total_hp_repaired : 0.0f;
}

bool StationRepairServiceSystem::cancelRepair(const std::string& entity_id) {
    auto* repair = getComponentFor(entity_id);
    if (!repair) return false;
    if (repair->phase == components::StationRepairService::RepairPhase::Idle) return false;
    repair->phase = components::StationRepairService::RepairPhase::Idle;
    repair->shield_to_repair = 0.0f;
    repair->armor_to_repair = 0.0f;
    repair->hull_to_repair = 0.0f;
    return true;
}

void StationRepairServiceSystem::updateComponent(ecs::Entity& /*entity*/,
                                                 components::StationRepairService& repair,
                                                 float delta_time) {
    if (!repair.active) return;
    repair.elapsed += delta_time;

    using Phase = components::StationRepairService::RepairPhase;

    float hp_this_tick = repair.repair_rate * delta_time;

    switch (repair.phase) {
        case Phase::RepairingShield: {
            float to_repair = std::min(hp_this_tick, repair.shield_to_repair);
            repair.shield_to_repair -= to_repair;
            repair.total_hp_repaired += to_repair;
            repair.total_cost += to_repair * repair.cost_per_hp;
            if (repair.shield_to_repair <= 0.0f) {
                repair.shield_to_repair = 0.0f;
                if (repair.armor_to_repair > 0.0f)
                    repair.phase = Phase::RepairingArmor;
                else if (repair.hull_to_repair > 0.0f)
                    repair.phase = Phase::RepairingHull;
                else
                    repair.phase = Phase::Complete;
            }
            break;
        }
        case Phase::RepairingArmor: {
            float to_repair = std::min(hp_this_tick, repair.armor_to_repair);
            repair.armor_to_repair -= to_repair;
            repair.total_hp_repaired += to_repair;
            repair.total_cost += to_repair * repair.cost_per_hp;
            if (repair.armor_to_repair <= 0.0f) {
                repair.armor_to_repair = 0.0f;
                if (repair.hull_to_repair > 0.0f)
                    repair.phase = Phase::RepairingHull;
                else
                    repair.phase = Phase::Complete;
            }
            break;
        }
        case Phase::RepairingHull: {
            float to_repair = std::min(hp_this_tick, repair.hull_to_repair);
            repair.hull_to_repair -= to_repair;
            repair.total_hp_repaired += to_repair;
            repair.total_cost += to_repair * repair.cost_per_hp;
            if (repair.hull_to_repair <= 0.0f) {
                repair.hull_to_repair = 0.0f;
                repair.phase = Phase::Complete;
            }
            break;
        }
        default:
            break;
    }
}

} // namespace systems
} // namespace atlas
