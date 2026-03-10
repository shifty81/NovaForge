#include "systems/respawn_selection_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

RespawnSelectionSystem::RespawnSelectionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void RespawnSelectionSystem::updateComponent(ecs::Entity& /*entity*/,
                                              components::RespawnSelection& comp,
                                              float delta_time) {
    if (!comp.active) return;
    if (!comp.is_open) return;
    if (comp.auto_select_timer <= 0.0f) return;

    comp.auto_select_timer -= delta_time;
    if (comp.auto_select_timer <= 0.0f) {
        comp.auto_select_timer = 0.0f;
        // Pick the default location, or fall back to the first location.
        if (!comp.locations.empty()) {
            for (const auto& loc : comp.locations) {
                if (loc.is_default) {
                    comp.selected_location_id = loc.location_id;
                    break;
                }
            }
            if (comp.selected_location_id.empty()) {
                comp.selected_location_id = comp.locations.front().location_id;
            }
        }
        comp.is_open = false;
        comp.total_selections++;
    }
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool RespawnSelectionSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::RespawnSelection>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Panel control
// ---------------------------------------------------------------------------

bool RespawnSelectionSystem::openSelection(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->is_open) return false;
    comp->is_open = true;
    comp->selected_location_id.clear();
    comp->auto_select_timer = comp->auto_select_duration;
    return true;
}

bool RespawnSelectionSystem::confirmSelection(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_open) return false;
    if (comp->selected_location_id.empty()) return false;
    comp->is_open = false;
    comp->auto_select_timer = 0.0f;
    comp->total_selections++;
    return true;
}

// ---------------------------------------------------------------------------
// Location management
// ---------------------------------------------------------------------------

bool RespawnSelectionSystem::addLocation(const std::string& entity_id,
                                          const std::string& location_id,
                                          const std::string& location_name,
                                          float distance_ly,
                                          bool is_default) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (location_id.empty()) return false;
    if (static_cast<int>(comp->locations.size()) >= comp->max_locations) return false;

    for (const auto& loc : comp->locations) {
        if (loc.location_id == location_id) return false;
    }

    components::RespawnSelection::RespawnLocation loc;
    loc.location_id   = location_id;
    loc.location_name = location_name;
    loc.distance_ly   = distance_ly;
    loc.is_default    = is_default;
    comp->locations.push_back(loc);
    return true;
}

bool RespawnSelectionSystem::removeLocation(const std::string& entity_id,
                                             const std::string& location_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->locations.begin(), comp->locations.end(),
        [&](const components::RespawnSelection::RespawnLocation& l) {
            return l.location_id == location_id;
        });
    if (it == comp->locations.end()) return false;
    comp->locations.erase(it);
    if (comp->selected_location_id == location_id) {
        comp->selected_location_id.clear();
    }
    return true;
}

bool RespawnSelectionSystem::selectLocation(const std::string& entity_id,
                                             const std::string& location_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->is_open) return false;
    for (const auto& loc : comp->locations) {
        if (loc.location_id == location_id) {
            comp->selected_location_id = location_id;
            return true;
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

bool RespawnSelectionSystem::isOpen(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp && comp->is_open;
}

std::string RespawnSelectionSystem::getSelectedLocation(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->selected_location_id : std::string();
}

int RespawnSelectionSystem::getLocationCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->locations.size()) : 0;
}

int RespawnSelectionSystem::getTotalSelections(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_selections : 0;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool RespawnSelectionSystem::setAutoSelectDuration(const std::string& entity_id,
                                                    float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || duration < 0.0f) return false;
    comp->auto_select_duration = duration;
    return true;
}

} // namespace systems
} // namespace atlas
