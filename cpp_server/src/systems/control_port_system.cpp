#include "systems/control_port_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

ControlPortSystem::ControlPortSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void ControlPortSystem::updateComponent(ecs::Entity& /*entity*/,
                                         components::ControlPortState& comp,
                                         float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    for (auto& p : comp.ports) {
        if (p.occupied) {
            p.use_time += delta_time;
        }
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool ControlPortSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ControlPortState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Port management
// ---------------------------------------------------------------------------

bool ControlPortSystem::add_port(const std::string& entity_id,
                                  const std::string& port_id,
                                  const std::string& port_type,
                                  int enter_mode) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (port_id.empty()) return false;
    if (static_cast<int>(comp->ports.size()) >= comp->max_ports) return false;

    for (const auto& p : comp->ports) {
        if (p.port_id == port_id) return false;
    }

    components::ControlPortState::Port port;
    port.port_id    = port_id;
    port.port_type  = port_type;
    port.enter_mode = enter_mode;
    comp->ports.push_back(port);
    return true;
}

bool ControlPortSystem::remove_port(const std::string& entity_id,
                                     const std::string& port_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->ports.begin(), comp->ports.end(),
                           [&](const auto& p) { return p.port_id == port_id; });
    if (it == comp->ports.end()) return false;
    comp->ports.erase(it);
    return true;
}

bool ControlPortSystem::occupy_port(const std::string& entity_id,
                                     const std::string& port_id,
                                     const std::string& occupant_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& p : comp->ports) {
        if (p.port_id == port_id) {
            if (p.occupied) return false;
            p.occupied    = true;
            p.occupant_id = occupant_id;
            p.use_time    = 0.0f;
            comp->total_uses++;
            return true;
        }
    }
    return false;
}

bool ControlPortSystem::vacate_port(const std::string& entity_id,
                                     const std::string& port_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& p : comp->ports) {
        if (p.port_id == port_id) {
            if (!p.occupied) return false;
            p.occupied    = false;
            p.occupant_id = "";
            return true;
        }
    }
    return false;
}

bool ControlPortSystem::clear_ports(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->ports.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool ControlPortSystem::is_occupied(const std::string& entity_id,
                                     const std::string& port_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& p : comp->ports) {
        if (p.port_id == port_id) return p.occupied;
    }
    return false;
}

std::string ControlPortSystem::get_occupant(const std::string& entity_id,
                                             const std::string& port_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& p : comp->ports) {
        if (p.port_id == port_id) return p.occupant_id;
    }
    return "";
}

int ControlPortSystem::get_port_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->ports.size());
}

std::string ControlPortSystem::get_port_type(const std::string& entity_id,
                                              const std::string& port_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& p : comp->ports) {
        if (p.port_id == port_id) return p.port_type;
    }
    return "";
}

int ControlPortSystem::get_total_uses(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_uses;
}

} // namespace systems
} // namespace atlas
