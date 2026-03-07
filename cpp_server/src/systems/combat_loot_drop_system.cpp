#include "systems/combat_loot_drop_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CombatLootDropSystem::CombatLootDropSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void CombatLootDropSystem::updateComponent(ecs::Entity& entity,
    components::CombatLootDrop& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Process pending drops each tick
    for (auto& pending : comp.pending_drops) {
        if (pending.resolved) continue;

        for (auto& entry : comp.drop_table) {
            // Deterministic roll based on elapsed time
            float roll = comp.elapsed - static_cast<int>(comp.elapsed);
            if (roll < entry.drop_chance) {
                // Determine quantity within min/max range
                int range = entry.max_qty - entry.min_qty;
                int qty = entry.min_qty;
                if (range > 0) {
                    int seed = static_cast<int>(comp.elapsed * 1000.0f) % (range + 1);
                    qty += seed;
                }
                comp.total_items_dropped += qty;
            }
        }
        pending.resolved = true;
        comp.total_drops_triggered++;
    }

    // Remove resolved pending drops
    comp.pending_drops.erase(
        std::remove_if(comp.pending_drops.begin(), comp.pending_drops.end(),
            [](const components::CombatLootDrop::PendingDrop& p) { return p.resolved; }),
        comp.pending_drops.end());
}

bool CombatLootDropSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CombatLootDrop>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CombatLootDropSystem::addDropEntry(const std::string& entity_id,
    const std::string& item_id, float drop_chance, int min_qty, int max_qty,
    const std::string& rarity) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (min_qty < 0 || max_qty < min_qty) return false;

    // No duplicate item IDs in drop table
    for (const auto& entry : comp->drop_table) {
        if (entry.item_id == item_id) return false;
    }
    if (static_cast<int>(comp->drop_table.size()) >= comp->max_drop_entries) return false;

    components::CombatLootDrop::DropEntry entry;
    entry.item_id = item_id;
    entry.drop_chance = std::max(0.0f, std::min(1.0f, drop_chance));
    entry.min_qty = min_qty;
    entry.max_qty = max_qty;
    entry.rarity = rarity;
    comp->drop_table.push_back(entry);
    return true;
}

bool CombatLootDropSystem::removeDropEntry(const std::string& entity_id,
    const std::string& item_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->drop_table.begin(), comp->drop_table.end(),
        [&item_id](const components::CombatLootDrop::DropEntry& e) {
            return e.item_id == item_id;
        });
    if (it == comp->drop_table.end()) return false;
    comp->drop_table.erase(it);
    return true;
}

bool CombatLootDropSystem::triggerDrop(const std::string& entity_id,
    const std::string& source_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->drop_table.empty()) return false;

    components::CombatLootDrop::PendingDrop pending;
    pending.source_id = source_id;
    comp->pending_drops.push_back(pending);
    comp->last_drop_source = source_id;
    return true;
}

int CombatLootDropSystem::getDropEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->drop_table.size()) : 0;
}

int CombatLootDropSystem::getTotalDropsTriggered(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_drops_triggered : 0;
}

int CombatLootDropSystem::getTotalItemsDropped(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_items_dropped : 0;
}

int CombatLootDropSystem::getPendingDropCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->pending_drops.size()) : 0;
}

std::string CombatLootDropSystem::getLastDropSource(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->last_drop_source : "";
}

} // namespace systems
} // namespace atlas
