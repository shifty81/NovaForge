#include "systems/rig_link_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

RigLinkSystem::RigLinkSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void RigLinkSystem::updateComponent(ecs::Entity& /*entity*/,
                                     components::RigLinkState& comp,
                                     float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool RigLinkSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::RigLinkState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Linking
// ---------------------------------------------------------------------------

bool RigLinkSystem::link_to_ship(const std::string& entity_id,
                                  const std::string& ship_id,
                                  const std::string& port_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->is_linked) return false;
    comp->linked_ship_id = ship_id;
    comp->linked_port_id = port_id;
    comp->is_linked      = true;
    comp->total_links++;
    return true;
}

bool RigLinkSystem::unlink(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_linked) return false;
    comp->linked_ship_id = "";
    comp->linked_port_id = "";
    comp->is_linked      = false;
    comp->total_unlinks++;
    return true;
}

// ---------------------------------------------------------------------------
// Stat management
// ---------------------------------------------------------------------------

bool RigLinkSystem::add_stat(const std::string& entity_id,
                              const std::string& stat_name,
                              float base_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (stat_name.empty()) return false;
    if (static_cast<int>(comp->stats.size()) >= comp->max_stats) return false;

    for (const auto& s : comp->stats) {
        if (s.stat_name == stat_name) return false;
    }

    components::RigLinkState::RigStat stat;
    stat.stat_name  = stat_name;
    stat.base_value = base_value;
    comp->stats.push_back(stat);
    return true;
}

bool RigLinkSystem::remove_stat(const std::string& entity_id,
                                 const std::string& stat_name) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->stats.begin(), comp->stats.end(),
                           [&](const auto& s) { return s.stat_name == stat_name; });
    if (it == comp->stats.end()) return false;
    comp->stats.erase(it);
    return true;
}

bool RigLinkSystem::set_stat_bonus(const std::string& entity_id,
                                    const std::string& stat_name,
                                    float bonus) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& s : comp->stats) {
        if (s.stat_name == stat_name) {
            s.bonus = bonus;
            return true;
        }
    }
    return false;
}

bool RigLinkSystem::clear_stats(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->stats.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool RigLinkSystem::set_interface_level(const std::string& entity_id,
                                         int level) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (level < 1) return false;
    comp->interface_level = level;
    return true;
}

bool RigLinkSystem::set_link_quality(const std::string& entity_id,
                                      float quality) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->link_quality = quality;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool RigLinkSystem::is_linked(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return comp->is_linked;
}

std::string RigLinkSystem::get_linked_ship(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->linked_ship_id;
}

std::string RigLinkSystem::get_linked_port(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->linked_port_id;
}

float RigLinkSystem::get_stat_value(const std::string& entity_id,
                                     const std::string& stat_name) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;

    for (const auto& s : comp->stats) {
        if (s.stat_name == stat_name) return s.base_value + s.bonus;
    }
    return 0.0f;
}

int RigLinkSystem::get_stat_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->stats.size());
}

int RigLinkSystem::get_interface_level(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->interface_level;
}

float RigLinkSystem::get_link_quality(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    return comp->link_quality;
}

int RigLinkSystem::get_total_links(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_links;
}

} // namespace systems
} // namespace atlas
