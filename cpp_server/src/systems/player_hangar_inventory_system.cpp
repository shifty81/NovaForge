#include "systems/player_hangar_inventory_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PlayerHangarInventorySystem::PlayerHangarInventorySystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PlayerHangarInventorySystem::updateComponent(ecs::Entity& entity,
    components::PlayerHangarInventory& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

bool PlayerHangarInventorySystem::initialize(const std::string& entity_id,
    const std::string& player_id, const std::string& station_id, double max_volume) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (player_id.empty() || station_id.empty()) return false;
    if (max_volume <= 0.0) return false;

    auto comp = std::make_unique<components::PlayerHangarInventory>();
    comp->player_id = player_id;
    comp->station_id = station_id;
    comp->max_volume = max_volume;
    entity->addComponent(std::move(comp));
    return true;
}

bool PlayerHangarInventorySystem::depositItem(const std::string& entity_id,
    const std::string& item_id, const std::string& item_name, const std::string& item_type,
    int quantity, double volume_per_unit, double estimated_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (item_id.empty() || item_name.empty() || item_type.empty()) return false;
    if (quantity <= 0 || volume_per_unit <= 0.0) return false;

    double required_volume = quantity * volume_per_unit;
    if (comp->used_volume + required_volume > comp->max_volume) return false;

    // Stack with existing item if same id
    for (auto& item : comp->items) {
        if (item.item_id == item_id) {
            item.quantity += quantity;
            item.estimated_value += estimated_value * quantity;
            comp->used_volume += required_volume;
            comp->total_value_stored += estimated_value * quantity;
            comp->total_deposits++;
            return true;
        }
    }

    // New item
    components::PlayerHangarInventory::HangarItem newItem;
    newItem.item_id = item_id;
    newItem.item_name = item_name;
    newItem.item_type = item_type;
    newItem.quantity = quantity;
    newItem.volume_per_unit = volume_per_unit;
    newItem.estimated_value = estimated_value * quantity;
    comp->items.push_back(newItem);
    comp->used_volume += required_volume;
    comp->total_value_stored += estimated_value * quantity;
    comp->total_deposits++;
    return true;
}

bool PlayerHangarInventorySystem::withdrawItem(const std::string& entity_id,
    const std::string& item_id, int quantity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (item_id.empty() || quantity <= 0) return false;

    auto it = std::find_if(comp->items.begin(), comp->items.end(),
        [&item_id](const components::PlayerHangarInventory::HangarItem& i) {
            return i.item_id == item_id;
        });
    if (it == comp->items.end()) return false;
    if (it->quantity < quantity) return false;

    double volume_freed = quantity * it->volume_per_unit;
    double value_per_unit = (it->quantity > 0) ? it->estimated_value / it->quantity : 0.0;
    double value_removed = value_per_unit * quantity;

    it->quantity -= quantity;
    it->estimated_value -= value_removed;
    comp->used_volume -= volume_freed;
    comp->total_value_stored -= value_removed;
    comp->total_withdrawals++;

    if (it->quantity <= 0) {
        comp->items.erase(it);
    }
    return true;
}

bool PlayerHangarInventorySystem::transferItem(const std::string& source_entity,
    const std::string& dest_entity, const std::string& item_id, int quantity) {
    auto* src = getComponentFor(source_entity);
    auto* dst = getComponentFor(dest_entity);
    if (!src || !dst) return false;
    if (item_id.empty() || quantity <= 0) return false;

    // Find item in source
    auto it = std::find_if(src->items.begin(), src->items.end(),
        [&item_id](const components::PlayerHangarInventory::HangarItem& i) {
            return i.item_id == item_id;
        });
    if (it == src->items.end()) return false;
    if (it->quantity < quantity) return false;

    double volume_needed = quantity * it->volume_per_unit;
    if (dst->used_volume + volume_needed > dst->max_volume) return false;

    double value_per_unit = (it->quantity > 0) ? it->estimated_value / it->quantity : 0.0;

    // Deposit to destination first (will stack or create new)
    if (!depositItem(dest_entity, it->item_id, it->item_name, it->item_type,
                     quantity, it->volume_per_unit, value_per_unit)) {
        return false;
    }

    // Withdraw from source
    withdrawItem(source_entity, item_id, quantity);
    return true;
}

bool PlayerHangarInventorySystem::setMaxVolume(const std::string& entity_id, double max_volume) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_volume <= 0.0) return false;
    if (max_volume < comp->used_volume) return false; // Can't shrink below used
    comp->max_volume = max_volume;
    return true;
}

int PlayerHangarInventorySystem::getItemCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->items.size()) : 0;
}

int PlayerHangarInventorySystem::getItemQuantity(const std::string& entity_id,
    const std::string& item_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    for (const auto& item : comp->items) {
        if (item.item_id == item_id) return item.quantity;
    }
    return 0;
}

double PlayerHangarInventorySystem::getUsedVolume(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->used_volume : 0.0;
}

double PlayerHangarInventorySystem::getRemainingVolume(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->remainingVolume() : 0.0;
}

double PlayerHangarInventorySystem::getTotalValueStored(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_value_stored : 0.0;
}

int PlayerHangarInventorySystem::getTotalDeposits(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_deposits : 0;
}

int PlayerHangarInventorySystem::getTotalWithdrawals(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_withdrawals : 0;
}

std::string PlayerHangarInventorySystem::getStationId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->station_id : "";
}

std::string PlayerHangarInventorySystem::getPlayerId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->player_id : "";
}

bool PlayerHangarInventorySystem::hasItem(const std::string& entity_id,
    const std::string& item_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& item : comp->items) {
        if (item.item_id == item_id) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
