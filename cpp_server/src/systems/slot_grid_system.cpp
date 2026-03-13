#include "systems/slot_grid_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

SlotGridSystem::SlotGridSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void SlotGridSystem::updateComponent(ecs::Entity& /*entity*/,
                                      components::SlotGridState& comp,
                                      float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool SlotGridSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SlotGridState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Slot management
// ---------------------------------------------------------------------------

bool SlotGridSystem::add_slot(const std::string& entity_id,
                               const std::string& slot_id,
                               int x, int y, int z, int size) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (slot_id.empty()) return false;
    if (static_cast<int>(comp->slots.size()) >= comp->max_slots) return false;

    for (const auto& s : comp->slots) {
        if (s.slot_id == slot_id) return false;
    }

    components::SlotGridState::Slot slot;
    slot.slot_id = slot_id;
    slot.x       = x;
    slot.y       = y;
    slot.z       = z;
    slot.size    = size;
    comp->slots.push_back(slot);
    return true;
}

bool SlotGridSystem::remove_slot(const std::string& entity_id,
                                  const std::string& slot_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->slots.begin(), comp->slots.end(),
                           [&](const auto& s) { return s.slot_id == slot_id; });
    if (it == comp->slots.end()) return false;
    comp->slots.erase(it);
    return true;
}

bool SlotGridSystem::place_module(const std::string& entity_id,
                                   const std::string& slot_id,
                                   const std::string& module_id,
                                   const std::string& module_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& s : comp->slots) {
        if (s.slot_id == slot_id) {
            if (s.occupied) return false;
            s.occupied    = true;
            s.module_id   = module_id;
            s.module_type = module_type;
            comp->total_modules_placed++;
            return true;
        }
    }
    return false;
}

bool SlotGridSystem::remove_module(const std::string& entity_id,
                                    const std::string& slot_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& s : comp->slots) {
        if (s.slot_id == slot_id) {
            if (!s.occupied) return false;
            s.occupied    = false;
            s.module_id   = "";
            s.module_type = "";
            return true;
        }
    }
    return false;
}

bool SlotGridSystem::clear_grid(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->slots.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Ship metadata
// ---------------------------------------------------------------------------

bool SlotGridSystem::set_ship_class(const std::string& entity_id,
                                     const std::string& ship_class) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->ship_class = ship_class;
    return true;
}

bool SlotGridSystem::set_tier(const std::string& entity_id, int tier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (tier < 1) return false;
    comp->tier = tier;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool SlotGridSystem::is_slot_occupied(const std::string& entity_id,
                                       const std::string& slot_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->slots) {
        if (s.slot_id == slot_id) return s.occupied;
    }
    return false;
}

std::string SlotGridSystem::get_module_at(const std::string& entity_id,
                                           const std::string& slot_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    for (const auto& s : comp->slots) {
        if (s.slot_id == slot_id) return s.module_id;
    }
    return "";
}

int SlotGridSystem::get_slot_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return static_cast<int>(comp->slots.size());
}

int SlotGridSystem::get_occupied_count(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->slots) {
        if (s.occupied) count++;
    }
    return count;
}

std::string SlotGridSystem::get_ship_class(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "";
    return comp->ship_class;
}

int SlotGridSystem::get_tier(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->tier;
}

int SlotGridSystem::get_total_modules_placed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    return comp->total_modules_placed;
}

} // namespace systems
} // namespace atlas
