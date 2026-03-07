#include "systems/cargo_hold_management_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CargoHoldManagementSystem::CargoHoldManagementSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CargoHoldManagementSystem::updateComponent(ecs::Entity& entity,
    components::CargoHoldState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Recalculate used volume each tick for consistency
    float total = 0.0f;
    for (const auto& item : comp.items) {
        total += item.volume_per_unit * item.quantity;
    }
    comp.used_volume = total;
}

bool CargoHoldManagementSystem::initialize(const std::string& entity_id,
    float max_volume) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CargoHoldState>();
    comp->max_volume = std::max(0.0f, max_volume);
    entity->addComponent(std::move(comp));
    return true;
}

bool CargoHoldManagementSystem::addItem(const std::string& entity_id,
    const std::string& item_id, int quantity, float volume_per_unit) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (quantity <= 0 || volume_per_unit < 0.0f) return false;

    float added_volume = volume_per_unit * quantity;
    if (comp->used_volume + added_volume > comp->max_volume) return false;

    // Try to stack with existing item
    for (auto& item : comp->items) {
        if (item.item_id == item_id) {
            item.quantity += quantity;
            comp->used_volume += added_volume;
            return true;
        }
    }

    // New item slot
    if (static_cast<int>(comp->items.size()) >= comp->max_item_stacks) return false;

    components::CargoHoldState::CargoItem item;
    item.item_id = item_id;
    item.quantity = quantity;
    item.volume_per_unit = volume_per_unit;
    comp->items.push_back(item);
    comp->used_volume += added_volume;
    return true;
}

bool CargoHoldManagementSystem::removeItem(const std::string& entity_id,
    const std::string& item_id, int quantity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (quantity <= 0) return false;

    for (auto it = comp->items.begin(); it != comp->items.end(); ++it) {
        if (it->item_id == item_id) {
            if (it->quantity < quantity) return false;
            float removed_volume = it->volume_per_unit * quantity;
            it->quantity -= quantity;
            comp->used_volume -= removed_volume;
            if (it->quantity == 0) {
                comp->items.erase(it);
            }
            return true;
        }
    }
    return false;
}

bool CargoHoldManagementSystem::jettisonItem(const std::string& entity_id,
    const std::string& item_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto it = comp->items.begin(); it != comp->items.end(); ++it) {
        if (it->item_id == item_id) {
            float removed_volume = it->volume_per_unit * it->quantity;
            comp->used_volume -= removed_volume;
            comp->items.erase(it);
            comp->total_jettisoned++;
            return true;
        }
    }
    return false;
}

bool CargoHoldManagementSystem::setMaxVolume(const std::string& entity_id,
    float max_volume) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->max_volume = std::max(0.0f, max_volume);
    return true;
}

int CargoHoldManagementSystem::getItemCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->items.size()) : 0;
}

int CargoHoldManagementSystem::getItemQuantity(const std::string& entity_id,
    const std::string& item_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& item : comp->items) {
        if (item.item_id == item_id) return item.quantity;
    }
    return 0;
}

float CargoHoldManagementSystem::getUsedVolume(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->used_volume : 0.0f;
}

float CargoHoldManagementSystem::getFreeVolume(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? (comp->max_volume - comp->used_volume) : 0.0f;
}

float CargoHoldManagementSystem::getMaxVolume(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_volume : 0.0f;
}

int CargoHoldManagementSystem::getTotalJettisoned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_jettisoned : 0;
}

} // namespace systems
} // namespace atlas
