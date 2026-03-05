#include "systems/atlas_ui_panel_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

AtlasUIPanelSystem::AtlasUIPanelSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void AtlasUIPanelSystem::updateComponent(ecs::Entity& /*entity*/, components::AtlasUIPanel& panel, float /*delta_time*/) {
    // Clamp selected_index within valid range
    if (!panel.items.empty()) {
        if (panel.selected_index >= static_cast<int>(panel.items.size())) {
            panel.selected_index = static_cast<int>(panel.items.size()) - 1;
        }
    } else {
        panel.selected_index = -1;
    }
}

bool AtlasUIPanelSystem::initializePanel(const std::string& entity_id,
                                          const std::string& owner_id,
                                          components::AtlasUIPanel::PanelType type) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::AtlasUIPanel>();
    if (existing) return false;

    auto comp = std::make_unique<components::AtlasUIPanel>();
    comp->panel_id = entity_id;
    comp->owner_id = owner_id;
    comp->panel_type = type;
    entity->addComponent(std::move(comp));
    return true;
}

bool AtlasUIPanelSystem::openPanel(const std::string& entity_id) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->is_open = true;
    return true;
}

bool AtlasUIPanelSystem::closePanel(const std::string& entity_id) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->is_open = false;
    return true;
}

bool AtlasUIPanelSystem::togglePanel(const std::string& entity_id) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->is_open = !panel->is_open;
    return true;
}

bool AtlasUIPanelSystem::addItem(const std::string& entity_id,
                                  const std::string& item_id,
                                  const std::string& name,
                                  int quantity, float value) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    if (static_cast<int>(panel->items.size()) >= panel->max_items) return false;

    // Check for duplicate
    if (panel->findItem(item_id)) return false;

    components::AtlasUIPanel::PanelItem item;
    item.item_id = item_id;
    item.name = name;
    item.quantity = quantity;
    item.value = value;
    panel->items.push_back(item);
    return true;
}

bool AtlasUIPanelSystem::removeItem(const std::string& entity_id,
                                     const std::string& item_id) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    for (auto it = panel->items.begin(); it != panel->items.end(); ++it) {
        if (it->item_id == item_id) {
            panel->items.erase(it);
            return true;
        }
    }
    return false;
}

bool AtlasUIPanelSystem::setFilter(const std::string& entity_id,
                                    const std::string& filter_text) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->filter_text = filter_text;
    return true;
}

bool AtlasUIPanelSystem::setSort(const std::string& entity_id,
                                  const std::string& field, bool ascending) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    panel->sort_field = field;
    panel->sort_ascending = ascending;
    return true;
}

bool AtlasUIPanelSystem::selectItem(const std::string& entity_id, int index) {
    auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    if (index < -1 || index >= static_cast<int>(panel->items.size())) return false;

    panel->selected_index = index;
    return true;
}

std::string AtlasUIPanelSystem::getPanelType(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return "";

    using PT = components::AtlasUIPanel::PanelType;
    switch (panel->panel_type) {
        case PT::Inventory: return "Inventory";
        case PT::Fitting:   return "Fitting";
        case PT::Market:    return "Market";
        case PT::Overview:  return "Overview";
        case PT::Chat:      return "Chat";
        case PT::Drone:     return "Drone";
    }
    return "";
}

bool AtlasUIPanelSystem::isOpen(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return false;

    return panel->is_open;
}

int AtlasUIPanelSystem::getItemCount(const std::string& entity_id) const {
    const auto* panel = getComponentFor(entity_id);
    if (!panel) return 0;

    return static_cast<int>(panel->items.size());
}

} // namespace systems
} // namespace atlas
