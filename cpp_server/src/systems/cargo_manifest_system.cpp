#include "systems/cargo_manifest_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CargoManifestSystem::CargoManifestSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CargoManifestSystem::updateComponent(ecs::Entity& /*entity*/,
    components::CargoManifest& /*cargo*/, float /*delta_time*/) {
    // Cargo updates are event-driven via add/remove methods
}

bool CargoManifestSystem::addItem(const std::string& entity_id,
    const std::string& item_id, const std::string& item_name,
    const std::string& category, int quantity, double volume_per_unit) {
    if (quantity <= 0 || volume_per_unit <= 0.0) return false;
    auto* cargo = getComponentFor(entity_id);
    if (!cargo) return false;

    double total_volume = quantity * volume_per_unit;
    bool is_ore = (category == "ore");

    if (is_ore && cargo->ore_hold_capacity > 0.0) {
        if (cargo->ore_hold_used + total_volume > cargo->ore_hold_capacity) return false;
    } else {
        if (cargo->general_used + total_volume > cargo->general_capacity) return false;
    }

    // Check if item already exists
    for (auto& item : cargo->items) {
        if (item.item_id == item_id) {
            double new_volume = quantity * item.volume_per_unit;
            if (is_ore && cargo->ore_hold_capacity > 0.0) {
                cargo->ore_hold_used += new_volume;
            } else {
                cargo->general_used += new_volume;
            }
            item.quantity += quantity;
            return true;
        }
    }

    // New item
    if (is_ore && cargo->ore_hold_capacity > 0.0) {
        cargo->ore_hold_used += total_volume;
    } else {
        cargo->general_used += total_volume;
    }

    components::CargoManifest::CargoItem ci;
    ci.item_id = item_id;
    ci.item_name = item_name;
    ci.category = category;
    ci.quantity = quantity;
    ci.volume_per_unit = volume_per_unit;
    cargo->items.push_back(ci);
    return true;
}

bool CargoManifestSystem::removeItem(const std::string& entity_id,
    const std::string& item_id, int quantity) {
    if (quantity <= 0) return false;
    auto* cargo = getComponentFor(entity_id);
    if (!cargo) return false;

    for (auto it = cargo->items.begin(); it != cargo->items.end(); ++it) {
        if (it->item_id == item_id) {
            if (it->quantity < quantity) return false;
            double volume_freed = quantity * it->volume_per_unit;
            bool is_ore = (it->category == "ore") && (cargo->ore_hold_capacity > 0.0);

            it->quantity -= quantity;
            if (is_ore) {
                cargo->ore_hold_used -= volume_freed;
            } else {
                cargo->general_used -= volume_freed;
            }

            if (it->quantity == 0) {
                cargo->items.erase(it);
            }
            return true;
        }
    }
    return false;
}

int CargoManifestSystem::getItemQuantity(const std::string& entity_id,
    const std::string& item_id) const {
    auto* cargo = getComponentFor(entity_id);
    if (!cargo) return 0;
    for (const auto& item : cargo->items) {
        if (item.item_id == item_id) return item.quantity;
    }
    return 0;
}

double CargoManifestSystem::getGeneralUsed(const std::string& entity_id) const {
    auto* cargo = getComponentFor(entity_id);
    return cargo ? cargo->general_used : 0.0;
}

double CargoManifestSystem::getGeneralCapacity(const std::string& entity_id) const {
    auto* cargo = getComponentFor(entity_id);
    return cargo ? cargo->general_capacity : 0.0;
}

double CargoManifestSystem::getOreHoldUsed(const std::string& entity_id) const {
    auto* cargo = getComponentFor(entity_id);
    return cargo ? cargo->ore_hold_used : 0.0;
}

double CargoManifestSystem::getOreHoldCapacity(const std::string& entity_id) const {
    auto* cargo = getComponentFor(entity_id);
    return cargo ? cargo->ore_hold_capacity : 0.0;
}

int CargoManifestSystem::getItemCount(const std::string& entity_id) const {
    auto* cargo = getComponentFor(entity_id);
    return cargo ? static_cast<int>(cargo->items.size()) : 0;
}

bool CargoManifestSystem::jettisonItem(const std::string& entity_id,
    const std::string& item_id, int quantity) {
    return removeItem(entity_id, item_id, quantity);
}

bool CargoManifestSystem::transferItem(const std::string& from_id,
    const std::string& to_id, const std::string& item_id, int quantity) {
    if (quantity <= 0) return false;
    auto* from_cargo = getComponentFor(from_id);
    auto* to_cargo = getComponentFor(to_id);
    if (!from_cargo || !to_cargo) return false;

    // Find item in source
    for (const auto& item : from_cargo->items) {
        if (item.item_id == item_id) {
            if (item.quantity < quantity) return false;
            // Check destination has space
            if (!hasSpace(to_id, item.category, quantity * item.volume_per_unit)) return false;

            // Remove from source, add to destination
            removeItem(from_id, item_id, quantity);
            addItem(to_id, item_id, item.item_name, item.category, quantity, item.volume_per_unit);
            return true;
        }
    }
    return false;
}

double CargoManifestSystem::getTotalVolumeUsed(const std::string& entity_id) const {
    auto* cargo = getComponentFor(entity_id);
    if (!cargo) return 0.0;
    return cargo->general_used + cargo->ore_hold_used;
}

bool CargoManifestSystem::hasSpace(const std::string& entity_id,
    const std::string& category, double volume) const {
    auto* cargo = getComponentFor(entity_id);
    if (!cargo) return false;
    if (category == "ore" && cargo->ore_hold_capacity > 0.0) {
        return (cargo->ore_hold_used + volume) <= cargo->ore_hold_capacity;
    }
    return (cargo->general_used + volume) <= cargo->general_capacity;
}

} // namespace systems
} // namespace atlas
