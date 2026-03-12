#include "systems/mining_ledger_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

MiningLedgerSystem::MiningLedgerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void MiningLedgerSystem::updateComponent(ecs::Entity& /*entity*/,
                                          components::MiningLedgerState& comp,
                                          float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool MiningLedgerSystem::initialize(const std::string& entity_id,
                                     const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MiningLedgerState>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Entry management
// ---------------------------------------------------------------------------

bool MiningLedgerSystem::addEntry(const std::string& entity_id,
                                   const std::string& entry_id,
                                   const std::string& ore_type,
                                   int quantity,
                                   float isk_value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (entry_id.empty()) return false;
    if (ore_type.empty()) return false;
    if (quantity <= 0) return false;
    if (isk_value < 0.0f) return false;

    // Check for duplicate entry_id
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return false;
    }

    // Purge oldest if at capacity
    if (static_cast<int>(comp->entries.size()) >= comp->max_entries) {
        comp->entries.erase(comp->entries.begin());
    }

    components::MiningLedgerState::LedgerEntry entry;
    entry.entry_id  = entry_id;
    entry.ore_type  = ore_type;
    entry.quantity  = quantity;
    entry.isk_value = isk_value;
    entry.timestamp = comp->elapsed;
    comp->entries.push_back(entry);

    comp->total_entries++;
    comp->total_quantity += quantity;
    comp->total_isk      += isk_value;
    return true;
}

bool MiningLedgerSystem::removeEntry(const std::string& entity_id,
                                      const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->entries.begin(), comp->entries.end(),
        [&](const components::MiningLedgerState::LedgerEntry& e) {
            return e.entry_id == entry_id;
        });
    if (it == comp->entries.end()) return false;
    comp->entries.erase(it);
    return true;
}

bool MiningLedgerSystem::clearLedger(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->entries.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool MiningLedgerSystem::setOwner(const std::string& entity_id,
                                   const std::string& owner_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (owner_id.empty()) return false;
    comp->owner_id = owner_id;
    return true;
}

bool MiningLedgerSystem::setMaxEntries(const std::string& entity_id, int max) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max <= 0) return false;
    comp->max_entries = max;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int MiningLedgerSystem::getEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->entries.size()) : 0;
}

int MiningLedgerSystem::getTotalEntries(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_entries : 0;
}

int MiningLedgerSystem::getTotalQuantity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_quantity : 0;
}

float MiningLedgerSystem::getTotalIsk(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_isk : 0.0f;
}

std::string MiningLedgerSystem::getOwner(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->owner_id : "";
}

bool MiningLedgerSystem::hasEntry(const std::string& entity_id,
                                   const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return true;
    }
    return false;
}

int MiningLedgerSystem::getQuantityByOreType(const std::string& entity_id,
                                              const std::string& ore_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int total = 0;
    for (const auto& e : comp->entries) {
        if (e.ore_type == ore_type) total += e.quantity;
    }
    return total;
}

float MiningLedgerSystem::getIskByOreType(const std::string& entity_id,
                                           const std::string& ore_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    float total = 0.0f;
    for (const auto& e : comp->entries) {
        if (e.ore_type == ore_type) total += e.isk_value;
    }
    return total;
}

} // namespace systems
} // namespace atlas
