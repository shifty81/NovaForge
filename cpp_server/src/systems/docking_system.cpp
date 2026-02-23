#include "systems/docking_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

DockingSystem::DockingSystem(ecs::World* world)
    : System(world) {
}

void DockingSystem::update(float /*delta_time*/) {
    // Docking system is event-driven via dock/undock
}

bool DockingSystem::dock(const std::string& port_entity_id, const std::string& ship_entity_id) {
    auto* entity = world_->getEntity(port_entity_id);
    if (!entity) return false;
    auto* port = entity->getComponent<components::DockingPort>();
    if (!port) return false;
    if (port->isOccupied()) return false;

    // Extend docking ring if needed
    if (port->type == components::DockingPort::PortType::DockingRing) {
        port->is_extended = true;
    }

    port->docked_entity_id = ship_entity_id;
    return true;
}

std::string DockingSystem::undock(const std::string& port_entity_id) {
    auto* entity = world_->getEntity(port_entity_id);
    if (!entity) return "";
    auto* port = entity->getComponent<components::DockingPort>();
    if (!port) return "";
    if (!port->isOccupied()) return "";

    std::string undocked = port->docked_entity_id;
    port->docked_entity_id.clear();

    if (port->type == components::DockingPort::PortType::DockingRing) {
        port->is_extended = false;
    }

    return undocked;
}

bool DockingSystem::extendDockingRing(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* port = entity->getComponent<components::DockingPort>();
    if (!port) return false;
    if (port->type != components::DockingPort::PortType::DockingRing) return false;
    port->is_extended = true;
    return true;
}

bool DockingSystem::retractDockingRing(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* port = entity->getComponent<components::DockingPort>();
    if (!port) return false;
    if (port->type != components::DockingPort::PortType::DockingRing) return false;
    if (port->isOccupied()) return false;
    port->is_extended = false;
    return true;
}

bool DockingSystem::isOccupied(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* port = entity->getComponent<components::DockingPort>();
    if (!port) return false;
    return port->isOccupied();
}

std::string DockingSystem::getDockedEntity(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";
    auto* port = entity->getComponent<components::DockingPort>();
    if (!port) return "";
    return port->docked_entity_id;
}

} // namespace systems
} // namespace atlas
