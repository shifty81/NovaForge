#include "systems/fleet_command_terminal_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/fps_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using FCT = components::FleetCommandTerminal;

const char* stateToString(FCT::TerminalState s) {
    switch (s) {
        case FCT::TerminalState::Offline:     return "Offline";
        case FCT::TerminalState::Idle:        return "Idle";
        case FCT::TerminalState::Booting:     return "Booting";
        case FCT::TerminalState::Active:      return "Active";
        case FCT::TerminalState::CommandMode: return "CommandMode";
        case FCT::TerminalState::Cooldown:    return "Cooldown";
        case FCT::TerminalState::Damaged:     return "Damaged";
    }
    return "Unknown";
}

const char* orderToString(FCT::FleetOrder o) {
    switch (o) {
        case FCT::FleetOrder::None:       return "None";
        case FCT::FleetOrder::Hold:       return "Hold";
        case FCT::FleetOrder::Engage:     return "Engage";
        case FCT::FleetOrder::FocusFire:  return "FocusFire";
        case FCT::FleetOrder::Retreat:    return "Retreat";
        case FCT::FleetOrder::Regroup:    return "Regroup";
        case FCT::FleetOrder::FormUp:     return "FormUp";
        case FCT::FleetOrder::Patrol:     return "Patrol";
        case FCT::FleetOrder::Escort:     return "Escort";
        case FCT::FleetOrder::Warp:       return "Warp";
    }
    return "None";
}

} // anonymous namespace

FleetCommandTerminalSystem::FleetCommandTerminalSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetCommandTerminalSystem::updateComponent(ecs::Entity& entity,
    FCT& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Check damage threshold
    if (comp.integrity < comp.damage_threshold &&
        comp.state != FCT::TerminalState::Offline &&
        comp.state != FCT::TerminalState::Damaged) {
        comp.state = FCT::TerminalState::Damaged;
        comp.active_user_id.clear();
        return;
    }

    // Boot sequence
    if (comp.state == FCT::TerminalState::Booting) {
        comp.boot_progress += delta_time / comp.boot_time;
        if (comp.boot_progress >= 1.0f) {
            comp.boot_progress = 1.0f;
            comp.state = FCT::TerminalState::Idle;
            comp.total_boots++;
        }
    }

    // Cooldown between orders
    if (comp.state == FCT::TerminalState::Cooldown) {
        comp.cooldown_remaining -= delta_time;
        if (comp.cooldown_remaining <= 0.0f) {
            comp.cooldown_remaining = 0.0f;
            comp.state = FCT::TerminalState::CommandMode;
        }
    }
}

bool FleetCommandTerminalSystem::initialize(const std::string& entity_id,
    const std::string& owner_id, const std::string& structure_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<FCT>();
    comp->owner_id = owner_id;
    comp->structure_id = structure_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetCommandTerminalSystem::placeTerminal(const std::string& entity_id) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    if (fct->placed) return false;
    fct->placed = true;
    return true;
}

bool FleetCommandTerminalSystem::powerOn(const std::string& entity_id) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    if (!fct->placed) return false;
    if (fct->state == FCT::TerminalState::Damaged) return false;
    if (fct->state != FCT::TerminalState::Offline) return false;
    fct->powered = true;
    fct->state = FCT::TerminalState::Booting;
    fct->boot_progress = 0.0f;
    return true;
}

bool FleetCommandTerminalSystem::powerOff(const std::string& entity_id) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    fct->powered = false;
    fct->state = FCT::TerminalState::Offline;
    fct->active_user_id.clear();
    fct->boot_progress = 0.0f;
    return true;
}

bool FleetCommandTerminalSystem::loginUser(const std::string& entity_id,
    const std::string& user_id) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    if (fct->state != FCT::TerminalState::Idle) return false;
    if (!fct->active_user_id.empty()) return false; // already in use
    fct->active_user_id = user_id;
    fct->state = FCT::TerminalState::Active;
    fct->total_sessions++;
    return true;
}

bool FleetCommandTerminalSystem::logoutUser(const std::string& entity_id) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    if (fct->active_user_id.empty()) return false;
    fct->active_user_id.clear();
    fct->current_order = FCT::FleetOrder::None;
    fct->current_target_id.clear();
    fct->linked_fleet_id.clear();
    if (fct->state != FCT::TerminalState::Offline &&
        fct->state != FCT::TerminalState::Damaged) {
        fct->state = FCT::TerminalState::Idle;
    }
    return true;
}

