#include "systems/corp_tax_ledger_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

CorpTaxLedgerSystem::CorpTaxLedgerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void CorpTaxLedgerSystem::updateComponent(ecs::Entity& /*entity*/,
                                           components::CorpTaxLedgerState& comp,
                                           float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool CorpTaxLedgerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CorpTaxLedgerState>();
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Entry management
// ---------------------------------------------------------------------------

bool CorpTaxLedgerSystem::addEntry(
        const std::string& entity_id,
        const std::string& entry_id,
        components::CorpTaxLedgerState::TaxType tax_type,
        const std::string& member_id,
        double gross_amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    return addEntryWithRate(entity_id, entry_id, tax_type, member_id,
                            gross_amount, comp->default_tax_rate);
}

bool CorpTaxLedgerSystem::addEntryWithRate(
        const std::string& entity_id,
        const std::string& entry_id,
        components::CorpTaxLedgerState::TaxType tax_type,
        const std::string& member_id,
        double gross_amount,
        double tax_rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (entry_id.empty()) return false;
    if (member_id.empty()) return false;
    if (gross_amount <= 0.0) return false;
    if (tax_rate < 0.0 || tax_rate > 1.0) return false;

    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return false;
    }

    // Purge oldest if at capacity
    if (static_cast<int>(comp->entries.size()) >= comp->max_entries) {
        comp->entries.erase(comp->entries.begin());
    }

    double collected = gross_amount * tax_rate;

    components::CorpTaxLedgerState::TaxEntry entry;
    entry.entry_id      = entry_id;
    entry.tax_type      = tax_type;
    entry.member_id     = member_id;
    entry.gross_amount  = gross_amount;
    entry.tax_rate      = tax_rate;
    entry.tax_collected = collected;
    entry.timestamp     = comp->elapsed;
    comp->entries.push_back(entry);

    comp->total_collected    += collected;
    comp->total_entries_ever++;
    return true;
}

bool CorpTaxLedgerSystem::removeEntry(const std::string& entity_id,
                                       const std::string& entry_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    auto it = std::find_if(comp->entries.begin(), comp->entries.end(),
        [&](const components::CorpTaxLedgerState::TaxEntry& e) {
            return e.entry_id == entry_id;
        });
    if (it == comp->entries.end()) return false;
    comp->entries.erase(it);
    return true;
}

bool CorpTaxLedgerSystem::clearLedger(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->entries.clear();
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool CorpTaxLedgerSystem::setCorpId(const std::string& entity_id,
                                     const std::string& corp_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (corp_id.empty()) return false;
    comp->corp_id = corp_id;
    return true;
}

bool CorpTaxLedgerSystem::setDefaultTaxRate(const std::string& entity_id,
                                             double rate) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (rate < 0.0 || rate > 1.0) return false;
    comp->default_tax_rate = rate;
    return true;
}

bool CorpTaxLedgerSystem::setMaxEntries(const std::string& entity_id,
                                         int max_entries) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (max_entries <= 0) return false;
    comp->max_entries = max_entries;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int CorpTaxLedgerSystem::getEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->entries.size()) : 0;
}

int CorpTaxLedgerSystem::getTotalEntriesEver(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_entries_ever : 0;
}

double CorpTaxLedgerSystem::getTotalCollected(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_collected : 0.0;
}

double CorpTaxLedgerSystem::getDefaultTaxRate(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->default_tax_rate : 0.0;
}

std::string CorpTaxLedgerSystem::getCorpId(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->corp_id : "";
}

bool CorpTaxLedgerSystem::hasEntry(const std::string& entity_id,
                                    const std::string& entry_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& e : comp->entries) {
        if (e.entry_id == entry_id) return true;
    }
    return false;
}

double CorpTaxLedgerSystem::getCollectedByType(
        const std::string& entity_id,
        components::CorpTaxLedgerState::TaxType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0;
    double total = 0.0;
    for (const auto& e : comp->entries) {
        if (e.tax_type == type) total += e.tax_collected;
    }
    return total;
}

double CorpTaxLedgerSystem::getCollectedByMember(
        const std::string& entity_id,
        const std::string& member_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0;
    double total = 0.0;
    for (const auto& e : comp->entries) {
        if (e.member_id == member_id) total += e.tax_collected;
    }
    return total;
}

int CorpTaxLedgerSystem::getCountByType(
        const std::string& entity_id,
        components::CorpTaxLedgerState::TaxType type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& e : comp->entries) {
        if (e.tax_type == type) count++;
    }
    return count;
}

} // namespace systems
} // namespace atlas
