#include "systems/fleet_coordination_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/fleet_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using FC = components::FleetCoordination;
using TA = components::FleetCoordination::TargetAssignment;

TA* findTarget(FC* fc, const std::string& target_id) {
    for (auto& t : fc->target_assignments) {
        if (t.target_id == target_id) return &t;
    }
    return nullptr;
}

const TA* findTargetConst(const FC* fc, const std::string& target_id) {
    for (const auto& t : fc->target_assignments) {
        if (t.target_id == target_id) return &t;
    }
    return nullptr;
}

} // anonymous namespace

FleetCoordinationSystem::FleetCoordinationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetCoordinationSystem::updateComponent(ecs::Entity& entity,
    components::FleetCoordination& fc, float delta_time) {
    if (!fc.active) return;

    fc.order_duration += delta_time;

    if (fc.in_combat) {
        fc.total_time_in_combat += delta_time;

        fc.formation_coherence -= fc.coherence_decay_rate * delta_time;
        if (fc.formation_coherence < 0.0f) fc.formation_coherence = 0.0f;
    }

    fc.combat_readiness = fc.formation_coherence * fc.morale_factor;
}

bool FleetCoordinationSystem::initialize(const std::string& entity_id,
    const std::string& fleet_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetCoordination>();
    comp->fleet_id = fleet_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetCoordinationSystem::issueOrder(const std::string& entity_id, int order) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;
    if (order < 0 || order > 5) return false;

    fc->current_order = static_cast<FC::CoordinationOrder>(order);
    fc->order_duration = 0.0f;
    fc->total_orders_issued++;
    return true;
}

bool FleetCoordinationSystem::assignTarget(const std::string& entity_id,
    const std::string& target_id, int priority) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;
    if (static_cast<int>(fc->target_assignments.size()) >= fc->max_targets) return false;
    if (findTarget(fc, target_id)) return false;

    TA ta;
    ta.target_id = target_id;
    ta.priority = std::max(1, std::min(5, priority));
    fc->target_assignments.push_back(ta);
    fc->total_targets_assigned++;
    return true;
}

bool FleetCoordinationSystem::removeTarget(const std::string& entity_id,
    const std::string& target_id) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;

    auto it = std::find_if(fc->target_assignments.begin(), fc->target_assignments.end(),
        [&](const TA& t) { return t.target_id == target_id; });
    if (it == fc->target_assignments.end()) return false;
    fc->target_assignments.erase(it);
    return true;
}

bool FleetCoordinationSystem::addShip(const std::string& entity_id,
    const std::string& ship_id) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;
    if (static_cast<int>(fc->participating_ships.size()) >= fc->max_ships) return false;

    for (const auto& s : fc->participating_ships) {
        if (s == ship_id) return false;
    }
    fc->participating_ships.push_back(ship_id);
    return true;
}

bool FleetCoordinationSystem::removeShip(const std::string& entity_id,
    const std::string& ship_id) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;

    auto it = std::find(fc->participating_ships.begin(),
                        fc->participating_ships.end(), ship_id);
    if (it == fc->participating_ships.end()) return false;
    fc->participating_ships.erase(it);
    return true;
}

bool FleetCoordinationSystem::enterCombat(const std::string& entity_id) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;
    fc->in_combat = true;
    return true;
}

bool FleetCoordinationSystem::leaveCombat(const std::string& entity_id) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;
    fc->in_combat = false;
    return true;
}

bool FleetCoordinationSystem::setFormationCoherence(const std::string& entity_id,
    float value) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;
    fc->formation_coherence = std::max(0.0f, std::min(1.0f, value));
    return true;
}

bool FleetCoordinationSystem::setMoraleFactor(const std::string& entity_id,
    float value) {
    auto* fc = getComponentFor(entity_id);
    if (!fc) return false;
    fc->morale_factor = std::max(0.0f, std::min(1.0f, value));
    return true;
}

int FleetCoordinationSystem::getOrder(const std::string& entity_id) const {
    auto* fc = getComponentFor(entity_id);
    return fc ? static_cast<int>(fc->current_order) : 0;
}

int FleetCoordinationSystem::getTargetCount(const std::string& entity_id) const {
    auto* fc = getComponentFor(entity_id);
    return fc ? static_cast<int>(fc->target_assignments.size()) : 0;
}

int FleetCoordinationSystem::getShipCount(const std::string& entity_id) const {
    auto* fc = getComponentFor(entity_id);
    return fc ? static_cast<int>(fc->participating_ships.size()) : 0;
}

float FleetCoordinationSystem::getCombatReadiness(const std::string& entity_id) const {
    auto* fc = getComponentFor(entity_id);
    return fc ? fc->combat_readiness : 0.0f;
}

std::string FleetCoordinationSystem::getHighestPriorityTarget(
    const std::string& entity_id) const {
    auto* fc = getComponentFor(entity_id);
    if (!fc || fc->target_assignments.empty()) return "";

    const TA* best = nullptr;
    for (const auto& t : fc->target_assignments) {
        if (!best || t.priority > best->priority) {
            best = &t;
        }
    }
    return best ? best->target_id : "";
}

bool FleetCoordinationSystem::isInCombat(const std::string& entity_id) const {
    auto* fc = getComponentFor(entity_id);
    return fc ? fc->in_combat : false;
}

int FleetCoordinationSystem::getTotalOrdersIssued(const std::string& entity_id) const {
    auto* fc = getComponentFor(entity_id);
    return fc ? fc->total_orders_issued : 0;
}

} // namespace systems
} // namespace atlas
