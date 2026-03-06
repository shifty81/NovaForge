#include "systems/loot_container_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

LootContainerSystem::LootContainerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void LootContainerSystem::updateComponent(ecs::Entity& /*entity*/,
    components::LootContainer& container, float delta_time) {
    if (!container.active) return;

    // Count down expiry timer
    if (container.time_remaining > 0.0f) {
        container.time_remaining -= delta_time;
        if (container.time_remaining <= 0.0f) {
            container.time_remaining = 0.0f;
            container.active = false;
        }
    }
}

bool LootContainerSystem::addItem(const std::string& entity_id,
    const std::string& item_id, const std::string& name,
    const std::string& category, int quantity, float volume, float value) {
    if (quantity <= 0 || volume < 0.0f || value < 0.0f) return false;
    auto* container = getComponentFor(entity_id);
    if (!container) return false;
    if (container->is_locked) return false;

    // Check if item already exists, stack it
    for (auto& item : container->items) {
        if (item.item_id == item_id) {
            item.quantity += quantity;
            container->total_value += value * quantity;
            container->total_volume += volume * quantity;
            return true;
        }
    }

    if (static_cast<int>(container->items.size()) >= container->max_items) return false;

    components::LootContainer::LootItem item;
    item.item_id = item_id;
    item.name = name;
    item.category = category;
    item.quantity = quantity;
    item.volume = volume;
    item.value = value;
    container->items.push_back(item);
    container->total_value += value * quantity;
    container->total_volume += volume * quantity;
    return true;
}

bool LootContainerSystem::removeItem(const std::string& entity_id,
    const std::string& item_id, int quantity) {
    if (quantity <= 0) return false;
    auto* container = getComponentFor(entity_id);
    if (!container) return false;
    if (container->is_locked) return false;

    for (auto it = container->items.begin(); it != container->items.end(); ++it) {
        if (it->item_id == item_id) {
            if (it->quantity < quantity) return false;
            container->total_value -= it->value * quantity;
            container->total_volume -= it->volume * quantity;
            it->quantity -= quantity;
            container->total_looted += quantity;
            if (it->quantity <= 0) {
                container->items.erase(it);
            }
            return true;
        }
    }
    return false;
}

int LootContainerSystem::getItemCount(const std::string& entity_id) const {
    auto* container = getComponentFor(entity_id);
    return container ? static_cast<int>(container->items.size()) : 0;
}

float LootContainerSystem::getTotalValue(const std::string& entity_id) const {
    auto* container = getComponentFor(entity_id);
    return container ? container->total_value : 0.0f;
}

float LootContainerSystem::getTotalVolume(const std::string& entity_id) const {
    auto* container = getComponentFor(entity_id);
    return container ? container->total_volume : 0.0f;
}

float LootContainerSystem::getTimeRemaining(const std::string& entity_id) const {
    auto* container = getComponentFor(entity_id);
    return container ? container->time_remaining : 0.0f;
}

bool LootContainerSystem::abandonContainer(const std::string& entity_id) {
    auto* container = getComponentFor(entity_id);
    if (!container) return false;
    container->is_abandoned = true;
    container->owner_id.clear();
    return true;
}

bool LootContainerSystem::lockContainer(const std::string& entity_id, bool locked) {
    auto* container = getComponentFor(entity_id);
    if (!container) return false;
    container->is_locked = locked;
    return true;
}

bool LootContainerSystem::isAccessible(const std::string& entity_id,
    const std::string& accessor_id) const {
    auto* container = getComponentFor(entity_id);
    if (!container) return false;
    if (!container->active) return false;
    if (container->is_locked) return false;
    if (container->is_abandoned) return true;
    return container->owner_id == accessor_id;
}

int LootContainerSystem::getTotalLooted(const std::string& entity_id) const {
    auto* container = getComponentFor(entity_id);
    return container ? container->total_looted : 0;
}

bool LootContainerSystem::setOwner(const std::string& entity_id,
    const std::string& owner_id) {
    auto* container = getComponentFor(entity_id);
    if (!container) return false;
    container->owner_id = owner_id;
    return true;
}

} // namespace systems
} // namespace atlas