bool FleetCommandTerminalSystem::enterCommandMode(const std::string& entity_id,
    const std::string& fleet_id) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    if (fct->state != FCT::TerminalState::Active) return false;
    if (fct->active_user_id.empty()) return false;
    fct->linked_fleet_id = fleet_id;
    fct->state = FCT::TerminalState::CommandMode;
    return true;
}

bool FleetCommandTerminalSystem::exitCommandMode(const std::string& entity_id) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    if (fct->state != FCT::TerminalState::CommandMode &&
        fct->state != FCT::TerminalState::Cooldown) return false;
    fct->state = FCT::TerminalState::Active;
    fct->linked_fleet_id.clear();
    return true;
}

bool FleetCommandTerminalSystem::issueOrder(const std::string& entity_id,
    int order, const std::string& target_id) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    if (fct->state != FCT::TerminalState::CommandMode) return false;
    if (order < 0 || order > 9) return false;

    auto fleet_order = static_cast<FCT::FleetOrder>(order);

    fct->current_order = fleet_order;
    fct->current_target_id = target_id;

    // Record in order history
    FCT::IssuedOrder issued;
    issued.order = fleet_order;
    issued.target_id = target_id;
    issued.issued_at = fct->elapsed;
    issued.acknowledged = false;

    if (static_cast<int>(fct->order_history.size()) >= fct->max_orders_history) {
        fct->order_history.erase(fct->order_history.begin());
    }
    fct->order_history.push_back(issued);

    fct->total_orders_issued++;
    fct->cooldown_remaining = fct->cooldown_time;
    fct->state = FCT::TerminalState::Cooldown;
    return true;
}

bool FleetCommandTerminalSystem::damageTerminal(const std::string& entity_id,
    float amount) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    fct->integrity = std::max(0.0f, fct->integrity - amount);
    return true;
}

bool FleetCommandTerminalSystem::repairTerminal(const std::string& entity_id,
    float amount) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    fct->integrity = std::min(100.0f, fct->integrity + amount);
    // If repaired above threshold and was damaged, go to offline (needs reboot)
    if (fct->state == FCT::TerminalState::Damaged &&
        fct->integrity >= fct->damage_threshold) {
        fct->state = FCT::TerminalState::Offline;
    }
    return true;
}

bool FleetCommandTerminalSystem::updateFleetInfo(const std::string& entity_id,
    int ship_count, float readiness, float morale) {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    fct->fleet_ship_count = ship_count;
    fct->fleet_readiness = std::max(0.0f, std::min(1.0f, readiness));
    fct->fleet_morale = std::max(0.0f, std::min(100.0f, morale));
    return true;
}

std::string FleetCommandTerminalSystem::getState(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return "Unknown";
    return stateToString(fct->state);
}

std::string FleetCommandTerminalSystem::getActiveUser(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? fct->active_user_id : "";
}

std::string FleetCommandTerminalSystem::getCurrentOrder(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return "None";
    return orderToString(fct->current_order);
}

float FleetCommandTerminalSystem::getIntegrity(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? fct->integrity : 0.0f;
}

float FleetCommandTerminalSystem::getBootProgress(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? fct->boot_progress : 0.0f;
}

int FleetCommandTerminalSystem::getOrderHistoryCount(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? static_cast<int>(fct->order_history.size()) : 0;
}

int FleetCommandTerminalSystem::getTotalOrdersIssued(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? fct->total_orders_issued : 0;
}

int FleetCommandTerminalSystem::getTotalSessions(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? fct->total_sessions : 0;
}

int FleetCommandTerminalSystem::getFleetShipCount(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? fct->fleet_ship_count : 0;
}

float FleetCommandTerminalSystem::getFleetReadiness(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? fct->fleet_readiness : 0.0f;
}

float FleetCommandTerminalSystem::getFleetMorale(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    return fct ? fct->fleet_morale : 0.0f;
}

bool FleetCommandTerminalSystem::isOperational(const std::string& entity_id) const {
    auto* fct = getComponentFor(entity_id);
    if (!fct) return false;
    return fct->placed && fct->powered &&
           fct->integrity >= fct->damage_threshold &&
           fct->state != FCT::TerminalState::Offline &&
           fct->state != FCT::TerminalState::Damaged;
}

} // namespace systems
} // namespace atlas
