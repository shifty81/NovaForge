#include "systems/ship_maintenance_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using SM = components::ShipMaintenance;

const char* conditionToString(SM::Condition c) {
    switch (c) {
        case SM::Condition::Pristine: return "Pristine";
        case SM::Condition::Good:     return "Good";
        case SM::Condition::Fair:     return "Fair";
        case SM::Condition::Poor:     return "Poor";
        case SM::Condition::Critical: return "Critical";
    }
    return "Unknown";
}

SM::Condition integrityToCondition(float integrity) {
    if (integrity >= 0.9f) return SM::Condition::Pristine;
    if (integrity >= 0.7f) return SM::Condition::Good;
    if (integrity >= 0.5f) return SM::Condition::Fair;
    if (integrity >= 0.25f) return SM::Condition::Poor;
    return SM::Condition::Critical;
}

float conditionPenalty(SM::Condition c) {
    switch (c) {
        case SM::Condition::Pristine: return 0.0f;
        case SM::Condition::Good:     return 0.05f;
        case SM::Condition::Fair:     return 0.15f;
        case SM::Condition::Poor:     return 0.30f;
        case SM::Condition::Critical: return 0.50f;
    }
    return 0.0f;
}

void recalcCondition(SM& comp) {
    comp.condition = integrityToCondition(comp.hull_integrity);
    comp.performance_penalty = conditionPenalty(comp.condition);
}

} // anonymous namespace

ShipMaintenanceSystem::ShipMaintenanceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ShipMaintenanceSystem::updateComponent(ecs::Entity& entity,
    components::ShipMaintenance& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Apply wear
    float wear = comp.wear_rate * delta_time;
    if (comp.in_combat) {
        wear += comp.combat_wear_rate * delta_time;
    }
    comp.hull_integrity = std::max(0.0f, comp.hull_integrity - wear);
    recalcCondition(comp);

    // Process repair queue while docked
    if (comp.docked && !comp.repair_queue.empty()) {
        auto& front = comp.repair_queue.front();
        if (!front.completed) {
            front.time_elapsed += delta_time;
            if (front.time_elapsed >= front.time_required) {
                front.completed = true;
                comp.total_repair_cost += front.cost;
                comp.total_repairs_completed++;
                // Repair restores integrity proportional to cost
                comp.hull_integrity = std::min(1.0f,
                    comp.hull_integrity + 0.1f);
                recalcCondition(comp);
            }
        }
        // Remove completed repairs
        comp.repair_queue.erase(
            std::remove_if(comp.repair_queue.begin(), comp.repair_queue.end(),
                [](const SM::RepairOrder& r) { return r.completed; }),
            comp.repair_queue.end());
    }
}

bool ShipMaintenanceSystem::initialize(const std::string& entity_id,
    const std::string& ship_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ShipMaintenance>();
    comp->ship_id = ship_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool ShipMaintenanceSystem::setWearRate(const std::string& entity_id,
    float wear_rate, float combat_wear_rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->wear_rate = wear_rate;
    comp->combat_wear_rate = combat_wear_rate;
    return true;
}

bool ShipMaintenanceSystem::setCombatState(const std::string& entity_id,
    bool in_combat) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->in_combat = in_combat;
    return true;
}

bool ShipMaintenanceSystem::setDockedState(const std::string& entity_id,
    bool docked) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->docked = docked;
    return true;
}

bool ShipMaintenanceSystem::queueRepair(const std::string& entity_id,
    const std::string& module_name, double cost, float time_required) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->docked) return false;
    SM::RepairOrder order;
    order.module_name = module_name;
    order.cost = cost;
    order.time_required = time_required;
    comp->repair_queue.push_back(order);
    return true;
}

bool ShipMaintenanceSystem::applyDamage(const std::string& entity_id,
    float damage_amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->hull_integrity = std::max(0.0f, comp->hull_integrity - damage_amount);
    recalcCondition(*comp);
    return true;
}

std::string ShipMaintenanceSystem::getCondition(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    return conditionToString(comp->condition);
}

float ShipMaintenanceSystem::getHullIntegrity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->hull_integrity : 0.0f;
}

float ShipMaintenanceSystem::getPerformancePenalty(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->performance_penalty : 0.0f;
}

int ShipMaintenanceSystem::getRepairQueueSize(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->repair_queue.size()) : 0;
}

int ShipMaintenanceSystem::getTotalRepairsCompleted(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_repairs_completed : 0;
}

double ShipMaintenanceSystem::getTotalRepairCost(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_repair_cost : 0.0;
}

} // namespace systems
} // namespace atlas
